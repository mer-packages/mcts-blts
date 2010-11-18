/* wlan-core-crypto.h -- Crypto stuff, mainly for WPA

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

#ifndef WLAN_CORE_CRYPTO_H
#define WLAN_CORE_CRYPTO_H

#include "wlan-core-definitions.h"

#define SHA1_MAC_LEN 20
#define MD5_MAC_LEN 16
#define WPA_MAX_SSID_LEN 32
#define PMK_LEN 32
#define WPA_NONCE_LEN 32
#define WPA_KEY_RSC_LEN 8
#define WPA_GMK_LEN 32
#define WPA_GTK_MAX_LEN 32

int sha1_prf(const u8 *key, size_t key_len, const char *label,
	const u8 *data, size_t data_len, u8 *buf, size_t buf_len);

int aes_unwrap(const u8 *kek, int n, const u8 *cipher, u8 *plain);

int wpa_eapol_key_mic(const u8 *key, int ver, const u8 *buf, size_t len,
		u8 *mic);

void wpa_pmk_to_ptk(const u8 *pmk, size_t pmk_len, const char *label,
		const u8 *addr1, const u8 *addr2,
		const u8 *nonce1, const u8 *nonce2,
		u8 *ptk, size_t ptk_len);

int wpa_derive_ccmp_pmk(const char *passphrase, const char *ssid, u8 *pmk);
int rsna_nonce_generator(const u8 *our_addr, u8 *nonce);
int wpa_decrypt_eapol_key_data(u8 *ptk, struct eapol_key_frame *frame, u16 version);

#endif /* WLAN_CORE_CRYPTO_H */
