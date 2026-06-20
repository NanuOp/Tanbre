    /*
 * nyanBOX Compatibility Layer - Implementation
 */
#include "nyanbox_compat.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include <NimBLEDevice.h>

#define NIMBLE_V2_PLUS 1

NyanDisplay u8g2;

bool initBLE() {
    NimBLEDevice::init("");
    return true;
}

void cleanupBLE() {
    NimBLEDevice::deinit(true);
}

bool initWiFi(wifi_mode_t mode) {
    wifi_mode_t currentMode;
    if (esp_wifi_get_mode(&currentMode) != ESP_OK) {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        if (esp_wifi_init(&cfg) != ESP_OK) return false;
    }

    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(mode);
    if (esp_wifi_start() != ESP_OK) return false;

    return true;
}

void cleanupWiFi() {
    wifi_mode_t mode;
    if (esp_wifi_get_mode(&mode) == ESP_OK) {
        esp_wifi_set_promiscuous(false);
        esp_wifi_stop();
        delay(50);
        esp_wifi_deinit();
        delay(100);
    }

    esp_netif_t* sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif != NULL) {
        esp_netif_destroy(sta_netif);
    }

    esp_netif_t* ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (ap_netif != NULL) {
        esp_netif_destroy(ap_netif);
    }

    delay(100);
}

void cleanupRadio() {
    cleanupWiFi();
    cleanupBLE();
}

void runNyanModule(NyanSetupFn setup, NyanLoopFn loop, NyanCleanupFn cleanup) {
    // Run setup
    setup();

    // Run loop until user triggers returnToMenu (EscPress)
    returnToMenu = false;
    while (!returnToMenu) {
        loop();

        // Check for exit
        if (check(EscPress)) {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Run cleanup if provided
    if (cleanup) {
        cleanup();
    } else {
        cleanupRadio();
    }

    // Restore display
    tft.fillScreen(bruceConfig.bgColor);
    returnToMenu = true;
}

// ─── BLE Mock Implementation ─────────────────────────────────────────

static esp_gap_ble_cb_t registered_gap_callback = nullptr;
static uint8_t* raw_adv_data = nullptr;
static size_t raw_adv_data_len = 0;
static uint8_t* raw_scan_rsp_data = nullptr;
static size_t raw_scan_rsp_data_len = 0;
static esp_bd_addr_t local_rand_addr = {0};

extern "C" {

int esp_ble_gap_register_callback(esp_gap_ble_cb_t callback) {
    registered_gap_callback = callback;
    return 0; // ESP_OK
}

int esp_ble_gap_set_scan_params(esp_ble_scan_params_t *scan_params) {
    (void)scan_params;
    if (registered_gap_callback) {
        esp_ble_gap_cb_param_t param;
        memset(&param, 0, sizeof(param));
        param.scan_param_cmpl.status = ESP_BT_STATUS_SUCCESS;
        registered_gap_callback(ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT, &param);
    }
    return 0; // ESP_OK
}

class NyanCompatScanCallbacks : public NimBLEScanCallbacks {
    void onDiscovered(const NimBLEAdvertisedDevice* advertisedDevice) override {
        sendDeviceResult(advertisedDevice);
    }

    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        sendDeviceResult(advertisedDevice);
    }

    void sendDeviceResult(const NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.printf("[BLE Scan] Discovered: MAC=%s, Name=%s, RSSI=%d, AdvType=%d, PayloadLen=%d\n",
                      advertisedDevice->getAddress().toString().c_str(),
                      advertisedDevice->getName().c_str(),
                      advertisedDevice->getRSSI(),
                      advertisedDevice->getAdvType(),
                      (int)advertisedDevice->getPayload().size());

        if (registered_gap_callback) {
            esp_ble_gap_cb_param_t param;
            memset(&param, 0, sizeof(param));
            param.scan_rst.search_evt = ESP_GAP_SEARCH_INQ_RES_EVT;
            memcpy(param.scan_rst.bda, advertisedDevice->getAddress().getVal(), 6);
            param.scan_rst.ble_addr_type = (esp_ble_addr_type_t)advertisedDevice->getAddressType();
            param.scan_rst.ble_evt_type = advertisedDevice->getAdvType();
            param.scan_rst.rssi = advertisedDevice->getRSSI();
            
            size_t len = advertisedDevice->getPayload().size();
            if (len > 62) len = 62;
            memcpy(param.scan_rst.ble_adv, advertisedDevice->getPayload().data(), len);
            param.scan_rst.adv_data_len = len;
            param.scan_rst.scan_rsp_len = 0;
            
            registered_gap_callback(ESP_GAP_BLE_SCAN_RESULT_EVT, &param);
        }
    }
};

static NyanCompatScanCallbacks* scanCallbacks = nullptr;

int esp_ble_gap_start_scanning(uint32_t duration) {
    NimBLEScan* pScan = NimBLEDevice::getScan();
    if (!scanCallbacks) {
        scanCallbacks = new NyanCompatScanCallbacks();
    }
    pScan->setScanCallbacks(scanCallbacks);
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(99);
    
    // Start asynchronous scanning (duration in milliseconds)
    pScan->start(duration * 1000, [](NimBLEScanResults results) {
        (void)results;
        if (registered_gap_callback) {
            esp_ble_gap_cb_param_t param;
            memset(&param, 0, sizeof(param));
            param.scan_rst.search_evt = ESP_GAP_SEARCH_INQ_CMPL_EVT;
            registered_gap_callback(ESP_GAP_BLE_SCAN_RESULT_EVT, &param);
        }
    }, false);
    
    // Call callback to let module know scan started
    if (registered_gap_callback) {
        esp_ble_gap_cb_param_t param;
        memset(&param, 0, sizeof(param));
        param.scan_start_cmpl.status = ESP_BT_STATUS_SUCCESS;
        registered_gap_callback(ESP_GAP_BLE_SCAN_START_COMPLETE_EVT, &param);
    }
    return 0; // ESP_OK
}

int esp_ble_gap_stop_scanning(void) {
    NimBLEDevice::getScan()->stop();
    if (registered_gap_callback) {
        esp_ble_gap_cb_param_t param;
        memset(&param, 0, sizeof(param));
        param.scan_stop_cmpl.status = ESP_BT_STATUS_SUCCESS;
        registered_gap_callback(ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT, &param);
    }
    return 0; // ESP_OK
}

int esp_ble_gap_stop_advertising(void) {
    NimBLEDevice::getAdvertising()->stop();
    return 0; // ESP_OK
}

int esp_ble_gap_set_rand_addr(esp_bd_addr_t rand_addr) {
    memcpy(local_rand_addr, rand_addr, 6);
    return 0; // ESP_OK
}

int esp_ble_gap_set_device_name(const char *name) {
    (void)name;
    return 0; // ESP_OK
}

int esp_ble_gap_config_adv_data_raw(uint8_t *raw_data, uint32_t raw_data_len) {
    if (raw_adv_data) { free(raw_adv_data); raw_adv_data = nullptr; }
    if (raw_data && raw_data_len > 0) {
        raw_adv_data = (uint8_t*)malloc(raw_data_len);
        memcpy(raw_adv_data, raw_data, raw_data_len);
        raw_adv_data_len = raw_data_len;
    } else {
        raw_adv_data_len = 0;
    }
    return 0; // ESP_OK
}

int esp_ble_gap_config_scan_rsp_data_raw(uint8_t *raw_data, uint32_t raw_data_len) {
    if (raw_scan_rsp_data) { free(raw_scan_rsp_data); raw_scan_rsp_data = nullptr; }
    if (raw_data && raw_data_len > 0) {
        raw_scan_rsp_data = (uint8_t*)malloc(raw_data_len);
        memcpy(raw_scan_rsp_data, raw_data, raw_data_len);
        raw_scan_rsp_data_len = raw_data_len;
    } else {
        raw_scan_rsp_data_len = 0;
    }
    return 0; // ESP_OK
}

int esp_ble_gap_start_advertising(esp_ble_adv_params_t *adv_params) {
    (void)adv_params;
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    
    NimBLEAdvertisementData advData;
    if (raw_adv_data && raw_adv_data_len > 0) {
        advData.addData(raw_adv_data, raw_adv_data_len);
    }
    
    NimBLEAdvertisementData scanRspData;
    if (raw_scan_rsp_data && raw_scan_rsp_data_len > 0) {
        scanRspData.addData(raw_scan_rsp_data, raw_scan_rsp_data_len);
    }
    
    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setScanResponseData(scanRspData);
    pAdvertising->setMinInterval(32);
    pAdvertising->setMaxInterval(48);
    
    pAdvertising->start();
    return 0; // ESP_OK
}


uint8_t *esp_ble_resolve_adv_data(uint8_t *adv_data, uint8_t type, uint8_t *length) {
    if (!adv_data) return nullptr;
    uint8_t i = 0;
    while (i < 62) {
        uint8_t len = adv_data[i];
        if (len == 0) break;
        if (i + len + 1 > 62) break;
        uint8_t ad_type = adv_data[i + 1];
        if (ad_type == type) {
            *length = len - 1;
            return &adv_data[i + 2];
        }
        i += len + 1;
    }
    return nullptr;
}

} // extern "C"

#include "airtag_detector.h"
std::vector<AirTagDeviceData> airtagDevices;

