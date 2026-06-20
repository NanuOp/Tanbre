/*
    nyanBOX by Nyan Devices
    https://github.com/jbohack/nyanBOX
    Copyright (c) 2025 jbohack

    Licensed under the MIT License
    https://opensource.org/licenses/MIT

    SPDX-License-Identifier: MIT
*/

#ifndef wifiscan_H
#define wifiscan_H




#include <Arduino.h>
#include <vector>

void wifiscanSetup();
void wifiscanLoop();
void wifiscanCleanup();

void runWifiscan();
#endif
