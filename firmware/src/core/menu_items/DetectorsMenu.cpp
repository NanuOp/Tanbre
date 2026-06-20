#include "DetectorsMenu.h"
#include "core/display.h"
#include "core/utils.h"

// Include native BLE detector header
#include "modules/ble/native_detector.h"

// Include other specific detectors
#include "modules/detectors/pwnagotchi_detector.h"
#include "modules/detectors/drone_detector.h"
#include "modules/detectors/flock_detector.h"
#include "modules/wifi/pineapple_detector.h"

void DetectorsMenu::optionsMenu() {
    options = {
        {"Device Scout",        runNativeDeviceScout         },
        {"Camera Detector",     runNativeCameraDetector      },
        {"Pineapple Detector",  runPineappleDetector         },
        {"Pwnagotchi Detector", runPwnagotchiDetector        },
        {"SmartTag Detector",   runNativeSmarttagDetector    },
        {"Tile Detector",       runNativeTileDetector        },
        {"RayBan Detector",     runNativeRaybanDetector      },
        {"iBeacon Detector",    runNativeIBeaconDetector     },
        {"nyanBOX Detector",    runNativeNyanboxDetector     },
        {"Flipper Scanner",     runNativeFlipperzeroDetector },
        {"Axon Detector",       runNativeAxonDetector        },
        {"Meshtastic Detector", runNativeMeshtasticDetector  },
        {"MeshCore Detector",   runNativeMeshcoreDetector    },
        {"Skimmer Detector",    runNativeCardskimmerDetector },
        {"AirTag Detector",     runNativeAirtagDetector      },
        {"Drone Detector",      runDroneDetector             },
        {"Flock Detector",      runFlockDetector             },
    };

    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Detectors");
}

void DetectorsMenu::drawIcon(float scale) {
    clearIconArea();
    // Simple, clean text-only or geometric layout for Tanbre watch
    int size = scale * 12;
    tft.drawCircle(iconCenterX, iconCenterY, size, bruceConfig.priColor);
    tft.drawCircle(iconCenterX, iconCenterY, size / 2, bruceConfig.priColor);
}
