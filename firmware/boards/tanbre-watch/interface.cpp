#include "core/powerSave.h"
#include <globals.h>
#include <interface.h>
#include <Wire.h>
#include <Button.h>

volatile bool nxtPress = false;
volatile bool prvPress = false;
volatile bool ecPress = false;
volatile bool slPress = false;

static void onButtonSingleClickCb1(void *button_handle, void *usr_data) { nxtPress = true; } // Click D0 -> Next (Down)
static void onButtonDoubleClickCb1(void *button_handle, void *usr_data) { slPress = true; }  // Double-click D0 -> Select
static void onButtonHoldCb1(void *button_handle, void *usr_data) { slPress = true; }        // Hold D0 -> Select

static void onButtonSingleClickCb2(void *button_handle, void *usr_data) { prvPress = true; } // Click D2 -> Prev (Up)
static void onButtonDoubleClickCb2(void *button_handle, void *usr_data) { ecPress = true; }  // Double-click D2 -> Escape (Back)
static void onButtonHoldCb2(void *button_handle, void *usr_data) { ecPress = true; }        // Hold D2 -> Escape (Back)

Button *btn1;
Button *btn2;

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

    // Setup Buttons (D0/GPIO 1 and D2/GPIO 3)
    pinMode(DW_BTN, INPUT_PULLUP);
    pinMode(UP_BTN, INPUT_PULLUP);

    button_config_t bt1 = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 600,
        .short_press_time = 120,
        .gpio_button_config = {
                               .gpio_num = (int)DW_BTN,
                               .active_level = 0,
                               },
    };
    button_config_t bt2 = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 600,
        .short_press_time = 120,
        .gpio_button_config = {
                               .gpio_num = (int)UP_BTN,
                               .active_level = 0,
                               },
    };

    btn1 = new Button(bt1);
    btn1->attachSingleClickEventCb(&onButtonSingleClickCb1, NULL);
    btn1->attachDoubleClickEventCb(&onButtonDoubleClickCb1, NULL);
    btn1->attachLongPressStartEventCb(&onButtonHoldCb1, NULL);

    btn2 = new Button(bt2);
    btn2->attachSingleClickEventCb(&onButtonSingleClickCb2, NULL);
    btn2->attachDoubleClickEventCb(&onButtonDoubleClickCb2, NULL);
    btn2->attachLongPressStartEventCb(&onButtonHoldCb2, NULL);
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

    static unsigned long tm = 0;
    static bool btn_pressed = false;
    if (nxtPress || prvPress || ecPress || slPress) btn_pressed = true;

    if (millis() - tm > 200 || LongPress) {
        if (btn_pressed) {
            btn_pressed = false;
            tm = millis();
            if (!wakeUpScreen()) AnyKeyPress = true;
            else return;

            SelPress = slPress;
            EscPress = ecPress;
            NextPress = nxtPress;
            PrevPress = prvPress;

            nxtPress = false;
            prvPress = false;
            ecPress = false;
            slPress = false;
        }
    }
}

/*********************************************************************
** Function: keyboard
** Starts keyboard to type data
** **********************************************************************/
String keyboard(String mytext, int maxSize, String msg) {
    return mytext;
}

/*********************************************************************
** Function: powerOff
** Turns off the device (or try to)
** **********************************************************************/
void powerOff() {
    esp_sleep_enable_ext0_wakeup((gpio_num_t)DW_BTN, BTN_ACT);
    esp_deep_sleep_start();
}

/*********************************************************************
** Function: checkReboot
** Btn logic to turn off/reboot the device
** **********************************************************************/
void checkReboot() {}
