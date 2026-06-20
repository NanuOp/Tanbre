#include "native_detector.h"
#include <globals.h>
#include "core/display.h"
#include "core/utils.h"
#include "modules/ble/BLE_Suite.h"
#include <NimBLEDevice.h>
#include <vector>
#include <algorithm>

// Define target buttons based on globals.h mappings
// globals.h defines: NextPress, PrevPress, UpPress, DownPress, SelPress, EscPress, AnyKeyPress
// We can use check(SelPress), check(EscPress), check(UpPress), check(DownPress)

// Struct to store any scanned device info uniformly
struct ScannedDevice {
    String address;
    String name;
    int rssi;
    unsigned long lastSeen;
    
    // Type-specific extra metadata
    int extraInt1 = 0; // e.g. Samsung completed/part completed, Flipper color, or AirTag manufacturer specific len/pattern
    String extraStr1 = ""; // e.g. Version, method
    std::vector<uint8_t> payload;
};

// Global variables for the native detector
#include <freertos/semphr.h>
static SemaphoreHandle_t devicesMutex = nullptr;

static std::vector<ScannedDevice> detectedDevices;
static BLEDetectorType currentDetectorType;
static int selectIdx = 0;
static int startListIdx = 0;
static bool detailView = false;
static bool locateView = false;
static String targetMacAddress = "";
static unsigned long locateLastUpdate = 0;
static unsigned long listLastUpdate = 0;

// NimBLE Scan instances
static NimBLEScan* pNativeScan = nullptr;

// Helper: mask MAC Address for security/theme
static void getMaskedMAC(const String& rawMac, char* outMasked) {
    if (rawMac.length() < 17) {
        strncpy(outMasked, rawMac.c_str(), 17);
        outMasked[17] = '\0';
        return;
    }
    // Mask middle bytes
    snprintf(outMasked, 18, "%c%c:%c%c:XX:XX:XX:%c%c", 
             rawMac[0], rawMac[1], rawMac[3], rawMac[4], rawMac[15], rawMac[16]);
}

// Helper: mask device name
static void getMaskedName(const String& name, char* outMasked, size_t maxLen) {
    if (name.isEmpty() || name == "Unknown") {
        strncpy(outMasked, "Unknown", maxLen);
        outMasked[maxLen] = '\0';
        return;
    }
    // We only show first few chars or just return it since names aren't strictly confidential
    strncpy(outMasked, name.c_str(), maxLen);
    outMasked[maxLen] = '\0';
}


static bool hasIBeaconPayload(const std::vector<uint8_t>& payload) {
    size_t i = 0;
    while (i + 1 < payload.size()) {
        uint8_t len = payload[i];
        if (len == 0) break;
        if (i + len >= payload.size()) break;

        uint8_t type = payload[i + 1];
        if (type == 0xFF && len >= 26) {
            const uint8_t* data = payload.data() + i + 2;
            size_t dataLen = len - 1;
            if (dataLen >= 25 && data[0] == 0x4C && data[1] == 0x00 && data[2] == 0x02 && data[3] == 0x15) {
                return true;
            }
        }
        i += len + 1;
    }
    return false;
}

static bool looksLikeCameraDevice(const String& name) {
    if (name.isEmpty()) return false;
    String lower = name;
    lower.toLowerCase();
    return lower.indexOf("camera") >= 0 || lower.indexOf("bodycam") >= 0 ||
           lower.indexOf("body cam") >= 0 || lower.indexOf("dashcam") >= 0 ||
           lower.indexOf("dash cam") >= 0 || lower.indexOf("actioncam") >= 0 ||
           lower.indexOf("action cam") >= 0 || lower.indexOf("webcam") >= 0 ||
           lower.indexOf("ipcam") >= 0 || lower.indexOf("cam-") >= 0;
}

// Callback for NimBLE scan
class NativeScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        String address = advertisedDevice->getAddress().toString().c_str();
        String name = advertisedDevice->getName().c_str();
        int rssi = advertisedDevice->getRSSI();
        std::vector<uint8_t> payload = advertisedDevice->getPayload();

        bool match = false;
        int extraInt = 0;
        String extraStr = "";

        switch (currentDetectorType) {
            case BLEDetectorType::DEVICE_SCOUT:
                // Device Scout detects everything
                match = true;
                break;

            case BLEDetectorType::CAMERA: {
                if (looksLikeCameraDevice(name)) {
                    match = true;
                    extraStr = "Camera Device";
                }
                break;
            }

            case BLEDetectorType::IBEACON: {
                if (hasIBeaconPayload(payload)) {
                    match = true;
                    extraStr = "iBeacon";
                }
                break;
            }

            case BLEDetectorType::AIRTAG: {
                // Apple Manufacturer Specific Data (Company ID 0x004C)
                // AirTag uses length 29 or pattern 12 19 (length 25, type 0x12, sub-type 0x19)
                if (advertisedDevice->haveManufacturerData()) {
                    std::string mfg = advertisedDevice->getManufacturerData();
                    if (mfg.length() >= 2) {
                        const uint8_t* mfgBytes = (const uint8_t*)mfg.data();
                        // Apple company ID is 0x4C00 (little endian: 0x4C, 0x00)
                        // But NimBLE getManufacturerData() usually strips the Company ID header or returns it?
                        // Let's match Apple devices generally or checks lengths
                        if (mfg.length() == 29 || (mfg.length() >= 4 && mfgBytes[0] == 0x12 && mfgBytes[1] == 0x19)) {
                            match = true;
                            extraStr = "FindMy Payload";
                        }
                    }
                }
                break;
            }

            case BLEDetectorType::SMARTTAG: {
                // Samsung completed/partially completed 16-bit Service UUID 0xFD5A
                if (advertisedDevice->haveServiceUUID()) {
                    if (advertisedDevice->getServiceUUID() == NimBLEUUID((uint16_t)0xFD5A)) {
                        match = true;
                        extraStr = "Samsung SmartTag";
                    }
                }
                break;
            }

            case BLEDetectorType::TILE: {
                // Service UUIDs 0xFEED or 0xFEEC
                if (advertisedDevice->haveServiceUUID()) {
                    NimBLEUUID serviceUUID = advertisedDevice->getServiceUUID();
                    if (serviceUUID == NimBLEUUID((uint16_t)0xFEED) || serviceUUID == NimBLEUUID((uint16_t)0xFEEC)) {
                        match = true;
                        extraStr = (serviceUUID == NimBLEUUID((uint16_t)0xFEED)) ? "Tile FEED" : "Tile FEEC";
                    }
                }
                break;
            }

            case BLEDetectorType::RAYBAN_META: {
                // Ray-Ban specific UUID checks (0xFD5F)
                if (advertisedDevice->haveServiceUUID()) {
                    if (advertisedDevice->getServiceUUID() == NimBLEUUID((uint16_t)0xFD5F)) {
                        match = true;
                        extraStr = "Ray-Ban Meta";
                    }
                }
                break;
            }

            case BLEDetectorType::AXON_BODYCAM: {
                // Axon MAC Prefix 00:25:df
                if (address.startsWith("00:25:df") || address.startsWith("00:25:DF")) {
                    match = true;
                    extraStr = "Axon Bodycam";
                }
                break;
            }

            case BLEDetectorType::FLIPPER_ZERO: {
                // Flipper MAC prefixes: 80:e1:26, 80:e1:27, 0c:fa:22
                // Service UUIDs: 0x3081 (Black), 0x3082 (White), 0x3083 (Transparent), or mask 0xFFF0 == 0x3080
                bool macMatch = address.startsWith("80:e1:26") || address.startsWith("80:E1:26") ||
                                address.startsWith("80:e1:27") || address.startsWith("80:E1:27") ||
                                address.startsWith("0c:fa:22") || address.startsWith("0C:FA:22");
                bool uuidMatch = false;
                if (advertisedDevice->haveServiceUUID()) {
                    NimBLEUUID serviceUUID = advertisedDevice->getServiceUUID();
                    if (serviceUUID == NimBLEUUID((uint16_t)0x3081)) {
                        uuidMatch = true;
                        extraStr = "Black Flipper";
                    } else if (serviceUUID == NimBLEUUID((uint16_t)0x3082)) {
                        uuidMatch = true;
                        extraStr = "White Flipper";
                    } else if (serviceUUID == NimBLEUUID((uint16_t)0x3083)) {
                        uuidMatch = true;
                        extraStr = "Transparent Flipper";
                    } else if (serviceUUID.toString().find("308") != std::string::npos) {
                        uuidMatch = true;
                        extraStr = "Generic Flipper";
                    }
                }
                if (macMatch || uuidMatch) {
                    match = true;
                    if (extraStr.isEmpty()) extraStr = "Flipper MAC Match";
                }
                break;
            }

            case BLEDetectorType::CARD_SKIMMER: {
                // Names HC-03, HC-05, HC-06
                if (name.startsWith("HC-03") || name.startsWith("HC-05") || name.startsWith("HC-06")) {
                    match = true;
                    extraStr = "HC Skimmer";
                }
                break;
            }

            case BLEDetectorType::MESHCORE: {
                // MeshCore 128-bit service UUID: 9E CA DC 24 0E E5 A9 E0 93 F3 A3 B5 01 00 40 6E or name prefix "MeshCore-"
                bool nameMatch = name.startsWith("MeshCore-");
                bool uuidMatch = false;
                if (advertisedDevice->haveServiceUUID()) {
                    uint8_t meshcoreUUIDBytes[16] = {
                        0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                        0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E
                    };
                    NimBLEUUID targetUUID(meshcoreUUIDBytes, 16);
                    NimBLEUUID targetUUIDRev = targetUUID;
                    targetUUIDRev.reverseByteOrder();
                    NimBLEUUID devUUID = advertisedDevice->getServiceUUID();
                    if (devUUID == targetUUID || devUUID == targetUUIDRev || devUUID.toString().find("6e400001") != std::string::npos) {
                        uuidMatch = true;
                    }
                }
                if (nameMatch || uuidMatch) {
                    match = true;
                    extraStr = "MeshCore Node";
                }
                break;
            }

            case BLEDetectorType::MESHTASTIC: {
                // Meshtastic 128-bit service UUID: FD EA 73 E2 CA 5D A8 9F 1F 46 A8 15 18 B2 A1 6B
                if (advertisedDevice->haveServiceUUID()) {
                    uint8_t meshtasticUUIDBytes[16] = {
                        0xFD, 0xEA, 0x73, 0xE2, 0xCA, 0x5D, 0xA8, 0x9F,
                        0x1F, 0x46, 0xA8, 0x15, 0x18, 0xB2, 0xA1, 0x6B
                    };
                    NimBLEUUID targetUUID(meshtasticUUIDBytes, 16);
                    NimBLEUUID targetUUIDRev = targetUUID;
                    targetUUIDRev.reverseByteOrder();
                    NimBLEUUID devUUID = advertisedDevice->getServiceUUID();
                    if (devUUID == targetUUID || devUUID == targetUUIDRev || devUUID.toString().find("6ba1b218") != std::string::npos) {
                        match = true;
                        extraStr = "Meshtastic Node";
                    }
                }
                break;
            }

            case BLEDetectorType::NYANBOX: {
                // nyanBOX 128-bit service UUID: 21 65 63 69 76 72 65 73 2d 58 4f 42 6e 61 79 6e
                if (advertisedDevice->haveServiceUUID()) {
                    uint8_t nyanboxUUIDBytes[16] = {
                        0x21, 0x65, 0x63, 0x69, 0x76, 0x72, 0x65, 0x73,
                        0x2d, 0x58, 0x4f, 0x42, 0x6e, 0x61, 0x79, 0x6e
                    };
                    NimBLEUUID targetUUID(nyanboxUUIDBytes, 16);
                    NimBLEUUID targetUUIDRev = targetUUID;
                    targetUUIDRev.reverseByteOrder();
                    NimBLEUUID devUUID = advertisedDevice->getServiceUUID();
                    if (devUUID == targetUUID || devUUID == targetUUIDRev || devUUID.toString().find("6e79616e") != std::string::npos) {
                        match = true;
                        extraStr = "nyanBOX Node";
                    }
                }
                break;
            }
        }

        if (match) {
            if (devicesMutex && xSemaphoreTake(devicesMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                // Check if we are in locate mode and target doesn't match
                if (locateView && !targetMacAddress.isEmpty()) {
                    if (address != targetMacAddress) {
                        xSemaphoreGive(devicesMutex);
                        return;
                    }
                }

                // Find existing device to update
                bool found = false;
                for (auto& dev : detectedDevices) {
                    if (dev.address == address) {
                        dev.rssi = rssi;
                        dev.lastSeen = millis();
                        dev.payload = payload;
                        if (!name.isEmpty() && name != "Unknown") {
                            dev.name = name;
                        }
                        found = true;
                        break;
                    }
                }

                if (!found && !locateView) {
                    if (detectedDevices.size() < 100) {
                        ScannedDevice newDev;
                        newDev.address = address;
                        newDev.name = name.isEmpty() ? "Unknown" : name;
                        newDev.rssi = rssi;
                        newDev.lastSeen = millis();
                        newDev.extraInt1 = extraInt;
                        newDev.extraStr1 = extraStr;
                        newDev.payload = payload;
                        detectedDevices.push_back(newDev);
                    }
                }
                xSemaphoreGive(devicesMutex);
            }
        }
    }
};

static NativeScanCallbacks scanCallbackInstance;

// Main draw routine
static void drawUI() {
    if (devicesMutex == nullptr || xSemaphoreTake(devicesMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return;
    }

    tft.fillScreen(bruceConfig.bgColor);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    // Header title mapping
    String headerTitle = "Detector";
    switch (currentDetectorType) {
        case BLEDetectorType::DEVICE_SCOUT: headerTitle = "Device Scout"; break;
        case BLEDetectorType::CAMERA:       headerTitle = "Camera Detector"; break;
        case BLEDetectorType::IBEACON:      headerTitle = "iBeacon Detector"; break;
        case BLEDetectorType::AIRTAG:       headerTitle = "AirTag Detector"; break;
        case BLEDetectorType::SMARTTAG:     headerTitle = "SmartTag Trk"; break;
        case BLEDetectorType::TILE:         headerTitle = "Tile Tracker"; break;
        case BLEDetectorType::RAYBAN_META:   headerTitle = "Ray-Ban Meta"; break;
        case BLEDetectorType::AXON_BODYCAM:   headerTitle = "Axon Bodycam"; break;
        case BLEDetectorType::FLIPPER_ZERO:   headerTitle = "Flipper Zero"; break;
        case BLEDetectorType::CARD_SKIMMER:   headerTitle = "Card Skimmer"; break;
        case BLEDetectorType::MESHCORE:       headerTitle = "MeshCore"; break;
        case BLEDetectorType::MESHTASTIC:     headerTitle = "Meshtastic"; break;
        case BLEDetectorType::NYANBOX:        headerTitle = "nyanBOX"; break;
    }

    bool isOLED = (tftHeight <= 80);

    if (locateView) {
        // Find locate target device
        ScannedDevice targetDev;
        bool targetFound = false;
        for (const auto& dev : detectedDevices) {
            if (dev.address == targetMacAddress) {
                targetDev = dev;
                targetFound = true;
                break;
            }
        }

        char maskedName[32];
        getMaskedName(targetFound ? targetDev.name : "Unknown", maskedName, sizeof(maskedName) - 1);

        char maskedMac[18];
        getMaskedMAC(targetMacAddress, maskedMac);

        // Draw RSSI signal meter
        int rssi = targetFound ? targetDev.rssi : -100;
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
            tft.drawString(maskedName, 8, 14);
            tft.drawString(String(maskedMac) + " " + String(rssi), 8, 24);
            tft.drawString(String("Sig: ") + quality, 8, 34);

            // Draw compact OLED bar graph
            int barWidth = 10;
            int barSpacing = 4;
            int startX = 8;
            int baseY = 52;

            for (int i = 0; i < 5; i++) {
                int barHeight = 4 + (i * 2);
                int x = startX + (i * (barWidth + barSpacing));
                int y = baseY - barHeight;

                if (i < signalLevel) {
                    tft.fillRect(x, y, barWidth, barHeight, bruceConfig.priColor);
                } else {
                    tft.drawRect(x, y, barWidth, barHeight, bruceConfig.priColor);
                }
            }

            tft.drawString("ESC=Back  SEL=Exit", 8, 54);
        } else {
            tft.setTextSize(FP);
            tft.drawString("Locating target...", 8, 8);
            tft.drawString(maskedName, 8, 20);
            tft.drawString(maskedMac, 8, 32);

            tft.setTextSize(FM);
            String rssiStr = "RSSI: " + String(rssi) + " dBm";
            tft.drawString(rssiStr, 8, 48);

            tft.setTextSize(FP);
            tft.drawString(String("Signal: ") + quality, 8, 68);

            // Draw modern OLED bar graph
            int barWidth = 14;
            int barSpacing = 6;
            int startX = 8;
            int baseY = 110;

            for (int i = 0; i < 5; i++) {
                int barHeight = 8 + (i * 5);
                int x = startX + (i * (barWidth + barSpacing));
                int y = baseY - barHeight;

                if (i < signalLevel) {
                    tft.fillRect(x, y, barWidth, barHeight, bruceConfig.priColor);
                } else {
                    tft.drawRect(x, y, barWidth, barHeight, bruceConfig.priColor);
                }
            }

            // Action instructions
            tft.drawString("ESC=Back  SEL=Exit", 8, 120);
        }

    } else if (detailView) {
        if (selectIdx >= 0 && selectIdx < (int)detectedDevices.size()) {
            const auto& dev = detectedDevices[selectIdx];
            char maskedName[32];
            getMaskedName(dev.name, maskedName, sizeof(maskedName) - 1);
            char maskedMac[18];
            getMaskedMAC(dev.address, maskedMac);
            unsigned long age = (millis() - dev.lastSeen) / 1000;
            String typeStr = dev.extraStr1.isEmpty() ? "Generic BLE" : dev.extraStr1;

            if (isOLED) {
                tft.setTextSize(FP);
                tft.drawString("Device Detail", 8, 2);
                tft.drawLine(8, 11, tftWidth - 8, 11, bruceConfig.priColor);
                tft.drawString("Name: " + String(maskedName), 8, 13);
                tft.drawString("MAC:  " + String(maskedMac), 8, 23);
                tft.drawString("Type: " + typeStr, 8, 33);
                tft.drawString("RSSI: " + String(dev.rssi) + "dBm  " + String(age) + "s", 8, 43);
                tft.drawString("ESC:Back SEL:Exit UP:Loc", 4, 54);
            } else {
                tft.setTextSize(FM);
                tft.drawString("Device Detail", 8, 8);
                tft.drawLine(8, 22, tftWidth - 8, 22, bruceConfig.priColor);

                tft.setTextSize(FP);
                tft.drawString("Name: " + String(maskedName), 8, 28);
                tft.drawString("MAC:  " + String(maskedMac), 8, 40);
                tft.drawString("Type: " + typeStr, 8, 52);
                tft.drawString("RSSI: " + String(dev.rssi) + " dBm", 8, 64);
                tft.drawString("Last Seen: " + String(age) + "s ago", 8, 76);
                tft.drawString("Payload size: " + String(dev.payload.size()) + " B", 8, 88);
                tft.drawString("ESC=Back  SEL=Exit  UP=Locate", 8, 115);
            }
        } else {
            detailView = false;
        }
    } else {
        // Scanner list view
        if (isOLED) {
            tft.setTextSize(FP);
            tft.drawString(headerTitle, 8, 2);
            tft.drawLine(8, 11, tftWidth - 8, 11, bruceConfig.priColor);

            if (detectedDevices.empty()) {
                tft.drawString("Scanning...", 8, 18);
                tft.drawString("Wait for beacons", 8, 28);
                tft.drawString("ESC to exit", 8, 44);
            } else {
                // Scrollable list (show 3 items on OLED)
                selectIdx = constrain(selectIdx, 0, (int)detectedDevices.size() - 1);
                if (selectIdx < startListIdx) {
                    startListIdx = selectIdx;
                } else if (selectIdx >= startListIdx + 3) {
                    startListIdx = selectIdx - 2;
                }

                for (int i = 0; i < 3; i++) {
                    int itemIdx = startListIdx + i;
                    if (itemIdx >= (int)detectedDevices.size()) break;

                    const auto& dev = detectedDevices[itemIdx];
                    int yPos = 14 + (i * 12);

                    // Highlight selection cursor
                    if (itemIdx == selectIdx) {
                        tft.fillRect(4, yPos - 1, tftWidth - 8, 11, bruceConfig.priColor);
                        tft.setTextColor(bruceConfig.bgColor, bruceConfig.priColor);
                    } else {
                        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                    }

                    char maskedName[32];
                    getMaskedName(dev.name, maskedName, sizeof(maskedName) - 1);
                    
                    String displayLine = String(maskedName) + " (" + String(dev.rssi) + ")";
                    tft.drawString(displayLine, 8, yPos);
                }

                // Restore colors
                tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                tft.drawString("U/D:Nav SEL:Det ESC:Ex", 4, 54);
            }
        } else {
            tft.setTextSize(FM);
            tft.drawString(headerTitle, 8, 8);
            tft.drawLine(8, 22, tftWidth - 8, 22, bruceConfig.priColor);

            tft.setTextSize(FP);

            if (detectedDevices.empty()) {
                tft.drawString("Scanning...", 8, 40);
                tft.drawString("Please wait for beacons", 8, 60);
                tft.drawString("ESC to cancel/exit", 8, 110);
            } else {
                // Scrollable list
                selectIdx = constrain(selectIdx, 0, (int)detectedDevices.size() - 1);
                if (selectIdx < startListIdx) {
                    startListIdx = selectIdx;
                } else if (selectIdx >= startListIdx + 4) {
                    startListIdx = selectIdx - 3;
                }

                for (int i = 0; i < 4; i++) {
                    int itemIdx = startListIdx + i;
                    if (itemIdx >= (int)detectedDevices.size()) break;

                    const auto& dev = detectedDevices[itemIdx];
                    int yPos = 28 + (i * 20);

                    // Highlight selection cursor
                    if (itemIdx == selectIdx) {
                        tft.fillRect(4, yPos - 2, tftWidth - 8, 18, bruceConfig.priColor);
                        tft.setTextColor(bruceConfig.bgColor, bruceConfig.priColor);
                    } else {
                        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                    }

                    char maskedName[32];
                    getMaskedName(dev.name, maskedName, sizeof(maskedName) - 1);
                    
                    String displayLine = String(maskedName) + " (" + String(dev.rssi) + ")";
                    tft.drawString(displayLine, 12, yPos);
                }

                // Restore colors
                tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

                // Instructions footer
                tft.drawString("UP/DN=Nav  SEL=Details  ESC=Exit", 8, 115);
            }
        }
    }

    tft.flush();
    xSemaphoreGive(devicesMutex);
}

void runNativeBLEDetector(BLEDetectorType type) {
    if (devicesMutex == nullptr) {
        devicesMutex = xSemaphoreCreateMutex();
    }

    // Save screen/menu state
    tft.fillScreen(bruceConfig.bgColor);
    
    // Set parameters
    currentDetectorType = type;
    
    if (devicesMutex && xSemaphoreTake(devicesMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        detectedDevices.clear();
        xSemaphoreGive(devicesMutex);
    } else {
        detectedDevices.clear();
    }

    selectIdx = 0;
    startListIdx = 0;
    detailView = false;
    locateView = false;
    targetMacAddress = "";

    // Safely configure and setup NimBLE scan
    // De-init first to avoid duplicate tasks or collision
    if (BLEStateManager::isBLEActive()) {
        BLEStateManager::deinitBLE(true);
    }
    
    // Initialize BLE natively
    BLEStateManager::initBLE("TanbreWatch-Scout", ESP_PWR_LVL_P9);
    
    pNativeScan = NimBLEDevice::getScan();
    if (pNativeScan) {
        pNativeScan->setScanCallbacks(&scanCallbackInstance);
        pNativeScan->setActiveScan(true);
        pNativeScan->setInterval(100);
        pNativeScan->setWindow(99);

        // Start scanning
        // Start async scan with a long duration (e.g. 9999 seconds) or infinite
        pNativeScan->start(0, false, true);
    }

    unsigned long lastDrawTime = 0;
    bool needsDraw = true;

    // Reset button states
    check(UpPress);
    check(DownPress);
    check(SelPress);
    check(EscPress);

    while (true) {
        unsigned long currentMillis = millis();

        // Feed Watchdog
        yield();

        // Process button presses
        if (check(UpPress)) {
            if (locateView) {
                // Do nothing in locate view
            } else if (detailView) {
                // Enter locate view
                locateView = true;
                if (devicesMutex && xSemaphoreTake(devicesMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    if (selectIdx >= 0 && selectIdx < (int)detectedDevices.size()) {
                        targetMacAddress = detectedDevices[selectIdx].address;
                    }
                    xSemaphoreGive(devicesMutex);
                }
                locateLastUpdate = currentMillis;
                needsDraw = true;
            } else {
                if (selectIdx > 0) {
                    selectIdx--;
                    needsDraw = true;
                }
            }
        }

        if (check(DownPress)) {
            if (!detailView && !locateView) {
                int devSize = 0;
                if (devicesMutex && xSemaphoreTake(devicesMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    devSize = detectedDevices.size();
                    xSemaphoreGive(devicesMutex);
                }
                if (selectIdx < devSize - 1) {
                    selectIdx++;
                    needsDraw = true;
                }
            }
        }

        if (check(SelPress)) {
            if (locateView) {
                // Exit all the way back to main menu
                break;
            } else if (detailView) {
                // Exit to main menu
                break;
            } else {
                bool isEmpty = true;
                if (devicesMutex && xSemaphoreTake(devicesMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    isEmpty = detectedDevices.empty();
                    xSemaphoreGive(devicesMutex);
                }
                if (!isEmpty) {
                    detailView = true;
                    needsDraw = true;
                } else {
                    // Exit to menu if list empty
                    break;
                }
            }
        }

        if (check(EscPress)) {
            if (locateView) {
                locateView = false;
                needsDraw = true;
            } else if (detailView) {
                detailView = false;
                needsDraw = true;
            } else {
                // Exit detector module
                break;
            }
        }

        // Draw periodically or on state change
        if (needsDraw || (currentMillis - lastDrawTime > 1000)) {
            drawUI();
            needsDraw = false;
            lastDrawTime = currentMillis;
        }

        // Delay to prevent CPU hogging and let RTOS button polling run
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // Cleanup scan and stop BLE
    if (pNativeScan) {
        pNativeScan->stop();
        pNativeScan->setScanCallbacks(nullptr);
    }
    BLEStateManager::deinitBLE(true);

    // Refresh display
    tft.fillScreen(bruceConfig.bgColor);
}

// Submenu mappings
void runNativeDeviceScout() {
    runNativeBLEDetector(BLEDetectorType::DEVICE_SCOUT);
}

void runNativeCameraDetector() {
    runNativeBLEDetector(BLEDetectorType::CAMERA);
}

void runNativeIBeaconDetector() {
    runNativeBLEDetector(BLEDetectorType::IBEACON);
}

void runNativeAirtagDetector() {
    runNativeBLEDetector(BLEDetectorType::AIRTAG);
}

void runNativeSmarttagDetector() {
    runNativeBLEDetector(BLEDetectorType::SMARTTAG);
}

void runNativeTileDetector() {
    runNativeBLEDetector(BLEDetectorType::TILE);
}

void runNativeRaybanDetector() {
    runNativeBLEDetector(BLEDetectorType::RAYBAN_META);
}

void runNativeAxonDetector() {
    runNativeBLEDetector(BLEDetectorType::AXON_BODYCAM);
}

void runNativeFlipperzeroDetector() {
    runNativeBLEDetector(BLEDetectorType::FLIPPER_ZERO);
}

void runNativeCardskimmerDetector() {
    runNativeBLEDetector(BLEDetectorType::CARD_SKIMMER);
}

void runNativeMeshcoreDetector() {
    runNativeBLEDetector(BLEDetectorType::MESHCORE);
}

void runNativeMeshtasticDetector() {
    runNativeBLEDetector(BLEDetectorType::MESHTASTIC);
}

void runNativeNyanboxDetector() {
    runNativeBLEDetector(BLEDetectorType::NYANBOX);
}
