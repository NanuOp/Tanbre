#include <Arduino.h>
/*
    nyanBOX by Nyan Devices
    https://github.com/jbohack/nyanBOX
    Copyright (c) 2025 jbohack

    Licensed under the MIT License
    https://opensource.org/licenses/MIT

    SPDX-License-Identifier: MIT
*/

#ifndef BEACON_SPAM_H
#define BEACON_SPAM_H

#include "esp_wifi.h"
#include <vector>

#include <stdint.h>


void beaconSpamSetup();
void beaconSpamLoop();

void runBeaconSpam();
#endif
