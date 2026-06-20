#include "pwnagotchi_detector.h"
#include "drone_detector.h"
#include "flock_detector.h"
#include "modules/wifi/pineapple_detector.h"

#include <globals.h>
#include "core/display.h"
#include "core/utils.h"
#include "core/mykeyboard.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <ArduinoJson.h>
#include <vector>
#include <algorithm>

#include "modules/ble/BLE_Suite.h"
#include <NimBLEDevice.h>

enum class WifiDetectorType {
    PINEAPPLE,
    PWNAGOTCHI,
    DRONE,
    FLOCK
};

struct WifiDetectorDevice {
    String name;
    String mac;
    int rssi;
    int channel;
    unsigned long lastSeen;
    String extraStr;
    int extraInt1 = 0;
    bool isWifi = true;
};

static WifiDetectorType currentType;
static std::vector<WifiDetectorDevice> devices;
static portMUX_TYPE deviceMux = portMUX_INITIALIZER_UNLOCKED;
static bool scanning = false;
static uint8_t currentChannel = 1;
static unsigned long lastChannelHop = 0;
const unsigned long CHANNEL_HOP_INTERVAL = 400;

static bool locateMode = false;
static bool detailView = false;
static int selectIdx = 0;
static int startListIdx = 0;
static String locateMac = "";
static String locateName = "";

static bool check_pineapple_oui(const String& mac) {
    if (mac.length() < 8) return false;
    if (mac.startsWith("00:13:37")) return true;
    return (mac.substring(3, 5).equalsIgnoreCase("13") && mac.substring(6, 8).equalsIgnoreCase("37"));
}

static bool check_flock_mac(const String& mac) {
    const char* prefixes[] = {
        "58:8e:81", "cc:cc:cc", "ec:1b:bd", "90:35:ea", "04:0d:84",
        "f0:82:c0", "1c:34:f1", "38:5b:44", "94:34:69", "b4:e3:f9",
        "70:c9:4e", "3c:91:80", "d8:f3:bc", "80:30:49", "14:5a:fc",
        "74:4c:a1", "08:3a:88", "9c:2f:9d", "94:08:53", "e4:aa:ea"
    };
    for (const char* p : prefixes) {
        if (mac.startsWith(p)) return true;
    }
    return false;
}

static bool check_flock_ssid(const String& ssid) {
    if (ssid.isEmpty()) return false;
    String s = ssid;
    s.toLowerCase();
    return (s.indexOf("flock") >= 0 || s.indexOf("fs ext battery") >= 0 ||
            s.indexOf("penguin") >= 0 || s.indexOf("pigvision") >= 0);
}

static void parseOpenDroneIDWiFi(const uint8_t* payload, int len, const String& mac, int rssi, int ch) {
    for (int i = 0; i <= len - 25; i++) {
        if (payload[i] == 0x10) {
            int32_t lat_raw = (int32_t)payload[i+2] | ((int32_t)payload[i+3] << 8) | ((int32_t)payload[i+4] << 16) | ((int32_t)payload[i+5] << 24);
            int32_t lon_raw = (int32_t)payload[i+6] | ((int32_t)payload[i+7] << 8) | ((int32_t)payload[i+8] << 16) | ((int32_t)payload[i+9] << 24);
            uint16_t alt_raw = (uint16_t)payload[i+10] | ((uint16_t)payload[i+11] << 8);
            
            double lat = lat_raw / 10000000.0;
            double lon = lon_raw / 10000000.0;
            double alt = alt_raw * 0.5 - 1000.0;
            
            if (lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0 && lat != 0.0 && lon != 0.0) {
                portENTER_CRITICAL(&deviceMux);
                bool found = false;
                for (auto& d : devices) {
                    if (d.mac == mac) {
                        d.rssi = rssi;
                        d.lastSeen = millis();
                        d.extraStr = String(lat, 6) + ", " + String(lon, 6);
                        d.extraInt1 = (int)alt;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    WifiDetectorDevice dev;
                    dev.name = "Drone";
                    dev.mac = mac;
                    dev.rssi = rssi;
                    dev.channel = ch;
                    dev.lastSeen = millis();
                    dev.extraStr = String(lat, 6) + ", " + String(lon, 6);
                    dev.extraInt1 = (int)alt;
                    dev.isWifi = true;
                    devices.push_back(dev);
                }
                portEXIT_CRITICAL(&deviceMux);
            }
        }
    }
}

static void wifi_sniffer_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!scanning) return;
    if (type != WIFI_PKT_MGMT) return;

    wifi_promiscuous_pkt_t* packet = (wifi_promiscuous_pkt_t*)buf;
    uint8_t* payload = packet->payload;
    int len = packet->rx_ctrl.sig_len;

    if (len <= 4) return;
    len -= 4;

    uint8_t frameType = payload[0];
    if (frameType != 0x80 && frameType != 0x50) return;

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             payload[10], payload[11], payload[12], payload[13], payload[14], payload[15]);
    String mac(macStr);

    int offset = 36;
    String ssid = "";
    int ssidLen = 0;
    while (offset + 2 <= len) {
        uint8_t tag = payload[offset];
        uint8_t tag_len = payload[offset + 1];
        if (offset + 2 + tag_len > len) break;

        if (tag == 0) {
            ssidLen = tag_len;
            char ssidBuf[33];
            int copyLen = tag_len > 32 ? 32 : tag_len;
            memcpy(ssidBuf, &payload[offset + 2], copyLen);
            ssidBuf[copyLen] = '\0';
            ssid = String(ssidBuf);
            break;
        }
        offset += 2 + tag_len;
    }

    int rssi = packet->rx_ctrl.rssi;
    int ch = packet->rx_ctrl.channel;

    if (currentType == WifiDetectorType::PWNAGOTCHI) {
        if (mac.equalsIgnoreCase("de:ad:be:ef:de:ad")) {
            JsonDocument doc;
            if (deserializeJson(doc, ssid) == DeserializationError::Ok) {
                const char* jsName = doc["name"];
                const char* jsVer = doc["version"];
                int jsPwnd = doc["pwnd_tot"];

                if (jsName && jsVer) {
                    portENTER_CRITICAL(&deviceMux);
                    bool found = false;
                    for (auto& d : devices) {
                        if (d.name == String(jsName)) {
                            d.rssi = rssi;
                            d.channel = ch;
                            d.lastSeen = millis();
                            d.extraStr = String(jsVer);
                            d.extraInt1 = jsPwnd;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        WifiDetectorDevice dev;
                        dev.name = String(jsName);
                        dev.mac = mac;
                        dev.rssi = rssi;
                        dev.channel = ch;
                        dev.lastSeen = millis();
                        dev.extraStr = String(jsVer);
                        dev.extraInt1 = jsPwnd;
                        dev.isWifi = true;
                        devices.push_back(dev);
                    }
                    portEXIT_CRITICAL(&deviceMux);
                }
            }
        }
    }
    else if (currentType == WifiDetectorType::PINEAPPLE) {
        if (check_pineapple_oui(mac)) {
            portENTER_CRITICAL(&deviceMux);
            bool found = false;
            for (auto& d : devices) {
                if (d.mac == mac) {
                    d.rssi = rssi;
                    d.channel = ch;
                    d.lastSeen = millis();
                    found = true;
                    break;
                }
            }
            if (!found) {
                WifiDetectorDevice dev;
                dev.name = ssid.isEmpty() ? "Pineapple" : ssid;
                dev.mac = mac;
                dev.rssi = rssi;
                dev.channel = ch;
                dev.lastSeen = millis();
                dev.extraStr = "OUI Match";
                dev.isWifi = true;
                devices.push_back(dev);
            }
            portEXIT_CRITICAL(&deviceMux);
        }
    }
    else if (currentType == WifiDetectorType::DRONE) {
        bool isDrone = false;
        uint8_t nan_dest[6] = {0x51, 0x6f, 0x9a, 0x01, 0x00, 0x00};
        if (len > 30 && memcmp(nan_dest, &payload[4], 6) == 0) {
            isDrone = true;
        }

        int ieOffset = 36;
        while (ieOffset + 2 <= len) {
            uint8_t typ = payload[ieOffset];
            uint8_t ieLen = payload[ieOffset + 1];
            if (ieOffset + 2 + ieLen > len) break;

            if (typ == 0xdd && ieLen >= 4) {
                if ((payload[ieOffset + 2] == 0x90 && payload[ieOffset + 3] == 0x3a && payload[ieOffset + 4] == 0xe6) ||
                    (payload[ieOffset + 2] == 0xfa && payload[ieOffset + 3] == 0x0b && payload[ieOffset + 4] == 0xbc)) {
                    isDrone = true;
                    break;
                }
            }
            ieOffset += 2 + ieLen;
        }

        if (isDrone) {
            String droneName = "Drone (OpenID)";
            if (mac.startsWith("60:60:1f") || mac.startsWith("00:e0:4c") || mac.startsWith("04:e2:f8")) {
                droneName = "DJI Drone";
            }
            
            portENTER_CRITICAL(&deviceMux);
            bool found = false;
            for (auto& d : devices) {
                if (d.mac == mac) {
                    d.rssi = rssi;
                    d.channel = ch;
                    d.lastSeen = millis();
                    found = true;
                    break;
                }
            }
            if (!found) {
                WifiDetectorDevice dev;
                dev.name = droneName;
                dev.mac = mac;
                dev.rssi = rssi;
                dev.channel = ch;
                dev.lastSeen = millis();
                dev.extraStr = "WiFi Beacon";
                dev.isWifi = true;
                devices.push_back(dev);
            }
            portEXIT_CRITICAL(&deviceMux);
            
            parseOpenDroneIDWiFi(payload, len, mac, rssi, ch);
        }
    }
    else if (currentType == WifiDetectorType::FLOCK) {
        if (check_flock_ssid(ssid) || check_flock_mac(mac)) {
            String detectionMethod = check_flock_ssid(ssid) ? "WiFi SSID" : "WiFi MAC";
            portENTER_CRITICAL(&deviceMux);
            bool found = false;
            for (auto& d : devices) {
                if (d.mac == mac) {
                    d.rssi = rssi;
                    d.channel = ch;
                    d.lastSeen = millis();
                    d.name = ssid.isEmpty() ? "Flock Device" : ssid;
                    d.extraStr = detectionMethod;
                    found = true;
                    break;
                }
            }
            if (!found) {
                WifiDetectorDevice dev;
                dev.name = ssid.isEmpty() ? "Flock Device" : ssid;
                dev.mac = mac;
                dev.rssi = rssi;
                dev.channel = ch;
                dev.lastSeen = millis();
                dev.extraStr = detectionMethod;
                dev.isWifi = true;
                devices.push_back(dev);
            }
            portEXIT_CRITICAL(&deviceMux);
        }
    }
}

class DetectorBLECallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        if (!scanning) return;
        
        String mac = advertisedDevice->getAddress().toString().c_str();
        String name = advertisedDevice->getName().c_str();
        int rssi = advertisedDevice->getRSSI();
        std::vector<uint8_t> payload = advertisedDevice->getPayload();
        
        if (currentType == WifiDetectorType::DRONE) {
            bool isDrone = advertisedDevice->haveServiceData() && advertisedDevice->getServiceDataUUID() == NimBLEUUID((uint16_t)0xFFFD);
            if (!isDrone && payload.size() >= 5) {
                for (size_t i = 0; i <= payload.size() - 5; i++) {
                    if (payload[i] == 0x16 && payload[i+1] == 0xFA && payload[i+2] == 0xFF && payload[i+3] == 0x0D) {
                        isDrone = true;
                        break;
                    }
                }
            }

            if (isDrone) {
                portENTER_CRITICAL(&deviceMux);
                bool found = false;
                for (auto& d : devices) {
                    if (d.mac == mac) {
                        d.rssi = rssi;
                        d.lastSeen = millis();
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    WifiDetectorDevice dev;
                    dev.name = name.isEmpty() ? "Drone (BLE)" : name;
                    dev.mac = mac;
                    dev.rssi = rssi;
                    dev.channel = 0;
                    dev.lastSeen = millis();
                    dev.extraStr = "BLE RemoteID";
                    dev.isWifi = false;
                    devices.push_back(dev);
                }
                portEXIT_CRITICAL(&deviceMux);
            }
        }
        else if (currentType == WifiDetectorType::FLOCK) {
            bool isFlock = false;
            String lowerName = name;
            lowerName.toLowerCase();
            if (lowerName.indexOf("flock") >= 0 || lowerName.indexOf("fs ext battery") >= 0 || 
                lowerName.indexOf("penguin") >= 0 || lowerName.indexOf("pigvision") >= 0) {
                isFlock = true;
            }
            if (!isFlock && check_flock_mac(mac)) {
                isFlock = true;
            }

            if (isFlock) {
                String method = !name.isEmpty() ? "BLE Name" : "BLE MAC";
                portENTER_CRITICAL(&deviceMux);
                bool found = false;
                for (auto& d : devices) {
                    if (d.mac == mac) {
                        d.rssi = rssi;
                        d.lastSeen = millis();
                        d.name = name.isEmpty() ? "Flock Device" : name;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    WifiDetectorDevice dev;
                    dev.name = name.isEmpty() ? "Flock Device" : name;
                    dev.mac = mac;
                    dev.rssi = rssi;
                    dev.channel = 0;
                    dev.lastSeen = millis();
                    dev.extraStr = method;
                    dev.isWifi = false;
                    devices.push_back(dev);
                }
                portEXIT_CRITICAL(&deviceMux);
            }
        }
    }
};

static DetectorBLECallbacks bleCallbacksInstance;

static void hopChannel() {
    currentChannel++;
    if (currentChannel > 13) currentChannel = 1;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
}

static void drawWifiDetectorUI() {
    tft.fillScreen(bruceConfig.bgColor);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    String title = "Detector";
    if (currentType == WifiDetectorType::PINEAPPLE) title = "Pineapple Det";
    else if (currentType == WifiDetectorType::PWNAGOTCHI) title = "Pwnagotchi Det";
    else if (currentType == WifiDetectorType::DRONE) title = "Drone Radar";
    else if (currentType == WifiDetectorType::FLOCK) title = "Flock Safety";

    bool isOLED = (tftHeight <= 80);

    if (locateMode) {
        int rssi = -100;
        portENTER_CRITICAL(&deviceMux);
        for (const auto& d : devices) {
            if (d.mac == locateMac) {
                rssi = d.rssi;
                break;
            }
        }
        portEXIT_CRITICAL(&deviceMux);

        int rssiClamped = constrain(rssi, -100, -40);
        int signalLevel = map(rssiClamped, -100, -40, 0, 5);
        const char* quality = "VERY WEAK";
        if (signalLevel >= 5) quality = "EXCELLENT";
        else if (signalLevel >= 4) quality = "VERY GOOD";
        else if (signalLevel >= 3) quality = "GOOD";
        else if (signalLevel >= 2) quality = "FAIR";
        else if (signalLevel >= 1) quality = "WEAK";

        if (isOLED) {
            tft.setTextSize(FP);
            tft.drawString("Locating...", 8, 4);
            tft.drawString(locateName.substring(0, 18), 8, 14);
            tft.drawString(locateMac + " " + String(rssi), 8, 24);
            tft.drawString(String("Sig: ") + quality, 8, 34);

            int barWidth = 10;
            int barSpacing = 4;
            int startX = 8;
            int baseY = 52;
            for (int i = 0; i < 5; i++) {
                int barHeight = 4 + (i * 2);
                int x = startX + (i * (barWidth + barSpacing));
                int y = baseY - barHeight;
                if (i < signalLevel) tft.fillRect(x, y, barWidth, barHeight, bruceConfig.priColor);
                else tft.drawRect(x, y, barWidth, barHeight, bruceConfig.priColor);
            }
            tft.drawString("ESC=Back  SEL=Exit", 8, 54);
        } else {
            tft.setTextSize(FP);
            tft.drawString("Locating target...", 8, 8);
            tft.drawString(locateName, 8, 20);
            tft.drawString(locateMac, 8, 32);

            tft.setTextSize(FM);
            tft.drawString("RSSI: " + String(rssi) + " dBm", 8, 48);

            tft.setTextSize(FP);
            tft.drawString(String("Signal: ") + quality, 8, 68);

            int barWidth = 14;
            int barSpacing = 6;
            int startX = 8;
            int baseY = 110;
            for (int i = 0; i < 5; i++) {
                int barHeight = 8 + (i * 5);
                int x = startX + (i * (barWidth + barSpacing));
                int y = baseY - barHeight;
                if (i < signalLevel) tft.fillRect(x, y, barWidth, barHeight, bruceConfig.priColor);
                else tft.drawRect(x, y, barWidth, barHeight, bruceConfig.priColor);
            }
            tft.drawString("ESC=Back  SEL=Exit", 8, 120);
        }
    }
    else if (detailView) {
        WifiDetectorDevice dev;
        bool found = false;
        portENTER_CRITICAL(&deviceMux);
        if (selectIdx >= 0 && selectIdx < (int)devices.size()) {
            dev = devices[selectIdx];
            found = true;
        }
        portEXIT_CRITICAL(&deviceMux);

        if (found) {
            unsigned long age = (millis() - dev.lastSeen) / 1000;
            if (isOLED) {
                tft.setTextSize(FP);
                tft.drawString("Device Detail", 8, 2);
                tft.drawLine(8, 11, tftWidth - 8, 11, bruceConfig.priColor);
                tft.drawString("Name: " + dev.name.substring(0, 14), 8, 13);
                tft.drawString("MAC:  " + dev.mac, 8, 23);
                tft.drawString("RSSI: " + String(dev.rssi) + "dBm  " + String(age) + "s", 8, 33);
                tft.drawString("Info: " + dev.extraStr.substring(0, 14), 8, 43);
                tft.drawString("ESC:Back SEL:Exit UP:Loc", 4, 54);
            } else {
                tft.setTextSize(FM);
                tft.drawString("Device Detail", 8, 8);
                tft.drawLine(8, 22, tftWidth - 8, 22, bruceConfig.priColor);

                tft.setTextSize(FP);
                tft.drawString("Name: " + dev.name, 8, 28);
                tft.drawString("MAC:  " + dev.mac, 8, 40);
                tft.drawString("RSSI: " + String(dev.rssi) + " dBm", 8, 52);
                tft.drawString("Channel: " + String(dev.channel), 8, 64);
                tft.drawString("Last Seen: " + String(age) + "s ago", 8, 76);
                
                if (currentType == WifiDetectorType::PWNAGOTCHI) {
                    tft.drawString("Version: " + dev.extraStr, 8, 88);
                    tft.drawString("Handshakes: " + String(dev.extraInt1), 8, 100);
                } else {
                    tft.drawString("Info: " + dev.extraStr, 8, 88);
                }
                
                tft.drawString("ESC=Back  SEL=Exit  UP=Locate", 8, 120);
            }
        }
    }
    else {
        int devSize = 0;
        std::vector<WifiDetectorDevice> tempDevs;
        portENTER_CRITICAL(&deviceMux);
        devSize = devices.size();
        tempDevs = devices;
        portEXIT_CRITICAL(&deviceMux);

        if (isOLED) {
            tft.setTextSize(FP);
            tft.drawString(title + " (" + String(devSize) + ")", 8, 2);
            tft.drawLine(8, 11, tftWidth - 8, 11, bruceConfig.priColor);

            if (devSize == 0) {
                tft.drawString("Scanning CH " + String(currentChannel) + "...", 8, 20);
                tft.drawString("Press ESC to exit", 8, 40);
            } else {
                int start = startListIdx;
                int end = min(start + 4, devSize);
                for (int i = start; i < end; i++) {
                    int y = 14 + (i - start) * 10;
                    String prefix = (i == selectIdx) ? ">" : " ";
                    String line = prefix + tempDevs[i].name.substring(0, 8) + " [" + String(tempDevs[i].rssi) + "]";
                    tft.drawString(line, 8, y);
                }
            }
        } else {
            tft.setTextSize(FM);
            tft.drawString(title + " (" + String(devSize) + ")", 8, 8);
            tft.drawLine(8, 22, tftWidth - 8, 22, bruceConfig.priColor);

            if (devSize == 0) {
                tft.setTextSize(FP);
                tft.drawString("Scanning channel " + String(currentChannel) + "...", 8, 30);
                tft.drawString("No devices found yet.", 8, 45);
                tft.drawString("Press ESC to exit.", 8, 70);
            } else {
                int start = startListIdx;
                int end = min(start + 5, devSize);
                tft.setTextSize(FP);
                for (int i = start; i < end; i++) {
                    int y = 28 + (i - start) * 16;
                    
                    if (i == selectIdx) {
                        tft.fillRect(4, y - 2, tftWidth - 8, 15, bruceConfig.priColor);
                        tft.setTextColor(bruceConfig.bgColor, bruceConfig.priColor);
                    } else {
                        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                    }

                    String namePart = tempDevs[i].name;
                    if (namePart.length() > 14) namePart = namePart.substring(0, 12) + "..";
                    String line = namePart + " | RSSI: " + String(tempDevs[i].rssi) + " | CH:" + String(tempDevs[i].channel);
                    tft.drawString(line, 8, y);
                }
                
                tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                tft.drawString("SEL=Detail  ESC=Exit", 8, 120);
            }
        }
    }
}

void runWifiDetector(WifiDetectorType type) {
    currentType = type;
    
    portENTER_CRITICAL(&deviceMux);
    devices.clear();
    portEXIT_CRITICAL(&deviceMux);

    selectIdx = 0;
    startListIdx = 0;
    detailView = false;
    locateMode = false;
    locateMac = "";
    locateName = "";
    currentChannel = 1;
    lastChannelHop = millis();

    esp_wifi_set_promiscuous(false);
    esp_wifi_stop();
    delay(50);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_set_ps(WIFI_PS_NONE);

    wifi_promiscuous_filter_t flt = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT};
    esp_wifi_set_promiscuous_filter(&flt);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_cb);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);

    NimBLEScan* pScan = nullptr;
    if (type == WifiDetectorType::DRONE || type == WifiDetectorType::FLOCK) {
        if (BLEStateManager::isBLEActive()) {
            BLEStateManager::deinitBLE(true);
        }
        BLEStateManager::initBLE("TanbreWatch-Scout", ESP_PWR_LVL_P9);
        pScan = NimBLEDevice::getScan();
        if (pScan) {
            pScan->setScanCallbacks(&bleCallbacksInstance);
            pScan->setActiveScan(true);
            pScan->setInterval(100);
            pScan->setWindow(99);
            pScan->start(0, false, true);
        }
    }

    scanning = true;
    bool needsDraw = true;
    unsigned long lastDrawTime = 0;

    check(UpPress);
    check(DownPress);
    check(SelPress);
    check(EscPress);

    while (true) {
        unsigned long now = millis();
        yield();

        if (!detailView && !locateMode) {
            if (now - lastChannelHop > CHANNEL_HOP_INTERVAL) {
                hopChannel();
                lastChannelHop = now;
                needsDraw = true;
            }
        }

        if (check(UpPress)) {
            if (locateMode) {
                // Do nothing
            } else if (detailView) {
                locateMode = true;
                portENTER_CRITICAL(&deviceMux);
                if (selectIdx >= 0 && selectIdx < (int)devices.size()) {
                    locateMac = devices[selectIdx].mac;
                    locateName = devices[selectIdx].name;
                }
                portEXIT_CRITICAL(&deviceMux);
                needsDraw = true;
            } else {
                if (selectIdx > 0) {
                    selectIdx--;
                    if (selectIdx < startListIdx) {
                        startListIdx = selectIdx;
                    }
                    needsDraw = true;
                }
            }
        }

        if (check(DownPress)) {
            if (!detailView && !locateMode) {
                int devSize = 0;
                portENTER_CRITICAL(&deviceMux);
                devSize = devices.size();
                portEXIT_CRITICAL(&deviceMux);

                if (selectIdx < devSize - 1) {
                    selectIdx++;
                    if (selectIdx >= startListIdx + 5) {
                        startListIdx = selectIdx - 4;
                    }
                    needsDraw = true;
                }
            }
        }

        if (check(SelPress)) {
            if (locateMode) {
                break;
            } else if (detailView) {
                break;
            } else {
                int devSize = 0;
                portENTER_CRITICAL(&deviceMux);
                devSize = devices.size();
                portEXIT_CRITICAL(&deviceMux);

                if (devSize > 0) {
                    detailView = true;
                    needsDraw = true;
                } else {
                    break;
                }
            }
        }

        if (check(EscPress)) {
            if (locateMode) {
                locateMode = false;
                needsDraw = true;
            } else if (detailView) {
                detailView = false;
                needsDraw = true;
            } else {
                break;
            }
        }

        if (needsDraw || (now - lastDrawTime > 800)) {
            drawWifiDetectorUI();
            needsDraw = false;
            lastDrawTime = now;
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    scanning = false;
    esp_wifi_set_promiscuous(false);
    esp_wifi_stop();
    delay(50);

    if (type == WifiDetectorType::DRONE || type == WifiDetectorType::FLOCK) {
        if (pScan) {
            pScan->stop();
            pScan->setScanCallbacks(nullptr);
        }
        BLEStateManager::deinitBLE(true);
    }

    tft.fillScreen(bruceConfig.bgColor);
}

void runPineappleDetector() { runWifiDetector(WifiDetectorType::PINEAPPLE); }
void runPwnagotchiDetector() { runWifiDetector(WifiDetectorType::PWNAGOTCHI); }
void runDroneDetector() { runWifiDetector(WifiDetectorType::DRONE); }
void runFlockDetector() { runWifiDetector(WifiDetectorType::FLOCK); }
