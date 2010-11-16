/* wlan-core-wpa.c -- WPA connect test

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

#include <blts_log.h>
#include <pthread.h>

#include "wlan-core-definitions.h"
#include "wlan-core-connect.h"
#include "wlan-core-scan.h"
#include "wlan-core-dhcp.h"
#include "wlan-core-debug.h"
#include "wlan-core-utils.h"
#include "wlan-core-monitor.h"
#include "wlan-core-eapol.h"
#include "wlan-core-crypto.h"

pthread_mutex_t wpa_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wpa_wait_cond = PTHREAD_COND_INITIALIZER;
int wpa_wait_state = 0;

/* Wait until state machine hits given state, give up at
 * timeout_sec. Return 0 on success, <0 on timeout.*/
static int wpa_wait_for_state(int state, unsigned timeout_sec)
{
	struct timeval now;
	struct timespec timeout;
	int ret = 0;

	pthread_mutex_lock(&wpa_wait_mutex);
	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + timeout_sec;
	timeout.tv_nsec = now.tv_usec * 1000;

	while(state != wpa_wait_state && ret != ETIMEDOUT) {
		ret = pthread_cond_timedwait(&wpa_wait_cond, &wpa_wait_mutex,
					&timeout);
	}

	pthread_mutex_unlock(&wpa_wait_mutex);
	return -ret;
}

/* Signal waiting thread that state changed. Callback from eapol loop thread. */
static int wpa_eapol_state_change_cb(int newstate, void *data)
{
	BLTS_TRACE("WPA: EAPOL state changed -> %s\n",
		eapol_auth_state_as_string(newstate));
	pthread_mutex_lock(&wpa_wait_mutex);

	wpa_wait_state = newstate;
	pthread_cond_broadcast(&wpa_wait_cond);

	pthread_mutex_unlock(&wpa_wait_mutex);

	return 0;
}

int associate_with_wpa2psk_ap(wlan_core_data *data)
{
	int res = 0;
	u8 *ie;
	u8 *ssid = (u8*) data->cmd->ssid;
	u8 *wpaie;

	struct auth_params au_params;
	struct associate_params as_params;
	struct scan_res* bss = NULL;

	memset(&au_params, 0, sizeof(au_params));
	memset(&as_params, 0, sizeof(as_params));
	memset(&data->cmd->bssid, 0, sizeof(data->cmd->bssid));
	memset(&data->cmd->auth_bssid, 0, sizeof(data->cmd->auth_bssid));

	if(!ssid)
		return -1;

	res = wpa_derive_ccmp_pmk(data->cmd->wpa_passphrase, data->cmd->ssid,
				data->cmd->wpa_pmk);

	if(res) {
		BLTS_ERROR("ERROR - PMK derivation failed\n");
		goto done;
	}

	res = nl80211_create_monitor_iface(data);
	if(res < 0) {
		BLTS_ERROR("ERROR - Cannot create monitor interface\n");
		goto done;
	}

	res = eapol_loop_run(data, 0);
	if(res) {
		BLTS_ERROR("EAPOL init failed, can't continue test.\n");
		goto done;
	}
	res = eapol_register_callback(wpa_eapol_state_change_cb, NULL);
	if(res) {
		BLTS_ERROR("Cannot register callback for EAPOL, stopping test.\n");
		goto done;
	}

	res = nl80211_scan_oneshot(data, ssid, strlen((const char*)ssid));
	if(res) {
		BLTS_ERROR("\nERROR - Initial scan failed!\n");
		goto done;
	}

	bss = get_bss_by_ssid(data, ssid, strlen((const char*)ssid));
	if(!bss) {
		BLTS_ERROR("ERROR - No SSID \"%s\" found.\n", ssid);
		res = -1;
		goto done;
	}
	ie = (u8*)scan_get_ie(bss, WLAN_EID_SSID);
	data->cmd->last_ap_rsn_ie = scan_get_ie(bss, WLAN_EID_RSN);
	if(!data->cmd->last_ap_rsn_ie) {
		BLTS_ERROR("ERROR - No RSN IE in AP probe response.\n");
		BLTS_ERROR("NOTE - Check AP settings: WPA 2 with pre-shared key, CCMP(AES) required\n");
		res = -1;
		goto done;
	}

	memcpy(data->their_addr, bss->bssid, ETH_ALEN);

	au_params.bssid = as_params.bssid = bss->bssid;
	au_params.ssid = as_params.ssid = ie ? ie + 2 : (u8 *) "";
	au_params.ssid_len = as_params.ssid_len = ie ? ie[1] : 0;
	au_params.freq = as_params.freq = bss->freq;
	au_params.auth_alg = as_params.auth_alg = WPA_AUTH_ALG_OPEN;

	size_t wpaie_len = sizeof(struct rsn_ie_hdr) + RSN_SELECTOR_LEN +
		2 + RSN_SELECTOR_LEN + 2 + RSN_SELECTOR_LEN + 2 + 0;

	wpaie = malloc(wpaie_len);
	res = wpa_gen_wpa_ie_rsn(wpaie, wpaie_len,
				WPA_CIPHER_CCMP, WPA_CIPHER_CCMP,
				WPA_KEY_MGMT_PSK, 0);

	as_params.wpa_ie = wpaie;
	as_params.wpa_ie_len = wpaie_len;

	data->cmd->rsn_ie = wpaie;
	data->cmd->rsn_ie_len = wpaie_len;

	as_params.pairwise_suite = CIPHER_CCMP;
	as_params.group_suite = CIPHER_CCMP;
	as_params.key_mgmt_suite = KEY_MGMT_PSK;

	res = nl80211_authenticate_oneshot(data, &au_params);
	if(res) {
		BLTS_ERROR("Authentication failed!\n");
		goto done;
	}

	res = nl80211_associate_oneshot(data, &as_params);
	if(res)	{
		BLTS_ERROR("Association failed!\n");
		goto done;
	}

	if(!data->cmd->associated) {
		res = -1;
		BLTS_ERROR("WPA: Not associated, cannot continue.\n");
		goto done;
	}
	res = wpa_wait_for_state(AUTH_STATE_GROUP_HS_READY, 30);
	if(res) {
		BLTS_ERROR("WPA: Timed out waiting for key exchange\n");
		goto done;
	}

	res = send_dhcp_discover(data);
	if(res) {
		BLTS_ERROR("WPA: No reply to DHCP Discover, assuming test failure.\n");
		goto done;
	}

done:
	eapol_unregister_callback(wpa_eapol_state_change_cb);

	eapol_loop_unref(data);
	nl80211_remove_monitor_iface(data);

	if(bss && nl80211_disconnect(data, bss->bssid, WLAN_REASON_DEAUTH_LEAVING))
	{
		BLTS_ERROR("Disconnect from AP failed (or never connected properly)\n");
		res = -1;
	}

	return res;

}

