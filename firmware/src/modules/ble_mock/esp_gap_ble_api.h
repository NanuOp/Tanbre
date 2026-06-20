#ifndef ESP_GAP_BLE_API_H_MOCK_
#define ESP_GAP_BLE_API_H_MOCK_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_BT_STATUS_SUCCESS 0

typedef uint8_t esp_bd_addr_t[6];

typedef enum {
    ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT = 0,
    ESP_GAP_BLE_SCAN_RESULT_EVT,
    ESP_GAP_BLE_SCAN_START_COMPLETE_EVT,
    ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT,
    ESP_GAP_BLE_ADV_START_COMPLETE_EVT,
    ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT,
} esp_gap_ble_cb_event_t;

typedef enum {
    ESP_GAP_SEARCH_INQ_RES_EVT = 0,
    ESP_GAP_SEARCH_INQ_CMPL_EVT,
} esp_gap_search_evt_t;

#include "esp_bt.h"

typedef enum {
    BLE_SCAN_TYPE_PASSIVE = 0x00,
    BLE_SCAN_TYPE_ACTIVE = 0x01,
} esp_ble_scan_type_t;

typedef enum {
    BLE_ADDR_TYPE_PUBLIC = 0x00,
    BLE_ADDR_TYPE_RANDOM = 0x01,
    BLE_ADDR_TYPE_RPA_PUBLIC = 0x02,
    BLE_ADDR_TYPE_RPA_RANDOM = 0x03,
} esp_ble_addr_type_t;

typedef enum {
    BLE_SCAN_FILTER_ALLOW_ALL = 0x00,
} esp_ble_scan_filter_policy_t;

typedef enum {
    BLE_SCAN_DUPLICATE_DISABLE = 0x00,
    BLE_SCAN_DUPLICATE_ENABLE = 0x01,
} esp_ble_scan_duplicate_t;

typedef struct {
    esp_ble_scan_type_t scan_type;
    esp_ble_addr_type_t own_addr_type;
    esp_ble_scan_filter_policy_t scan_filter_policy;
    uint16_t scan_interval;
    uint16_t scan_window;
    esp_ble_scan_duplicate_t scan_duplicate;
} esp_ble_scan_params_t;

typedef enum {
    ADV_TYPE_IND = 0x00,
    ADV_TYPE_DIRECT_IND_HIGH = 0x01,
    ADV_TYPE_SCAN_IND = 0x02,
    ADV_TYPE_NONCONN_IND = 0x03,
    ADV_TYPE_DIRECT_IND_LOW = 0x04,
} esp_ble_adv_type_t;

typedef enum {
    ADV_CHNL_37 = 0x01,
    ADV_CHNL_38 = 0x02,
    ADV_CHNL_39 = 0x04,
    ADV_CHNL_ALL = 0x07,
} esp_ble_adv_channel_t;

typedef enum {
    ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY = 0x00,
} esp_ble_adv_filter_t;

typedef struct {
    uint16_t adv_int_min;
    uint16_t adv_int_max;
    esp_ble_adv_type_t adv_type;
    esp_ble_addr_type_t own_addr_type;
    esp_ble_addr_type_t peer_addr_type;
    uint8_t peer_addr[6];
    esp_ble_adv_channel_t channel_map;
    esp_ble_adv_filter_t adv_filter_policy;
} esp_ble_adv_params_t;

typedef uint8_t esp_ble_evt_type_t;

typedef union {
    struct ble_scan_result_evt_param {
        esp_gap_search_evt_t search_evt;
        esp_bd_addr_t bda;
        esp_ble_addr_type_t ble_addr_type;
        esp_ble_evt_type_t ble_evt_type;
        int rssi;
        uint8_t ble_adv[62];
        int flag;
        int num_resps;
        uint8_t adv_data_len;
        uint8_t scan_rsp_len;
    } scan_rst;
    
    struct ble_scan_param_evt_param {
        uint32_t status;
    } scan_param_cmpl;
    
    struct ble_scan_start_evt_param {
        uint32_t status;
    } scan_start_cmpl;

    struct ble_scan_stop_evt_param {
        uint32_t status;
    } scan_stop_cmpl;

    struct ble_adv_start_evt_param {
        uint32_t status;
    } adv_start_cmpl;

    struct ble_adv_stop_evt_param {
        uint32_t status;
    } adv_stop_cmpl;
} esp_ble_gap_cb_param_t;

typedef void (*esp_gap_ble_cb_t)(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

// Functions
int esp_ble_gap_register_callback(esp_gap_ble_cb_t callback);
int esp_ble_gap_set_scan_params(esp_ble_scan_params_t *scan_params);
int esp_ble_gap_start_scanning(uint32_t duration);
int esp_ble_gap_stop_scanning(void);
int esp_ble_gap_start_advertising(esp_ble_adv_params_t *adv_params);
int esp_ble_gap_stop_advertising(void);
int esp_ble_gap_set_rand_addr(esp_bd_addr_t rand_addr);
int esp_ble_gap_set_device_name(const char *name);
int esp_ble_gap_config_adv_data_raw(uint8_t *raw_data, uint32_t raw_data_len);
int esp_ble_gap_config_scan_rsp_data_raw(uint8_t *raw_data, uint32_t raw_data_len);
int esp_ble_tx_power_set(esp_ble_power_type_t power_type, esp_power_level_t power_level);
uint8_t *esp_ble_resolve_adv_data(uint8_t *adv_data, uint8_t type, uint8_t *length);

#define ESP_BLE_AD_TYPE_NAME_CMPL 0x09
#define ESP_BLE_AD_TYPE_NAME_SHORT 0x08
#define ESP_BLE_AD_TYPE_128SRV_CMPL 0x07
#define ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE 0xFF
#define ESP_BLE_AD_TYPE_16SRV_PART 0x02
#define ESP_BLE_AD_TYPE_16SRV_CMPL 0x03
#define ESP_BLE_AD_TYPE_128SRV_PART 0x06

#ifdef __cplusplus
}
#endif

#endif // ESP_GAP_BLE_API_H_MOCK_
