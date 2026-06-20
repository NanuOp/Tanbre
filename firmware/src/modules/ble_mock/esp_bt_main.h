#ifndef ESP_BT_MAIN_H_MOCK_
#define ESP_BT_MAIN_H_MOCK_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ESP_BLUEDROID_STATUS_UNINITIALIZED = 0,
    ESP_BLUEDROID_STATUS_INITIALIZED,
    ESP_BLUEDROID_STATUS_ENABLED
} esp_bluedroid_status_t;

inline esp_bluedroid_status_t esp_bluedroid_get_status(void) {
    return ESP_BLUEDROID_STATUS_UNINITIALIZED;
}

inline int esp_bluedroid_init(void) {
    return 0; // ESP_OK
}

inline int esp_bluedroid_enable(void) {
    return 0; // ESP_OK
}

inline int esp_bluedroid_disable(void) {
    return 0; // ESP_OK
}

inline int esp_bluedroid_deinit(void) {
    return 0; // ESP_OK
}

#ifdef __cplusplus
}
#endif

#endif // ESP_BT_MAIN_H_MOCK_
