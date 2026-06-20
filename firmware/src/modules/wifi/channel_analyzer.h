/*
    nyanBOX by Nyan Devices
    https://github.com/jbohack/nyanBOX
    Copyright (c) 2025 jbohack

    Licensed under the MIT License
    https://opensource.org/licenses/MIT

    SPDX-License-Identifier: MIT
*/

#ifndef CHANNEL_MONITOR_H
#define CHANNEL_MONITOR_H


#include <Arduino.h>


void channelAnalyzerSetup();
void channelAnalyzerLoop();

void drawNetworkCountView();
void drawSignalStrengthView();
const char* getSignalStrengthLabel(int rssi);

void runChannelAnalyzer();
#endif