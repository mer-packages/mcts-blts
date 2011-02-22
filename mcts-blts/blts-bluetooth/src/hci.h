/* hci.h -- HCI specific functionality

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

#ifndef HCI_H
#define HCI_H

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "bt_ctx.h"

/** wait time after reset in seconds */
#define WAIT_TIME_AFTER_RESET 10

/** wait time to connect properly before disconnecting in seconds */
#define WAIT_TIME_CONNECT_DISCONNECT 1

/* -------------------------------------------------------------------------- */
/* Helpers not in libbluetooth */

typedef struct {
	uint16_t handle;
} __attribute__ ((packed)) read_automatic_flush_timeout_cp;
#define READ_AUTOMATIC_FLUSH_TIMEOUT_CP_SIZE 2

typedef struct {
	uint8_t status;
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed)) read_automatic_flush_timeout_rp;
#define READ_AUTOMATIC_FLUSH_TIMEOUT_RP_SIZE 5

int hci_read_automatic_flush_timeout(int dd, uint16_t handle, uint16_t *timeout, int to);


typedef struct {
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed)) write_automatic_flush_timeout_cp;
#define WRITE_AUTOMATIC_FLUSH_TIMEOUT_CP_SIZE 4

typedef struct {
	uint8_t status;
	uint16_t handle;
} __attribute__ ((packed)) write_automatic_flush_timeout_rp;
#define WRITE_AUTOMATIC_FLUSH_TIMEOUT_RP_SIZE 3

int hci_write_automatic_flush_timeout(int dd, uint16_t handle, uint16_t timeout, int to);

#define MAX_EXT_PAGES_TO_VERIFY 5

struct device_info
{
	struct hci_version version;
	uint8_t got_version;
	uint8_t features[8];
	uint8_t got_features;
	uint8_t got_ext_features;
	uint8_t ext_features[MAX_EXT_PAGES_TO_VERIFY][8];
	uint8_t max_page;
};
typedef struct device_info device_info_t;

struct link_info
{
	int8_t rssi;
	uint8_t got_rssi;
	uint8_t lq;
	uint8_t got_lq;
	int8_t level;
	uint8_t got_level;
	uint8_t map[10];
	uint8_t got_map;
	uint32_t clock;
	uint16_t accuracy;
	uint8_t got_clock;
	uint16_t offset;
	uint8_t got_offset;
};
typedef struct link_info link_info_t;

struct frame
{
	void    *data;
    int     data_len;
    void    *ptr;
    int     len;
    int     in;
    int     handle;
    int     cid;
    long    flags;
    struct  timeval ts;
};
typedef int (*read_hci_data_callback)(struct frame *frm);

int read_hci_socket(int sock, int buf_size, int count, read_hci_data_callback call_function);
int open_hci_socket(int dev);
int verify_acl_test_data(struct frame *frm);
int filter_acl_data(int sock);
int filter_events(int sock, int *events);
int hci_init_controller(struct bt_ctx *ctx);
int hci_receive_conn_request(struct bt_ctx *ctx);

device_info_t* hci_get_info_remote (struct bt_ctx *ctx);
device_info_t* hci_get_info_local(struct bt_ctx *ctx);

link_info_t* hci_get_link_info(struct bt_ctx *ctx, int which);

/* -------------------------------------------------------------------------- */
/* Test code */
int do_change_name(struct bt_ctx *ctx);
int do_change_class(struct bt_ctx *ctx);
int do_scan(struct bt_ctx *ctx, int *n_responses, inquiry_info *responses, char **names);
int do_reset(struct bt_ctx *ctx);
int hci_connect_remote(struct bt_ctx *ctx);
int hci_disconnect_remote(struct bt_ctx *ctx);
int hci_verify_class_change(struct bt_ctx *ctx);
int hci_verify_name_change(struct bt_ctx *ctx);
int hci_transfer_acl_data(struct bt_ctx *ctx);
int hci_receive_acl_data(struct bt_ctx *ctx);
int hci_audit_incoming_connect(struct bt_ctx *ctx);

int do_le_scan(struct bt_ctx *ctx);
int le_set_advertise_mode(struct bt_ctx *ctx, int adv_on);
int le_connect_remote(struct bt_ctx *ctx);
int le_disconnect_remote(struct bt_ctx *ctx);

#endif /* HCI_H */
