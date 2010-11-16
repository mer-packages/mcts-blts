/* wlan-core-eapol.c -- EAPOL handling

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Portions derived from wpa_supplicant and hostapd code. License header:
 *
 * Copyright (c) 2002-2010, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2003-2004, Instant802 Networks, Inc.
 * Copyright (c) 2005-2006, Devicescape Software, Inc.
 * Copyright (c) 2007, Johannes Berg <johannes@sipsolutions.net>
 * Copyright (c) 2009-2010, Atheros Communications
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include <blts_log.h>

#include "wlan-core-definitions.h"
#include "ieee802_11_defs.h"
#include "wlan-core-eapol.h"
#include "wlan-core-debug.h"
#include "wlan-core-monitor.h"
#include "wlan-core-crypto.h"
#include "wlan-core-utils.h"

/* unused for now */
/* enum { */
/* 	EAPOL_SUPP_KM_STATE_UNKNOWN, */
/* 	EAPOL_SUPP_KM_STATE_DISCONNECTED, */
/* 	EAPOL_SUPP_KM_STATE_INITIALIZE, */
/* 	EAPOL_SUPP_KM_STATE_AUTHENTICATION, */
/* 	EAPOL_SUPP_KM_STATE_STAKEYSTART, */
/* }; */

/* enum { */
/* 	EAPOL_PORT_CTRL_AUTO, */
/* 	EAPOL_PORT_CTRL_FORCE_UNAUTHORIZED, */
/* 	EAPOL_PORT_CTRL_FORCE_AUTHORIZED */
/* }; */
/* enum { */
/* 	EAPOL_PORT_STATUS_UNAUTHORIZED, */
/* 	EAPOL_PORT_STATUS_AUTHORIZED */
/* }; */

/* Basic supplicant state machine state, mostly according to
 * standard. We mostly use this for temporary key storage. */
struct eapol_supplicant_sm {
	unsigned ksm_state;

	u8 *msk;
	unsigned msk_len;
	/* Names with Caps are from the standard; see 802.11 Ch.8 */
	unsigned DeauthenticationRequest;
	unsigned AuthenticationRequest;
	unsigned AuthenticationFailed;
	unsigned EAPOLKeyReceived;
	unsigned IntegrityFailed;
	unsigned MICVerified;

	u64 Counter;

	u8 SNonce[WPA_NONCE_LEN];
	u8 ANonce[WPA_NONCE_LEN];

	u8 PTK[64];
	u8 TPTK[64];
	unsigned ptk_len; /* 48 for ccmp, 64 for tkip */

	u8 GTK[4][WPA_GTK_MAX_LEN];
	unsigned gtk_present; /* bitfield; BIT(n) == 1 -> gtk with keyid n present */

	unsigned keycount;

	unsigned PortEnabled;
	unsigned PortValid;
	unsigned PortControl;
};

/* Callback data for loop */
struct eapol_loop_cb {
	int (*on_change_cb)(int auth_status, void *user_data);
	void *data;
};

/* Top level EAPOL state. */
struct eapol_loop_state {
	unsigned refcnt;
	pthread_t loop_tid;
	pthread_t worker_tid;

	int state;

	int eapol_sock;

	struct eapol_supplicant_sm sm;

	int eapol_auth_state;

	wlan_core_data *ctx;

	struct eapol_loop_cb state_change_cb;
};

/* Ethernet / 80211 header for raw packet handling. */
struct ieee80211_data_header_for_8021x {
        u16 frame_control;
        u16 duration;
        u8 da[ETH_ALEN];
        u8 sa[ETH_ALEN];
        u8 bssid[ETH_ALEN];
        u16 seq_ctrl;

	u8 llc_id[3];
	u8 llc_org[3];
	u16 llc_type;
} __attribute__((packed));

enum { EAPOL_KEY_TYPE_RC4 = 1, EAPOL_KEY_TYPE_RSN = 2,
       EAPOL_KEY_TYPE_WPA = 254 };

/* This protects EAPOL state during callbacks etc. */
static pthread_mutex_t loop_state_lock = PTHREAD_MUTEX_INITIALIZER;
static struct eapol_loop_state loop_state = {};

/* hostap wpa ie parser (rsn_supp/wpa_ie.[ch]) */
struct wpa_eapol_ie_parse {
	const u8 *wpa_ie;
	size_t wpa_ie_len;
	const u8 *rsn_ie;
	size_t rsn_ie_len;
	const u8 *pmkid;
	const u8 *gtk;
	size_t gtk_len;
	const u8 *mac_addr;
	size_t mac_addr_len;
	const u8 *smk;
	size_t smk_len;
	const u8 *nonce;
	size_t nonce_len;
	const u8 *lifetime;
	size_t lifetime_len;
	const u8 *error;
	size_t error_len;
	const u8 *igtk;
	size_t igtk_len;
	const u8 *mdie;
	size_t mdie_len;
	const u8 *ftie;
	size_t ftie_len;
	const u8 *reassoc_deadline;
	const u8 *key_lifetime;
};

static int wpa_supplicant_parse_ies(const u8 *buf, size_t len,
				struct wpa_eapol_ie_parse *ie);

/* This is the LLC header, minus protocol (0x88 0x8e for EAPOL). */
static const u8 rfc1042_header[6] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };

/* Used by key install code */
struct wpa_gtk_data {
	unsigned alg;
        unsigned tx, keyidx;
	u8 gtk[WPA_GTK_MAX_LEN];
	size_t key_rsc_len, gtk_len;
};

/* Broadcast state change to registered callbacks. */
static int eapol_auth_state_change(int newstate)
{
	loop_state.eapol_auth_state = newstate;

	if(loop_state.state_change_cb.on_change_cb)
		return loop_state.state_change_cb.on_change_cb(newstate,
				loop_state.state_change_cb.data);
	return 0;
}

static int eapol_install_ptk(struct eapol_key_frame *frame)
{
	const u8 null_rsc[WPA_KEY_RSC_LEN] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int ret;

	ret = nl80211_set_key_ccmp(loop_state.ctx, loop_state.ctx->their_addr, 0, 1,
				null_rsc, CCMP_RSC_LEN, loop_state.sm.PTK + 32, 16);
	if(ret) {
		BLTS_ERROR("EAPOL: Failed setting PTK in driver\n");
		return ret;
	}

	BLTS_TRACE("EAPOL: PTK installed.\n");
	return 0;
}

static int eapol_install_gtk(struct eapol_key_frame *frame, struct wpa_gtk_data *gtk)
{
	const u8 addr[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	int ret;

	BLTS_TRACE("EAPOL: Installing GTK (index=%u, tx bit=%u, length=%u)\n",
		gtk->keyidx, gtk->tx, gtk->gtk_len);
	hexdump("EAPOL: GTK", gtk->gtk, gtk->gtk_len);
	hexdump("EAPOL: RSC", frame->key_rsc, CCMP_RSC_LEN);

	ret = nl80211_set_key_ccmp(loop_state.ctx, addr, gtk->keyidx, gtk->tx,
			frame->key_rsc,	CCMP_RSC_LEN, gtk->gtk, gtk->gtk_len);
	if(ret) {
		BLTS_ERROR("EAPOL: Failed installing group key\n");
		return ret;
	}
	BLTS_TRACE("EAPOL: Group key installed.\n");
	return 0;
}

static int eapol_set_pairwise_gtk(struct eapol_key_frame *frame, const u8 *gtk, size_t gtk_len)
{
	struct wpa_gtk_data gd = {};

	if(gtk_len < 2 || gtk_len - 2 > WPA_GTK_MAX_LEN) {
		BLTS_ERROR("EAPOL: ERROR - invalid group key length\n");
		return -EINVAL;
	}

	gd.keyidx = gtk[0] & 0x03;
	gd.tx = !!(gtk[0] & BIT(2));

	gtk += 2;
	gtk_len -= 2;

	memcpy(gd.gtk, gtk, gtk_len);
	gd.gtk_len = gtk_len;

	return eapol_install_gtk(frame, &gd);
}

static int eapol_send_key_frame(wlan_core_data *data, void *frame,
				size_t frame_len)
{
	int res;

	res = monitor_send_l2_packet(data, ETH_P_EAPOL, frame, frame_len);

	if(res < 0) {
		int err = errno;
		BLTS_ERROR("EAPOL: eapol_send_key_frame - EAPOL-KEY len: %lu - "
			   "failed: %d (%s)\n",
			   (unsigned long) frame_len, err, strerror(err));
	}
	return res;
}

static int eapol_set_port_auth(unsigned authorized)
{
	BLTS_DEBUG("EAPOL: Authorizing port.\n");
	if(nl80211_set_supp_port(loop_state.ctx, authorized)) {
		BLTS_ERROR("EAPOL: ERROR - Cannot change port authorization state.\n");
		return -1;
	}

	if(authorized) {
		/* At this point the network ifce is up and running, and data
		 * transfer can be performed. */
		loop_state.sm.PortEnabled = 1;
		loop_state.sm.PortValid = 1;
	} else {
		loop_state.sm.PortEnabled = 0;
		loop_state.sm.PortValid = 0;
	}
	return 0;
}

/* Return 1 if raw frame is eapol data, 0 otherwise. */
static int is_eapol_frame(u8 *frame, unsigned len)
{
	struct ieee80211_data_header_for_8021x *data_f;
	if (len < sizeof(struct ieee80211_data_header_for_8021x))
		return 0;

	data_f = (struct ieee80211_data_header_for_8021x *) frame;

	if(WLAN_FC_GET_TYPE(data_f->frame_control) != WLAN_FC_TYPE_DATA)
		return 0;
	if(data_f->llc_type != host_to_be16(ETH_P_EAPOL))
		return 0;

	return 1;
}

static void eapol_debug_dump_frame(u8 *frame, unsigned len)
{
	struct eapol_key_frame *f;
	if(len < sizeof (struct eapol_key_frame)) {
		BLTS_TRACE("!!! Not a EAPOL-KEY frame !!!\n");
	}
	f = (struct eapol_key_frame *) frame;
	BLTS_TRACE("EAPOL-KEY frame {\n"
		"\tprotocol_version   = %u\n"
		"\tpacket_type        = 0x%02x\n"
		"\tpacket_body_len    = %u\n"
		"\tdesc_type          = 0x%02x\n"
		"\tkey_info           = 0x%04x\n"
		"\tkey_len            = %u\n"
		"\tkey_replay_counter = [%02x %02x %02x %02x %02x %02x %02x %02x]\n"
		"\tkey_nonce          = [%02x %02x %02x %02x %02x %02x %02x %02x\n"
		"\t                      %02x %02x %02x %02x %02x %02x %02x %02x\n"
		"\t                      %02x %02x %02x %02x %02x %02x %02x %02x\n"
		"\t                      %02x %02x %02x %02x %02x %02x %02x %02x]\n"
		"\teapol_key_iv       = [%02x %02x %02x %02x %02x %02x %02x %02x\n"
		"\t                      %02x %02x %02x %02x %02x %02x %02x %02x]\n"
		"\tkey_rsc            = [%02x %02x %02x %02x %02x %02x %02x %02x]\n"
		"\treserved           = [%02x %02x %02x %02x %02x %02x %02x %02x]\n"
		"\tkey_mic            = [%02x %02x %02x %02x %02x %02x %02x %02x\n"
		"\t                      %02x %02x %02x %02x %02x %02x %02x %02x]\n"
		"\tkey_data_len       = %u\n"
		"}\n",
		f->protocol_version, f->packet_type, be_to_host16(f->packet_body_len),
		f->desc_type, f->key_info, be_to_host16(f->key_len),
		f->key_replay_counter[0], f->key_replay_counter[1],
		f->key_replay_counter[2], f->key_replay_counter[3],
		f->key_replay_counter[4], f->key_replay_counter[5],
		f->key_replay_counter[6], f->key_replay_counter[7],
		f->key_nonce[0], f->key_nonce[1], f->key_nonce[2],
		f->key_nonce[3], f->key_nonce[4], f->key_nonce[5],
		f->key_nonce[6], f->key_nonce[7], f->key_nonce[8],
		f->key_nonce[9], f->key_nonce[10], f->key_nonce[11],
		f->key_nonce[12], f->key_nonce[13], f->key_nonce[14],
		f->key_nonce[15], f->key_nonce[16], f->key_nonce[17],
		f->key_nonce[18], f->key_nonce[19], f->key_nonce[20],
		f->key_nonce[21], f->key_nonce[22], f->key_nonce[23],
		f->key_nonce[24], f->key_nonce[25], f->key_nonce[26],
		f->key_nonce[27], f->key_nonce[28], f->key_nonce[29],
		f->key_nonce[30], f->key_nonce[31], f->eapol_key_iv[0],
		f->eapol_key_iv[1], f->eapol_key_iv[2], f->eapol_key_iv[3],
		f->eapol_key_iv[4], f->eapol_key_iv[5], f->eapol_key_iv[6],
		f->eapol_key_iv[7], f->eapol_key_iv[8], f->eapol_key_iv[9],
		f->eapol_key_iv[10], f->eapol_key_iv[11], f->eapol_key_iv[12],
		f->eapol_key_iv[13], f->eapol_key_iv[14], f->eapol_key_iv[15],
		f->key_rsc[0], f->key_rsc[1], f->key_rsc[2], f->key_rsc[3],
		f->key_rsc[4], f->key_rsc[5], f->key_rsc[6], f->key_rsc[7],
		f->_reserved[0], f->_reserved[1], f->_reserved[2],
		f->_reserved[3], f->_reserved[4], f->_reserved[5],
		f->_reserved[6], f->_reserved[7], f->key_mic[0], f->key_mic[1],
		f->key_mic[2], f->key_mic[3], f->key_mic[4], f->key_mic[5],
		f->key_mic[6], f->key_mic[7], f->key_mic[8], f->key_mic[9],
		f->key_mic[10], f->key_mic[11],	f->key_mic[12], f->key_mic[13],
		f->key_mic[14], f->key_mic[15],	be_to_host16(f->key_data_len));
}

static int is_bitfield_zero(u8 *field, unsigned len)
{
	u8 s = 0,i;
	for(i = 0; i < len; ++i)
		s |= field[i];
	return s?0:1;
}

static void debug_decode_key_info(u16 ki)
{
	BLTS_TRACE("Key info ->\n");
	BLTS_TRACE("\tdescr v=%u\n", (ki & WPA_KEY_INFO_TYPE_MASK));
	BLTS_TRACE("\ttype=%u\n", (ki & WPA_KEY_INFO_KEY_TYPE)?1:0);
	BLTS_TRACE("\tindex=%llu\n", (ki & WPA_KEY_INFO_KEY_INDEX_MASK)>> WPA_KEY_INFO_KEY_INDEX_SHIFT);
	BLTS_TRACE("\tack=%u\n", (ki & WPA_KEY_INFO_ACK)?1:0);
	BLTS_TRACE("\tmic=%u\n", (ki & WPA_KEY_INFO_MIC)?1:0);
	BLTS_TRACE("\tsecure=%u\n", (ki & WPA_KEY_INFO_SECURE)?1:0);
	BLTS_TRACE("\terror=%u\n", (ki & WPA_KEY_INFO_ERROR)?1:0);
	BLTS_TRACE("\trequest=%u\n", (ki & WPA_KEY_INFO_REQUEST)?1:0);
	BLTS_TRACE("\tencr_key_data=%u\n", (ki & WPA_KEY_INFO_ENCR_KEY_DATA)?1:0);
	BLTS_TRACE("\tsmk_message=%u\n", (ki & WPA_KEY_INFO_SMK_MESSAGE)?1:0);
}

/* recognize start of 4way handshake */
static int is_4way_msg_1(struct eapol_key_frame *frame, unsigned frame_len)
{
	u16 ki = be_to_host16(frame->key_info);
	if(frame->desc_type != EAPOL_KEY_TYPE_RSN) {
		BLTS_TRACE("EAPOL: Not RSN key\n");
		return 0;
	}
	if(!((ki & WPA_KEY_INFO_TYPE_MASK) == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES
		&& (ki & WPA_KEY_INFO_KEY_TYPE)
		&& (ki & WPA_KEY_INFO_ACK)
		&& !(ki & WPA_KEY_INFO_MIC)
		&& !(ki & WPA_KEY_INFO_SECURE)
		&& !(ki & WPA_KEY_INFO_ERROR)
		&& !(ki & WPA_KEY_INFO_REQUEST)
		&& !(ki & WPA_KEY_INFO_ENCR_KEY_DATA)
		&& !(ki & WPA_KEY_INFO_SMK_MESSAGE))) {
		BLTS_TRACE("EAPOL: Improper key info for msg 1\n");
		debug_decode_key_info(ki);
		return 0;
	}
	if(!is_bitfield_zero(frame->eapol_key_iv, 16)) {
		BLTS_TRACE("EAPOL: Junk in key IV\n");
		return 0;
	}

	if(!is_bitfield_zero(frame->key_rsc, 8)) {
		BLTS_TRACE("EAPOL: Junk in key RSC\n");
		return 0;
	}
	if(!is_bitfield_zero(frame->key_mic, 16)) {
		BLTS_TRACE("EAPOL: Junk in key MIC\n");
		return 0;
	}

	return 1;
}

/* Security related checks on 3 of 4
 * 1) replay counter must be unused (in practise, last seen + 1)
 * 2) AP nonce must be same as it sent on msg 1
 * 3) RSN IE must be identical to AP's beacon/probe response frame RSN IE
 * Note that this must be the last check in the chain.
 * ies will be returned in ies (if non-null), if everything checks out.
 */
static int eapol_3_of_4_security_ok(struct eapol_key_frame *frame,
				struct wpa_eapol_ie_parse *ies)
{
	u64 frame_key_replay_counter;
	u16 ap_rsn_ie_len, key_data_len;
	struct wpa_eapol_ie_parse ie;

	frame_key_replay_counter = WPA_GET_BE64(frame->key_replay_counter);
	if(frame_key_replay_counter <= loop_state.sm.Counter) {
		BLTS_ERROR("EAPOL: WARNING: Key Replay Counter reuse detected - bad frame/bad AP end\n");
		return 0;
	}

	if(memcmp(frame->key_nonce, loop_state.sm.ANonce, WPA_NONCE_LEN)) {
		BLTS_ERROR("EAPOL: WARNING: AP Nonce change detected - bad frame/bad AP end\n");
		return 0;
	}

	/* rsnie: <ie tag:8><len:8>[len bytes] */
	ap_rsn_ie_len = ((struct rsn_ie_hdr *) loop_state.ctx->cmd->last_ap_rsn_ie)->len + 2;

	key_data_len = be_to_host16(frame->key_data_len);
	wpa_supplicant_parse_ies(frame->key_data, key_data_len, &ie);

	if(ap_rsn_ie_len != ie.rsn_ie_len) {
		BLTS_ERROR("EAPOL: RSN IE Mismatch - length (%d) not same as AP beacon (%d).\n",
			ie.rsn_ie_len, ap_rsn_ie_len);
		hexdump("Beacon  ", loop_state.ctx->cmd->last_ap_rsn_ie, ap_rsn_ie_len);
		hexdump("Received", ie.rsn_ie, ie.rsn_ie_len);
		return 0;
	}
	if(memcmp(loop_state.ctx->cmd->last_ap_rsn_ie, ie.rsn_ie, ap_rsn_ie_len)) {
		BLTS_ERROR("EAPOL: RSN IE Mismatch - content not identical to beacon.\n");
		hexdump("Beacon  ", loop_state.ctx->cmd->last_ap_rsn_ie, ap_rsn_ie_len);
		hexdump("Received", ie.rsn_ie, ie.rsn_ie_len);
		return 0;
	}

	/* Looks like this frame is ok, so we can save the parsed GTK. */
	if(!ie.gtk || ie.gtk_len != 18) { /* GTK len == 24, minus header (see std) */
		BLTS_ERROR("EAPOL: Invalid GTK in frame\n");
		hexdump("GTK", ie.gtk, ie.gtk_len);
		return 0;
	}
	u8 keyid = ie.gtk[0] & 0x3;
	memcpy(loop_state.sm.GTK[keyid], ie.gtk, ie.gtk_len);
	loop_state.sm.gtk_present |= BIT(keyid);

	BLTS_TRACE("EAPOL: GTK[%d] found in 3 of 4\n", keyid);
	if(ies)
		memcpy(ies, &ie, sizeof(ie));

	return 1;
}

/* Verify EAPOL-KEY MIC using current PTK */
static int eapol_key_frame_mic_ok(struct eapol_key_frame *frame, unsigned frame_len)
{
	u8 frame_mic[16];
	int res = 1;

	memcpy(frame_mic, frame->key_mic, 16);
	memset(frame->key_mic, 0, 16);

	if(wpa_eapol_key_mic(loop_state.sm.PTK, WPA_KEY_INFO_TYPE_HMAC_SHA1_AES,
				(u8*)frame, frame_len, frame->key_mic)) {
		BLTS_ERROR("EAPOL: ERROR - cannot compute RX frame MIC\n");
		res = 0;
		goto done;
	}

	if(memcmp(frame_mic, frame->key_mic, 16)) {
		BLTS_ERROR("EAPOL: WARNING: Invalid MIC in frame, dropping.\n");
		res = 0;
	}
done:
	return res;
}



/* recognize received message #3 of 4way handshake. If so, the parsed
 * IEs in key data are returned in ies. */
static int is_4way_msg_3(struct eapol_key_frame *frame, unsigned frame_len,
			struct wpa_eapol_ie_parse *ies)
{
	u16 ki = be_to_host16(frame->key_info);

	if(frame->desc_type != EAPOL_KEY_TYPE_RSN) {
		BLTS_TRACE("EAPOL: Not RSN key\n");
		return 0;
	}
	if(!((ki & WPA_KEY_INFO_TYPE_MASK) == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES
		&& (ki & WPA_KEY_INFO_KEY_TYPE)
		&& (ki & WPA_KEY_INFO_ACK)
		&& (ki & WPA_KEY_INFO_MIC)
		&& (ki & WPA_KEY_INFO_SECURE)
		&& !(ki & WPA_KEY_INFO_ERROR)
		&& !(ki & WPA_KEY_INFO_REQUEST)
		&& (ki & WPA_KEY_INFO_ENCR_KEY_DATA)
		&& !(ki & WPA_KEY_INFO_SMK_MESSAGE))) {
		BLTS_TRACE("EAPOL: Improper key info for msg 3\n");
		debug_decode_key_info(ki);
		return 0;
	}

	if(frame_len < sizeof (struct eapol_key_frame) + be_to_host16(frame->key_data_len)) {
		BLTS_TRACE("EAPOL: Frame too short for EAPOL 3 of 4\n");
		return 0;
	}
	if(ntohs(frame->key_data_len) < sizeof(struct rsn_ie_hdr)) {
		BLTS_TRACE("EAPOL: Key data block in frame too short\n");
		return 0;
	}

	if(!is_bitfield_zero(frame->eapol_key_iv, 16)) {
		BLTS_TRACE("EAPOL: WARNING: Junk in key IV (should be 0 for v2), check other end\n");
		/* return 0; */
	}
	BLTS_TRACE("EAPOL: Checking frame MIC\n");
	if(!eapol_key_frame_mic_ok(frame, frame_len)) {
		BLTS_ERROR("EAPOL: ERROR - EAPOL-KEY frame with invalid MIC\n");
		return 0;
	}

	BLTS_TRACE("EAPOL: Decrypting potential message 3 of 4-way handshake\n");
	if(wpa_decrypt_eapol_key_data(loop_state.sm.PTK, frame,
					WPA_KEY_INFO_TYPE_HMAC_SHA1_AES)) {
		BLTS_ERROR("EAPOL: Key data decryption failed.\n");
		return 0;
	}

	BLTS_TRACE("EAPOL: Checking message 3 of 4-way handshake security variables\n");
	if(!eapol_3_of_4_security_ok(frame, ies)) {
		BLTS_ERROR("EAPOL: 4-way handshake security failure (bad message 3 from AP). Deauthenticating.\n");
		return -1;
	}

	return 1;
}

/* Security check for eapol group key handshake msg 1. Parses GTK from
 * frame to ies if valid. */
static int eapol_group_hs_security_ok(struct eapol_key_frame *frame,
				struct wpa_eapol_ie_parse *ies)
{
	u64 frame_key_replay_counter;
	u16 key_data_len;
	struct wpa_eapol_ie_parse ie;

	frame_key_replay_counter = WPA_GET_BE64(frame->key_replay_counter);
	if(frame_key_replay_counter <= loop_state.sm.Counter) {
		BLTS_ERROR("EAPOL: WARNING: Key Replay Counter reuse detected - bad frame/bad AP end\n");
		return 0;
	}

	key_data_len = be_to_host16(frame->key_data_len);
	wpa_supplicant_parse_ies(frame->key_data, key_data_len, &ie);

	/* Looks like this frame is ok, so we can save the parsed GTK. */
	if(!ie.gtk || ie.gtk_len != 18) { /* GTK len == 24, minus header (see std) */
		BLTS_ERROR("EAPOL: Invalid GTK in frame\n");
		hexdump("GTK", ie.gtk, ie.gtk_len);
		return 0;
	}
	u8 keyid = ie.gtk[0] & 0x3;
	memcpy(loop_state.sm.GTK[keyid], ie.gtk, ie.gtk_len);
	loop_state.sm.gtk_present |= BIT(keyid);

	BLTS_TRACE("EAPOL: GTK[%d] found in group key handshake\n", keyid);
	if(ies)
		memcpy(ies, &ie, sizeof(ie));

	/* Update replay counter, since we know this was a valid
	 * message. This value must be used in msg 2. */
	loop_state.sm.Counter = frame_key_replay_counter;

	return 1;
}

/* Recognise group handshake start frame */
static int is_group_hs_msg_1(struct eapol_key_frame *frame, unsigned frame_len, struct wpa_eapol_ie_parse *ies)
{
	u16 ki = be_to_host16(frame->key_info);

	if(frame->desc_type != EAPOL_KEY_TYPE_RSN) {
		BLTS_TRACE("EAPOL: Not RSN key\n");
		return 0;
	}
	if(!((ki & WPA_KEY_INFO_TYPE_MASK) == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES
		&& !(ki & WPA_KEY_INFO_KEY_TYPE)
		&& !(ki & WPA_KEY_INFO_SMK_MESSAGE)
		&& !(ki & WPA_KEY_INFO_INSTALL)
		&& (ki & WPA_KEY_INFO_ACK)
		&& (ki & WPA_KEY_INFO_MIC)
		&& (ki & WPA_KEY_INFO_SECURE)
		&& !(ki & WPA_KEY_INFO_ERROR)
		&& !(ki & WPA_KEY_INFO_REQUEST)
		&& (ki & WPA_KEY_INFO_ENCR_KEY_DATA))) {
		BLTS_TRACE("EAPOL: Improper key info for group handshake msg 1\n");
		debug_decode_key_info(ki);
		return 0;
	}

	if(!is_bitfield_zero(frame->key_nonce, WPA_NONCE_LEN)) {
		BLTS_TRACE("EAPOL: Nonce non-empty, discarding frame\n");
		return 0;
	}
	if(!is_bitfield_zero(frame->eapol_key_iv, 16)) {
		BLTS_TRACE("EAPOL: Junk in key IV (should be 0 for v2), check other end\n");
		return 0;
	}

	/* Note: In a real implementation, we'd also check that key
	 * RSC equals TX sequence number for the GTK here (as per
	 * standard). TODO: add ability to do this. */

	BLTS_TRACE("EAPOL: Checking frame MIC\n");
	if(!eapol_key_frame_mic_ok(frame, frame_len)) {
		BLTS_ERROR("EAPOL: ERROR - EAPOL-KEY frame with invalid MIC\n");
		return 0;
	}

	BLTS_TRACE("EAPOL: Decrypting potential group key from handshake\n");
	if(wpa_decrypt_eapol_key_data(loop_state.sm.PTK, frame,
					WPA_KEY_INFO_TYPE_HMAC_SHA1_AES)) {
		BLTS_ERROR("EAPOL: Key data decryption failed.\n");
		return 0;
	}

	BLTS_TRACE("EAPOL: Checking group handshake security variables\n");
	if(!eapol_group_hs_security_ok(frame, ies)) {
		BLTS_ERROR("EAPOL: Group handshake security failure (bad message 1 from AP). Deauthenticating.\n");
		return -1;
	}

	return 1;
}

/* Store data from AP that's needed later. */
static void eapol_save_1_of_4_data(struct eapol_key_frame *frame)
{
	loop_state.sm.Counter = WPA_GET_BE64(frame->key_replay_counter);
	memcpy(loop_state.sm.ANonce, frame->key_nonce, WPA_NONCE_LEN);
}

/* process 4way handshake start and send message #2 */
static int process_eapol_4way_msg_1(struct eapol_key_frame *frame, unsigned frame_len)
{
	struct eapol_key_frame *frame2;
	u8 *packet, *pos;
	u16 host_key_info;
	int ret = 0;
	size_t msg2_len, msg2_body_len;

	if(!loop_state.ctx || !loop_state.ctx->cmd) {
		BLTS_ERROR("BUG: bad state\n");
		return -1;
	}
	if(!loop_state.ctx->cmd->rsn_ie
		|| !loop_state.ctx->cmd->associated) {
		BLTS_ERROR("BUG: Not properly associated but still in 4way handshake!\n");
		return -1;
	}
	msg2_len = sizeof(struct eapol_key_frame) + loop_state.ctx->cmd->rsn_ie_len;

	/* "packet body" == bits from desc_type to key data end */
	msg2_body_len = msg2_len - (1 + 1 + 2);

	packet = malloc(msg2_len);
	pos = packet;
	memset(packet, 0, msg2_len);

	if(!packet)
		return -ENOMEM;

	frame2 = (struct eapol_key_frame *) packet;

	eapol_save_1_of_4_data(frame);

	memcpy(frame2, frame, sizeof(struct eapol_key_frame));

	frame2->protocol_version = 2;
	frame2->packet_body_len = host_to_be16(msg2_body_len);

	WPA_PUT_BE64(frame2->key_replay_counter, loop_state.sm.Counter);

	ret = rsna_nonce_generator(loop_state.ctx->our_addr, loop_state.sm.SNonce);
	if(ret) {
		BLTS_ERROR("EAPOL: Nonce generation error\n");
		goto done;
	}
	memcpy(frame2->key_nonce, loop_state.sm.SNonce, 32);

	loop_state.sm.ptk_len = 48; /* CCMP */

	wpa_pmk_to_ptk(loop_state.ctx->cmd->wpa_pmk, 32,
		"Pairwise key expansion",
		loop_state.ctx->our_addr,
		loop_state.ctx->their_addr,
		loop_state.sm.ANonce, loop_state.sm.SNonce,
		loop_state.sm.PTK, loop_state.sm.ptk_len);

	host_key_info = 0;
	host_key_info |= WPA_KEY_INFO_TYPE_HMAC_SHA1_AES;
	host_key_info |= WPA_KEY_INFO_KEY_TYPE;
	host_key_info |= WPA_KEY_INFO_MIC;

	frame2->key_info = host_to_be16(host_key_info);
	frame2->key_len = 0;

	memset(frame2->eapol_key_iv, 0, sizeof(frame2->eapol_key_iv));
	memset(frame2->key_rsc, 0, sizeof(frame2->key_rsc));
	frame2->key_data_len = host_to_be16(loop_state.ctx->cmd->rsn_ie_len);
	memset(frame2->key_mic, 0, sizeof(frame2->key_mic));

	pos += sizeof(struct eapol_key_frame);

	memcpy(pos, loop_state.ctx->cmd->rsn_ie,
		loop_state.ctx->cmd->rsn_ie_len);

	ret = wpa_eapol_key_mic(loop_state.sm.PTK,
		WPA_KEY_INFO_TYPE_HMAC_SHA1_AES,
		(u8*) packet, msg2_len, frame2->key_mic);

 	if(ret) {
		BLTS_ERROR("EAPOL: Key MIC compute error\n");
		goto done;
	}
	BLTS_TRACE("EAPOL: 4-way auth message 2 ready:\n");
	eapol_debug_dump_frame(packet, msg2_len);

	ret = eapol_send_key_frame(loop_state.ctx, packet, msg2_len);
 	if(ret < 0) {
		BLTS_ERROR("EAPOL: EAPOL-KEY frame send error\n");
		goto done;
	}
	BLTS_TRACE("EAPOL: TX total frame=%d\n", ret);
	BLTS_DEBUG("EAPOL: TX 4-way auth message 2\n");

	ret = eapol_auth_state_change(AUTH_STATE_4WAY_2_SENT);

done:
	free(packet);
	return ret;
}

/* process 4way handshake message #3 and send message #4, completing handshake */

/* Note: We have a fully decrypted frame available at this point (and
 * we know it's valid), with keys collected in sm state.
 */
static int process_eapol_4way_msg_3(struct eapol_key_frame *frame,
		unsigned frame_len, const struct wpa_eapol_ie_parse *ies)
{
	struct eapol_key_frame *msg4;
	u16 host_key_info;
	size_t msg4_len, msg4_body_len;
	int ret = 0;

	if(!loop_state.ctx || !loop_state.ctx->cmd) {
		BLTS_ERROR("BUG: bad state\n");
		return -1;
	}
	if(!loop_state.ctx->cmd->associated) {
		BLTS_ERROR("BUG: Not properly associated but still in 4way handshake!\n");
		return -1;
	}

	/* update key replay counter (we verified this in the security
	 * check) */
	loop_state.sm.Counter = WPA_GET_BE64(frame->key_replay_counter);

	/* construct msg #4 */

	msg4_len = sizeof(struct eapol_key_frame);
	/* "packet body" == bits from desc_type to key data end */
	msg4_body_len = msg4_len - (1 + 1 + 2);

	msg4 = malloc(msg4_len);
	if(!msg4)
		return -ENOMEM;
	memset(msg4, 0, msg4_len);

	msg4->protocol_version = 2;
	msg4->packet_type = 3; /* EAPOL-KEY */
	msg4->packet_body_len = host_to_be16(msg4_body_len);
	msg4->desc_type = EAPOL_KEY_TYPE_RSN;

	host_key_info = 0;
	host_key_info |= WPA_KEY_INFO_TYPE_HMAC_SHA1_AES;
	host_key_info |= WPA_KEY_INFO_KEY_TYPE;
	host_key_info |= WPA_KEY_INFO_MIC;
	host_key_info |= WPA_KEY_INFO_SECURE;

	msg4->key_info = host_to_be16(host_key_info);
	WPA_PUT_BE64(msg4->key_replay_counter, loop_state.sm.Counter);

	ret = wpa_eapol_key_mic(loop_state.sm.PTK,
			WPA_KEY_INFO_TYPE_HMAC_SHA1_AES,
			(u8*) msg4, msg4_len, msg4->key_mic);
 	if(ret) {
		BLTS_ERROR("EAPOL: Key MIC compute error\n");
		goto done;
	}

	/* send msg #4 */

	BLTS_TRACE("EAPOL: 4-way auth message 4 ready:\n");
	eapol_debug_dump_frame((u8*)msg4, msg4_len);

	ret = eapol_send_key_frame(loop_state.ctx, msg4, msg4_len);
 	if(ret < 0) {
		BLTS_ERROR("EAPOL: EAPOL-KEY frame send error\n");
		goto done;
	}
	BLTS_TRACE("EAPOL: TX total frame=%d\n", ret);
	BLTS_DEBUG("EAPOL: TX 4-way auth message 4\n");

	ret = eapol_auth_state_change(AUTH_STATE_4WAY_COMPLETE);
	if(ret)
		goto done;

	/* install PTK and GTK */
	BLTS_TRACE("EAPOL: Installing keys\n");

	if(be_to_host16(frame->key_info) & WPA_KEY_INFO_INSTALL) {
		ret = eapol_install_ptk(frame);
		if(ret) {
			BLTS_ERROR("EAPOL: PTK install failed\n");
			goto done;
		}
	}

	if(be_to_host16(frame->key_info) & WPA_KEY_INFO_SECURE) {
		/* Some non-mac80211 drivers need to be told to use
		 * MLME_SETPROTECTION here, not the case for us. */
		loop_state.sm.PortValid = 1;
	}

	/* Install group key, set up for general data traffic.  */
	ret = eapol_set_pairwise_gtk(frame, ies->gtk, ies->gtk_len);
	if(ret) {
		BLTS_ERROR("EAPOL: ERROR - Group key not installed, cannot continue.\n");
		goto done;
	}

	/* Authorize port for general data */
	ret = eapol_set_port_auth(1);
	if(ret)
		goto done;

	ret = eapol_auth_state_change(AUTH_STATE_GROUP_HS_READY);
	if(ret)
		goto done;

done:
	free(msg4);
	return ret;
}


/* Change group key and complete group handshake */
static int process_eapol_group_hs_msg1(struct eapol_key_frame *frame,
		unsigned frame_len, const struct wpa_eapol_ie_parse *ies)
{
	struct eapol_key_frame *msg2;
	u16 host_key_info;
	size_t msg2_len, msg2_body_len;
	int ret = 0;

	if(!loop_state.ctx || !loop_state.ctx->cmd) {
		BLTS_ERROR("BUG: bad state\n");
		return -1;
	}

	ret = eapol_set_pairwise_gtk(frame, ies->gtk, ies->gtk_len);
	if(ret) {
		BLTS_ERROR("EAPOL: ERROR - Group key not installed, cannot continue.\n");
		return ret;
	}

	msg2_len = sizeof(struct eapol_key_frame);
	/* "packet body" == bits from desc_type to key data end */
	msg2_body_len = msg2_len - (1 + 1 + 2);

	msg2 = malloc(msg2_len);
	if(!msg2)
		return -ENOMEM;
	memset(msg2, 0, msg2_len);

	msg2->protocol_version = 2;
	msg2->packet_type = 3; /* EAPOL-KEY */
	msg2->packet_body_len = host_to_be16(msg2_body_len);
	msg2->desc_type = EAPOL_KEY_TYPE_RSN;

	host_key_info = 0;
	host_key_info |= WPA_KEY_INFO_TYPE_HMAC_SHA1_AES;
	host_key_info |= WPA_KEY_INFO_MIC;
	host_key_info |= WPA_KEY_INFO_SECURE;

	msg2->key_info = host_to_be16(host_key_info);
	WPA_PUT_BE64(msg2->key_replay_counter, loop_state.sm.Counter);

	ret = wpa_eapol_key_mic(loop_state.sm.PTK,
			WPA_KEY_INFO_TYPE_HMAC_SHA1_AES,
			(u8*) msg2, msg2_len, msg2->key_mic);
 	if(ret) {
		BLTS_ERROR("EAPOL: Key MIC compute error\n");
		goto done;
	}

	BLTS_TRACE("EAPOL: Group handshake completion message ready:\n");
	eapol_debug_dump_frame((u8*)msg2, msg2_len);

	ret = eapol_send_key_frame(loop_state.ctx, msg2, msg2_len);
 	if(ret < 0) {
		BLTS_ERROR("EAPOL: EAPOL-KEY frame send error\n");
		goto done;
	}
	BLTS_TRACE("EAPOL: TX total frame=%d\n", ret);
	BLTS_DEBUG("EAPOL: TX Group handshake message 2\n");

	ret = eapol_set_port_auth(1);
	if(ret)
		goto done;

	/* Finally, increment replay counter. Any retries from AP will
	 * do the same if the frame we sent was lost. */
	loop_state.sm.Counter++;

	ret = eapol_auth_state_change(AUTH_STATE_GROUP_HS_READY);
	if(ret)
		goto done;

done:
	free(msg2);
	return ret;
}


/* Main dispatch for EAPOL-KEY processing. Expects the frame to be
 * some variation of EAPOL proto, further checks go in state-specific
 * code. Call with lock. */
static int process_eapol_frame(struct eapol_key_frame *frame, unsigned frame_len)
{
	int ret = 0;
	struct wpa_eapol_ie_parse ies = {};

	if(!frame || frame_len < sizeof (*frame)) {
		BLTS_TRACE("EAPOL: Bad frame (%p)(len=%u) - ignoring frame\n",frame,frame_len);
		return -EINVAL;
	}

	if(!loop_state.ctx->cmd->associated) {
		BLTS_TRACE("EAPOL: Not associated - ignoring frame\n");
		return 0;
	}

	if(frame->protocol_version != 2) {
		BLTS_ERROR("EAPOL: WARNING: Frame with unsupported protocol version (%d); Check AP\n",
			frame->protocol_version);
		/* return 0; */
	}

	switch(loop_state.eapol_auth_state) {
	case AUTH_STATE_PREAUTH:
		/* expecting 4way handshake start */
		if(!is_4way_msg_1(frame, frame_len)) {
			BLTS_TRACE("EAPOL: Not 4-Way handshake start, ignoring frame\n");
			return 0;
		}
		BLTS_DEBUG("EAPOL: RX 4-way auth message 1\n");
		eapol_debug_dump_frame((void*)frame, frame_len);
		ret = eapol_auth_state_change(AUTH_STATE_4WAY_1_RCVD);
		if(ret)
			break;
		ret = process_eapol_4way_msg_1(frame, frame_len);
		break;
	case AUTH_STATE_4WAY_1_RCVD:
		/* not waiting for anything */
		BLTS_TRACE("EAPOL: In AUTH_STATE_4WAY_1_RCVD -> ignoring frame\n");
		return 0;
	case AUTH_STATE_4WAY_2_SENT:
		/* expecting 4way handshake message #3 */
		ret = is_4way_msg_3(frame, frame_len, &ies);
		if(ret == 0) {
			BLTS_TRACE("EAPOL: Not 4-Way handshake message #3, ignoring frame\n");
			return 0;
		} else if (ret < 0) {
			BLTS_TRACE("EAPOL: Deautheticating due to security failure -> disconnect.\n");
			break;
		}
		BLTS_DEBUG("EAPOL: RX 4-way auth message 3\n");
		eapol_debug_dump_frame((void*)frame, frame_len);
		ret = eapol_auth_state_change(AUTH_STATE_4WAY_3_RCVD);
		if(ret)
			break;
		ret = process_eapol_4way_msg_3(frame, frame_len, &ies);
		break;
	case AUTH_STATE_4WAY_3_RCVD:
		/* not waiting for anything */
		BLTS_TRACE("EAPOL: In AUTH_STATE_4WAY_3_RCVD -> ignoring frame\n");
		return 0;
	case AUTH_STATE_4WAY_COMPLETE:
		BLTS_TRACE("EAPOL: In AUTH_STATE_4WAY_4_COMPLETE -> busy installing keys, ignoring frame\n");
		return 0;
	case AUTH_STATE_GROUP_HS_READY:
		/* At this point, we mainly expect disconnects or
		 * rekeying. Std says we must have been deauthed
		 * and/or disassociated in that case. New KX uses
		 * either 4-way hs + group hs or just group hs,
		 * depending on what's needed. The group hs never
		 * starts before 4-way completes -> we can just revert
		 * to earlier state here if 4-way is needed. */
		if(is_4way_msg_1(frame, frame_len)) {
			BLTS_DEBUG("EAPOL: RX 4-way auth message 1 -- Rekeying\n");
			ret = eapol_auth_state_change(AUTH_STATE_4WAY_1_RCVD);
			if(ret)
				break;
			ret = process_eapol_4way_msg_1(frame, frame_len);
			break;
		}
		ret = is_group_hs_msg_1(frame, frame_len, &ies);
		if(ret == 0) {
			BLTS_TRACE("EAPOL: Ignoring unexpected EAPOL frame\n");
			return 0;
		} else if (ret < 0) {
			BLTS_TRACE("EAPOL: Deautheticating due to security failure -> disconnect.\n");
			break;
		}

		BLTS_DEBUG("EAPOL: RX Group handshake message 1 -- continuing rekeying\n");
		eapol_debug_dump_frame((void*)frame, frame_len);
		ret = eapol_auth_state_change(AUTH_STATE_GROUP_HS_1_RCVD);
		if(ret)
			break;

		ret = process_eapol_group_hs_msg1(frame, frame_len, &ies);
		break;
	default:
		BLTS_ERROR("BUG: EAPOL: Unknown auth state (internal error)\n");
		return -1;
	}

	return ret;
}

struct raw_packet_header {
	u8 a1[ETH_ALEN];
	u8 a2[ETH_ALEN];
	u16 proto;
} __attribute__((packed));

/* Called from the eapol select() loop; reads immediately incoming
 * frame and hands it to processing. */
static int eapol_sock_handle_read()
{
	int ret = 0;
	u8 buf[4096];
	u8 *raw_frame;
	struct raw_packet_header *packet_hdr;
	unsigned frame_len = 0;

	pthread_mutex_lock(&loop_state_lock);

	ret = recv(loop_state.eapol_sock, buf, sizeof(buf), MSG_DONTWAIT);

	if(ret < 0 && (errno == EAGAIN || errno == EINTR)) {
		ret = 0;
		goto done;
	}
	if(!ret)
		goto done;

	if(!loop_state.ctx->monitor_alternate_in_use) {
		ret = monitor_extract_frame(loop_state.ctx, buf, ret, &raw_frame, &frame_len);
		if(ret)
			goto done;

		if(!is_eapol_frame(raw_frame, frame_len))
			goto done;

		raw_frame += sizeof(struct ieee80211_data_header_for_8021x);
		frame_len -= sizeof(struct ieee80211_data_header_for_8021x);
	} else {
		if(ret < sizeof(struct raw_packet_header)) {
			ret = 0;
			goto done;
		}
		packet_hdr = (struct raw_packet_header *) buf;
		if(packet_hdr->proto != htons(ETH_P_EAPOL)) {
			ret = 0;
			goto done;
		}
		raw_frame = buf + sizeof(struct raw_packet_header);
		frame_len = ret - sizeof(struct raw_packet_header);
	}

	/* eapol_debug_dump_frame(raw_frame, frame_len); */
	ret = process_eapol_frame((struct eapol_key_frame *) raw_frame, frame_len);
done:

	pthread_mutex_unlock(&loop_state_lock);
	return ret;
}

/* OOB branch from select loop. Never called in reality since this is not a TTY. */
static int eapol_sock_handle_oob()
{
	BLTS_DEBUG("EAPOL:OOB\n");

	return 0;
}

/* Main eapol select() loop, waits for incoming frames on monitor interface */
static void *eapol_thread_fn(void *ptr)
{
	fd_set rfd, wfd, xfd;
	int err;
	for(;;) {
		pthread_mutex_lock(&loop_state_lock);

		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_ZERO(&xfd);
		FD_SET(loop_state.eapol_sock, &rfd);
		FD_SET(loop_state.eapol_sock, &xfd);

		pthread_mutex_unlock(&loop_state_lock);

		err = select(loop_state.eapol_sock + 1, &rfd, &wfd, &xfd, NULL);

		if(err < 0 && errno == EAGAIN)
			continue;
		if(err < 0 && errno == EINTR)
			break;
		if(err < 0) {
			BLTS_ERROR("EAPOL: Error during select(), terminating.\n");
			break;
		}

		if(FD_ISSET(loop_state.eapol_sock, &xfd)) {
			err = eapol_sock_handle_oob();
			if(err < 0) {
				BLTS_ERROR("EAPOL: Cannot continue, terminating.\n");
				break;
			}
		}
		if(FD_ISSET(loop_state.eapol_sock, &rfd)) {
			err = eapol_sock_handle_read();
			if(err < 0) {
		/* Note: we'll get here from:
		 * 1) general read errors
		 * 2) test failures
		 * 3) places where standard says we must deauth (usually due
		 *    to security failure)
		 * 4) internal errors.
		 * 5) state change callbacks
		 */
				BLTS_ERROR("EAPOL: Terminating.\n");
				break;
			}
		}
	}

	pthread_mutex_lock(&loop_state_lock);
	loop_state.loop_tid = 0;
	pthread_mutex_unlock(&loop_state_lock);

	pthread_exit(ptr);
	return NULL;
}

/* Call with lock held. Note that in the future we may come here from deauth in
   the state machine also. */
static int eapol_sm_init(wlan_core_data *data)
{
	/* loop_state.sm.ksm_state = EAPOL_SUPP_KM_STATE_INITIALIZE; */
	loop_state.sm.AuthenticationFailed = 0;
	loop_state.sm.keycount = 0;
	loop_state.sm.msk_len = 0;
	loop_state.sm.PortEnabled = 0;
	loop_state.sm.PortValid = 0;
	loop_state.eapol_auth_state = AUTH_STATE_PREAUTH;
	return 0;
}


/* call with lock held */
static int eapol_loop_init(wlan_core_data *data)
{
	int ret;
	BLTS_TRACE("EAPOL init()\n");

	ret = monitor_sock_setup(data);
	if(ret) {
		BLTS_ERROR("EAPOL monitor socket init failed.\n");
		goto done;
	}
	loop_state.eapol_sock = data->monitor_sock;
	loop_state.ctx = data;

	ret = eapol_sm_init(data);
	if(ret) {
		BLTS_ERROR("EAPOL State machine init failed\n");
		goto done;
	}
	if(loop_state.loop_tid) {
		BLTS_ERROR("BUG: trying to start EAPOL thread a second time\n");
		ret = -1;
		goto done;
	}

	ret = pthread_create(&loop_state.loop_tid, NULL, eapol_thread_fn, &loop_state);
	if(ret) {
		BLTS_ERROR("Error: failed to start EAPOL communication thread\n");
		goto done;
	}
done:
	return ret;
}

/* call with lock held */
static void eapol_loop_destroy(wlan_core_data *data)
{
	BLTS_TRACE("EAPOL destroy()\n");

	if(loop_state.loop_tid) {
		pthread_cancel(loop_state.loop_tid);
		pthread_join(loop_state.loop_tid, NULL);
	}
	loop_state.loop_tid = 0;
	loop_state.ctx = NULL;
	monitor_sock_cleanup(data);
}

/* Public interface for starting eapol select() loop. */
int eapol_loop_run(wlan_core_data *data, void *user_data)
{
	int ret = 0;

	pthread_mutex_lock(&loop_state_lock);

	if(!loop_state.refcnt) {
		ret = eapol_loop_init(data);
		if(ret) {
			BLTS_ERROR("EAPOL init failed.\n");
			goto done;
		}
	}

	loop_state.refcnt++;

done:
	pthread_mutex_unlock(&loop_state_lock);
	return 0;
}

/* Public interface for releasing previously started loop. Last unref
 * causes loop termination. */
void eapol_loop_unref(wlan_core_data *data)
{
	pthread_mutex_lock(&loop_state_lock);
	if(!loop_state.refcnt) {
		BLTS_TRACE("eapol_loop_unref(): not destroying non-ref'd loop\n");
		goto done;
	}

	loop_state.refcnt--;

	if(!loop_state.refcnt)
		eapol_loop_destroy(data);
done:
	pthread_mutex_unlock(&loop_state_lock);
}




/* Adapted from hostapd */
/* Generate the RSN IE for a WPA frame from given data (see std) */
int wpa_gen_wpa_ie_rsn(u8 *rsn_ie, size_t rsn_ie_len,
		int pairwise_cipher, int group_cipher,
		int key_mgmt, int mgmt_group_cipher)
{
	u8 *pos;
	struct rsn_ie_hdr *hdr;
	u16 capab;
	size_t ie_len;
	ie_len = sizeof(*hdr) + RSN_SELECTOR_LEN +
		2 + RSN_SELECTOR_LEN + 2 + RSN_SELECTOR_LEN + 2 + 0;

	if (rsn_ie_len < ie_len) {
		BLTS_ERROR("BUG: Too short IE buffer (%lu bytes)",
			   (unsigned long) rsn_ie_len);
		return -1;
	}

	hdr = (struct rsn_ie_hdr *) rsn_ie;
	hdr->elem_id = WLAN_EID_RSN;
	WPA_PUT_LE16(hdr->version, RSN_VERSION);
	pos = (u8 *) (hdr + 1);

	if (group_cipher == WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP);
	} else if (group_cipher == WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_TKIP);
	} else if (group_cipher == WPA_CIPHER_WEP104) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_WEP104);
	} else if (group_cipher == WPA_CIPHER_WEP40) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_WEP40);
	} else {
		BLTS_ERROR("BUG: Invalid group cipher (%d).",
			   group_cipher);
		return -1;
	}
	pos += RSN_SELECTOR_LEN;

	*pos++ = 1;
	*pos++ = 0;
	if (pairwise_cipher == WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP);
	} else if (pairwise_cipher == WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_TKIP);
	} else if (pairwise_cipher == WPA_CIPHER_NONE) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_NONE);
	} else {
		BLTS_ERROR("BUG: Invalid pairwise cipher (%d).",
			   pairwise_cipher);
		return -1;
	}
	pos += RSN_SELECTOR_LEN;

	*pos++ = 1;
	*pos++ = 0;
	if (key_mgmt == WPA_KEY_MGMT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_UNSPEC_802_1X);
	} else if (key_mgmt == WPA_KEY_MGMT_PSK) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X);
#ifdef CONFIG_IEEE80211R
	} else if (key_mgmt == WPA_KEY_MGMT_FT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_FT_802_1X);
	} else if (key_mgmt == WPA_KEY_MGMT_FT_PSK) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_FT_PSK);
#endif /* CONFIG_IEEE80211R */
#ifdef CONFIG_IEEE80211W
	} else if (key_mgmt == WPA_KEY_MGMT_IEEE8021X_SHA256) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_802_1X_SHA256);
	} else if (key_mgmt == WPA_KEY_MGMT_PSK_SHA256) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_PSK_SHA256);
#endif /* CONFIG_IEEE80211W */
	} else {
		BLTS_ERROR("BUG: Invalid key management type (%d).",
			   key_mgmt);
		return -1;
	}
	pos += RSN_SELECTOR_LEN;

	/* RSN Capabilities */
	capab = 0;
#ifdef CONFIG_IEEE80211W
	/* if (sm->mfp) */
	/* 	capab |= WPA_CAPABILITY_MFPC; */
	/* if (sm->mfp == 2) */
	/* 	capab |= WPA_CAPABILITY_MFPR; */
#endif /* CONFIG_IEEE80211W */
	WPA_PUT_LE16(pos, capab);
	pos += 2;

	/* if (sm->cur_pmksa) { */
	/* 	/\* PMKID Count (2 octets, little endian) *\/ */
	/* 	*pos++ = 1; */
	/* 	*pos++ = 0; */
	/* 	/\* PMKID *\/ */
	/* 	os_memcpy(pos, sm->cur_pmksa->pmkid, PMKID_LEN); */
	/* 	pos += PMKID_LEN; */
	/* } */

#ifdef CONFIG_IEEE80211W
	/* if (mgmt_group_cipher == WPA_CIPHER_AES_128_CMAC) { */
	/* 	if (!sm->cur_pmksa) { */
	/* 		/\* PMKID Count *\/ */
	/* 		WPA_PUT_LE16(pos, 0); */
	/* 		pos += 2; */
	/* 	} */

	/* 	/\* Management Group Cipher Suite *\/ */
	/* 	RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_AES_128_CMAC); */
	/* 	pos += RSN_SELECTOR_LEN; */
	/* } */
#endif /* CONFIG_IEEE80211W */

	hdr->len = (pos - rsn_ie) - 2;

	assert((size_t) (pos - rsn_ie) <= rsn_ie_len);

	return pos - rsn_ie;
}


/* hostapd rsn_supp/wpa_ie.[ch] */


/**
 * wpa_parse_generic - Parse EAPOL-Key Key Data Generic IEs
 * @pos: Pointer to the IE header
 * @end: Pointer to the end of the Key Data buffer
 * @ie: Pointer to parsed IE data
 * Returns: 0 on success, 1 if end mark is found, -1 on failure
 */
static int wpa_parse_generic(const u8 *pos, const u8 *end,
			     struct wpa_eapol_ie_parse *ie)
{
	if (pos[1] == 0)
		return 1;

	if (pos[1] >= 6 &&
	    RSN_SELECTOR_GET(pos + 2) == WPA_OUI_TYPE &&
	    pos[2 + WPA_SELECTOR_LEN] == 1 &&
	    pos[2 + WPA_SELECTOR_LEN + 1] == 0) {
		ie->wpa_ie = pos;
		ie->wpa_ie_len = pos[1] + 2;
		hexdump("EAPOL: WPA IE in EAPOL-Key",
			    ie->wpa_ie, ie->wpa_ie_len);
		return 0;
	}

	if (pos + 1 + RSN_SELECTOR_LEN < end &&
	    pos[1] >= RSN_SELECTOR_LEN + PMKID_LEN &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_PMKID) {
		ie->pmkid = pos + 2 + RSN_SELECTOR_LEN;
		hexdump("EAPOL: PMKID in EAPOL-Key",
			    pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_GROUPKEY) {
		ie->gtk = pos + 2 + RSN_SELECTOR_LEN;
		ie->gtk_len = pos[1] - RSN_SELECTOR_LEN;
		hexdump("EAPOL: GTK in EAPOL-Key",
				pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_MAC_ADDR) {
		ie->mac_addr = pos + 2 + RSN_SELECTOR_LEN;
		ie->mac_addr_len = pos[1] - RSN_SELECTOR_LEN;
		hexdump("EAPOL: MAC Address in EAPOL-Key",
			    pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_SMK) {
		ie->smk = pos + 2 + RSN_SELECTOR_LEN;
		ie->smk_len = pos[1] - RSN_SELECTOR_LEN;
		hexdump("EAPOL: SMK in EAPOL-Key",
				pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_NONCE) {
		ie->nonce = pos + 2 + RSN_SELECTOR_LEN;
		ie->nonce_len = pos[1] - RSN_SELECTOR_LEN;
		hexdump("EAPOL: Nonce in EAPOL-Key",
			    pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_LIFETIME) {
		ie->lifetime = pos + 2 + RSN_SELECTOR_LEN;
		ie->lifetime_len = pos[1] - RSN_SELECTOR_LEN;
		hexdump("EAPOL: Lifetime in EAPOL-Key",
			    pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_ERROR) {
		ie->error = pos + 2 + RSN_SELECTOR_LEN;
		ie->error_len = pos[1] - RSN_SELECTOR_LEN;
		hexdump("EAPOL: Error in EAPOL-Key",
			    pos, pos[1] + 2);
		return 0;
	}
	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_IGTK) {
		ie->igtk = pos + 2 + RSN_SELECTOR_LEN;
		ie->igtk_len = pos[1] - RSN_SELECTOR_LEN;
		hexdump("EAPOL: IGTK in EAPOL-Key",
				pos, pos[1] + 2);
		return 0;
	}

	return 0;
}



/**
 * wpa_supplicant_parse_ies - Parse EAPOL-Key Key Data IEs
 * @buf: Pointer to the Key Data buffer
 * @len: Key Data Length
 * @ie: Pointer to parsed IE data
 * Returns: 0 on success, -1 on failure
 */
static int wpa_supplicant_parse_ies(const u8 *buf, size_t len,
			     struct wpa_eapol_ie_parse *ie)
{
	const u8 *pos, *end;
	int ret = 0;

	memset(ie, 0, sizeof(*ie));
	for (pos = buf, end = pos + len; pos + 1 < end; pos += 2 + pos[1]) {
		if (pos[0] == 0xdd &&
		    ((pos == buf + len - 1) || pos[1] == 0)) {
			/* Ignore padding */
			break;
		}
		if (pos + 2 + pos[1] > end) {
			BLTS_TRACE("EAPOL: EAPOL-Key Key Data "
				   "underflow (ie=%d len=%d pos=%d)",
				   pos[0], pos[1], (int) (pos - buf));
			hexdump("EAPOL: Key Data",
					buf, len);
			ret = -1;
			break;
		}
		if (*pos == WLAN_EID_RSN) {
			ie->rsn_ie = pos;
			ie->rsn_ie_len = pos[1] + 2;
			hexdump("EAPOL: RSN IE in EAPOL-Key",
				    ie->rsn_ie, ie->rsn_ie_len);
		} else if (*pos == WLAN_EID_MOBILITY_DOMAIN) {
			ie->mdie = pos;
			ie->mdie_len = pos[1] + 2;
			hexdump("EAPOL: MDIE in EAPOL-Key",
				    ie->mdie, ie->mdie_len);
		} else if (*pos == WLAN_EID_FAST_BSS_TRANSITION) {
			ie->ftie = pos;
			ie->ftie_len = pos[1] + 2;
			hexdump("EAPOL: FTIE in EAPOL-Key",
				    ie->ftie, ie->ftie_len);
		} else if (*pos == WLAN_EID_TIMEOUT_INTERVAL && pos[1] >= 5) {
			if (pos[2] == WLAN_TIMEOUT_REASSOC_DEADLINE) {
				ie->reassoc_deadline = pos;
				hexdump("EAPOL: Reassoc Deadline "
					    "in EAPOL-Key",
					    ie->reassoc_deadline, pos[1] + 2);
			} else if (pos[2] == WLAN_TIMEOUT_KEY_LIFETIME) {
				ie->key_lifetime = pos;
				hexdump("EAPOL: KeyLifetime "
					    "in EAPOL-Key",
					    ie->key_lifetime, pos[1] + 2);
			} else {
				hexdump("EAPOL: Unrecognized "
					    "EAPOL-Key Key Data IE",
					    pos, 2 + pos[1]);
			}
		} else if (*pos == WLAN_EID_VENDOR_SPECIFIC) {
			ret = wpa_parse_generic(pos, end, ie);
			if (ret < 0)
				break;
			if (ret > 0) {
				ret = 0;
				break;
			}
		} else {
			hexdump("EAPOL: Unrecognized EAPOL-Key "
				    "Key Data IE", pos, 2 + pos[1]);
		}
	}

	return ret;
}


/* Callbacks registered through this are called for each state
 * transition in the EAPOL code. "auth_status" gets new state,
 * "user_data" the pointer "data". Callback return != 0 -> terminate
 * EAPOL.  TODO (when needed): allow any number of callbacks */
int eapol_register_callback(int (*on_change_cb)(int auth_status, void *user_data), void *data)
{
	loop_state.state_change_cb.on_change_cb = on_change_cb;
	loop_state.state_change_cb.data = data;
	return 0;
}

/* Remove previously registered callback for state transitions */
int eapol_unregister_callback(int (*on_change_cb)(int auth_status, void *user_data))
{
	loop_state.state_change_cb.on_change_cb = NULL;
	loop_state.state_change_cb.data = NULL;
	return 0;
}


/* Mostly for debugging; sync this with the states in wlan-core-eapol.h */
const char *eapol_auth_state_as_string(int state)
{
	static const char *states[] = {
		"AUTH_STATE_PREAUTH",
		"AUTH_STATE_4WAY_1_RCVD",
		"AUTH_STATE_4WAY_2_SENT",
		"AUTH_STATE_4WAY_3_RCVD",
		"AUTH_STATE_4WAY_COMPLETE",
		"AUTH_STATE_GROUP_HS_READY",
		"AUTH_STATE_GROUP_HS_1_RCVD",
		"AUTH_STATE_GROUP_HS_COMPLETE",
		"AUTH_STATE_AUTHENTICATED",
		"AUTH_STATE_DEAUTHENTICATED",
	};
	return states[state];
}

