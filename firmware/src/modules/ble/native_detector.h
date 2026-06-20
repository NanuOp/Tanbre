#ifndef NATIVE_DETECTOR_H
#define NATIVE_DETECTOR_H

#include <Arduino.h>

enum class BLEDetectorType {
    DEVICE_SCOUT,
    CAMERA,
    IBEACON,
    AIRTAG,
    SMARTTAG,
    TILE,
    RAYBAN_META,
    AXON_BODYCAM,
    FLIPPER_ZERO,
    CARD_SKIMMER,
    MESHCORE,
    MESHTASTIC,
    NYANBOX
};

void runNativeBLEDetector(BLEDetectorType type);

// Individual wrapper functions to match the menu callbacks
void runNativeDeviceScout();
void runNativeCameraDetector();
void runNativeIBeaconDetector();
void runNativeAirtagDetector();
void runNativeSmarttagDetector();
void runNativeTileDetector();
void runNativeRaybanDetector();
void runNativeAxonDetector();
void runNativeFlipperzeroDetector();
void runNativeCardskimmerDetector();
void runNativeMeshcoreDetector();
void runNativeMeshtasticDetector();
void runNativeNyanboxDetector();

#endif // NATIVE_DETECTOR_H
