#ifndef AIRTAG_DETECTOR_H
#define AIRTAG_DETECTOR_H

#include <Arduino.h>
#include <vector>

struct AirTagDeviceData {
  char name[32];
  char address[18];
  int8_t rssi;
  unsigned long lastSeen;
  uint8_t payload[64];
  size_t payloadLength;
  bool isAirTag;
};

extern std::vector<AirTagDeviceData> airtagDevices;

void runAirtagDetector();

#endif // AIRTAG_DETECTOR_H
