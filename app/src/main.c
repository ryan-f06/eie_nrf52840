/*
 * main.c
 */

// #include <inttypes.h>

// #include "BTN.h"
// #include "LED.h"

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>

#include <string.h>
// #include <inttypes.h>
// #include <stdlib.h>



#define SLEEP_MS 1

// static struct bt_uuid_128 BLE_CUSTOM_SERVICE_UUID =
//     BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111, 0x2222, 0x3333, 0x4444, 0x000000000001));
// static struct bt_uuid_128 BLE_CUSTOM_CHARACTERISTIC_UUID =
//     BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111, 0x2222, 0x3333, 0x4444, 0x000000000002));

static void start_scan(void);

static struct bt_conn* my_connection;



static bool ble_get_adv_device_name_cb(struct bt_data* data, void* user_data) {
  char* name = user_data;

  if (data->type == BT_DATA_NAME_COMPLETE || data->type == BT_DATA_NAME_SHORTENED) {
    memcpy(name, data->data, data->data_len);
    name[data->data_len] = 0; // Null-terminator
    return false; // Stop parsing
  }

  return true; // Continue looking through this advertising packet
}

static void ble_on_advertisement_received(const bt_addr_le_t* addr, int8_t rssi, uint8_t type,
                                          struct net_buf_simple* ad) {

  char addr_str[BT_ADDR_LE_STR_LEN];
  int err;                                       

  if (my_connection) {
    return;
  }
  if (type != BT_GAP_ADV_TYPE_ADV_IND && type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
    return;
  }

  bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

  char name[32] = {0};
  bt_data_parse(ad, ble_get_adv_device_name_cb, name);
  

  printk("Device found: %s (RSSI %d)\n", addr_str, rssi);
  printk("Name: %s\n", name);

  if (rssi < -50) {
    return;
  }

  if (strcmp(name, "Ryan eie") == 0) {

    if (bt_le_scan_stop()) {
      return;
    }

    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT, &my_connection);
    if (err) {
      printk("Create conn to %s failed (%d)\n", addr_str, err);
      start_scan();
    }
  }
  
}


static void start_scan(void) {

  int err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, ble_on_advertisement_received);
  if (err) {
    printk("Scanning failed to start (err %d)\n", err);
    return;
  }

  printk("Scanning successfully started\n");
}


static void ble_on_device_connected(struct bt_conn* conn, uint8_t err) {
  
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  if (err) {
    printk("Failed to connect to %s %u %s\n", addr, err, bt_hci_err_to_str(err));

    bt_conn_unref(my_connection);
    my_connection = NULL;

    start_scan();
    return;
  }

  if (conn != my_connection) {
    return;
  }

  printk("Connected: %s\n", addr);

  int counter = 0;
  while (counter < 30000){
    counter++;
    k_msleep(SLEEP_MS);
  }
  
  bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);

}


static void ble_on_device_disconnected(struct bt_conn* conn, uint8_t reason) {
  
  char addr[BT_ADDR_LE_STR_LEN];

  if (conn != my_connection) {
    return;
  }

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  printk("Disconnected: %s, reason 0x%02x %s\n", addr, reason, bt_hci_err_to_str(reason));

  bt_conn_unref(my_connection);
  my_connection = NULL;

  start_scan();

}


BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = ble_on_device_connected,
    .disconnected = ble_on_device_disconnected,
};


int main(void) {

  // if (0 > BTN_init()) {
  //   return 0;
  // }
  // if (0 > LED_init()) {
  //   return 0;
  // }

  int err = bt_enable(NULL);

  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return 0;
  } 
  
  printk("Bluetooth initialized\n");

  start_scan();
  
  // while(1) {
  //   k_msleep(SLEEP_MS);
  // }
	return 0;
}
