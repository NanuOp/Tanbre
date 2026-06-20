#include "core/powerSave.h"
#include <globals.h>
#include <interface.h>
#include <Wire.h>

/***************************************************************************************
** Function name: _setup_gpio()
** Description:   initial setup for the device
** *************************************************************************************/
void _setup_gpio() {
    // Settle delay for OLED power rail on power-cycle.
    delay(500);

    // This hardware revision only populates the ESP32-S3 module, I2C OLED display,
    // and battery power path. Do not reserve GPIOs for absent external modules.
    Wire.begin(GROVE_SDA, GROVE_SCL);
    Wire.setClock(100000);

    bruceConfigPins.irRx = -1;
    bruceConfigPins.irTx = -1;
    bruceConfigPins.rfidModule = M5_RFID2_MODULE;
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Description:   second stage gpio setup to make a few functions work
** *************************************************************************************/
void _post_setup_gpio() {}

/***************************************************************************************
** Function name: getBattery()
** Description:   Delivers the battery value from 1-100
** *************************************************************************************/
int getBattery() {
    // No dedicated battery sense pin is configured for this board profile.
    return 100;
}

/*********************************************************************
** Function: setBrightness
** set brightness value
** **********************************************************************/
void _setBrightness(uint8_t brightval) {
    // OLED displays do not support analog backlights, contrast controls can be ignored here.
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
** **********************************************************************/
void InputHandler(void) {
    checkPowerSaveTime();

    PrevPress = false;
    NextPress = false;
    SelPress = false;
    AnyKeyPress = false;
    EscPress = false;
}

/*********************************************************************
** Function: keyboard
** Starts keyboard to type data
**********************************************************************/
String keyboard(String mytext, int maxSize, String msg) {
    return mytext;
}

/*********************************************************************
** Function: powerOff
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {
    esp_deep_sleep_start();
}

/*********************************************************************
** Function: checkReboot
** Btn logic to turn off/reboot the device
**********************************************************************/
void checkReboot() {}
