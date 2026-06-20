/*
 * nyanBOX Compatibility Layer for Tanbre Watch
 * Maps nyanBOX U8G2/button APIs to Bruce/LovyanGFX framework
 */
#ifndef NYANBOX_COMPAT_H
#define NYANBOX_COMPAT_H

#include <globals.h>
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "esp_wifi.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

// ─── Button Compatibility ───────────────────────────────────────────
#define BUTTON_PIN_UP     100
#define BUTTON_PIN_DOWN   101
#define BUTTON_PIN_RIGHT  102 // Maps to Select
#define BUTTON_PIN_LEFT   103 // Maps to Back
#define BUTTON_PIN_CENTER 104 // Maps to Select

#define BTN_UP      BUTTON_PIN_UP
#define BTN_DOWN    BUTTON_PIN_DOWN
#define BTN_RIGHT   BUTTON_PIN_RIGHT
#define BTN_BACK    BUTTON_PIN_LEFT
#define BTN_CENTER  BUTTON_PIN_CENTER

inline int nyan_digitalRead(int pin) {
    if (pin == BTN_UP)     return check(PrevPress) ? LOW : HIGH;
    if (pin == BTN_DOWN)   return check(NextPress) ? LOW : HIGH;
    if (pin == BTN_RIGHT)  return check(SelPress) ? LOW : HIGH;
    if (pin == BTN_BACK)   return check(EscPress) ? LOW : HIGH;
    if (pin == BTN_CENTER) return check(SelPress) ? LOW : HIGH;
    return HIGH;
}

#define digitalRead(pin) nyan_digitalRead(pin)
#define pinMode(pin, mode) (void)0

// ─── Display Compatibility ──────────────────────────────────────────
class NyanDisplay {
public:
    uint8_t fontSize = 1;

    void begin() {}
    void clearBuffer() {
        tft.fillScreen(bruceConfig.bgColor);
    }
    void sendBuffer() {}
    void setFont(const void* font) {
        (void)font;
        tft.setTextSize(FP);
        fontSize = FP;
    }
    void drawStr(int x, int y, const char* str) {
        int adjustedY = y - 8;
        if (adjustedY < 0) adjustedY = 0;
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        tft.drawString(str, x, adjustedY);
    }
    void drawFrame(int x, int y, int w, int h) {
        tft.drawRect(x, y, w, h, bruceConfig.priColor);
    }
    void drawBox(int x, int y, int w, int h) {
        tft.fillRect(x, y, w, h, bruceConfig.priColor);
    }
    void drawRBox(int x, int y, int w, int h, int r) {
        tft.fillRoundRect(x, y, w, h, r, bruceConfig.priColor);
    }
    void drawRFrame(int x, int y, int w, int h, int r) {
        tft.drawRoundRect(x, y, w, h, r, bruceConfig.priColor);
    }
    void drawLine(int x0, int y0, int x1, int y1) {
        tft.drawLine(x0, y0, x1, y1, bruceConfig.priColor);
    }
    void drawCircle(int x, int y, int r) {
        tft.drawCircle(x, y, r, bruceConfig.priColor);
    }
    void drawDisc(int x, int y, int r) {
        tft.fillCircle(x, y, r, bruceConfig.priColor);
    }
    void drawPixel(int x, int y) {
        tft.drawPixel(x, y, bruceConfig.priColor);
    }
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2) {
        tft.drawTriangle(x0, y0, x1, y1, x2, y2, bruceConfig.priColor);
    }
    void drawHLine(int x, int y, int w) {
        tft.drawFastHLine(x, y, w, bruceConfig.priColor);
    }
    void drawVLine(int x, int y, int h) {
        tft.drawFastVLine(x, y, h, bruceConfig.priColor);
    }
    int getStrWidth(const char* str) {
        return tft.textWidth(str);
    }
    int getUTF8Width(const char* str) {
        return tft.textWidth(str);
    }
    void print(const char* str) {
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        tft.print(str);
    }
    void print(int val) {
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        tft.print(val);
    }
    void setCursor(int x, int y) {
        int adjustedY = y - 8;
        if (adjustedY < 0) adjustedY = 0;
        tft.setCursor(x, adjustedY);
    }
};

extern NyanDisplay u8g2;

// ─── Module Runner ──────────────────────────────────────────────────
typedef void (*NyanSetupFn)();
typedef void (*NyanLoopFn)();
typedef void (*NyanCleanupFn)();

void runNyanModule(NyanSetupFn setup, NyanLoopFn loop, NyanCleanupFn cleanup = nullptr);

// ─── Radio Manager Declarations ──────────────────────────────────────
bool initBLE();
void cleanupBLE();
bool initWiFi(wifi_mode_t mode);
void cleanupWiFi();
void cleanupRadio();

// ─── Stub Functions ─────────────────────────────────────────────────
#define displayMirrorSend(...) (void)0
#define resetSleepTimer() (void)0
#define checkIdle() (void)0

inline bool isDangerousActionsEnabled() { return true; }
inline bool isContinuousScanEnabled() { return true; }
inline bool isPrivacyModeEnabled() { return false; }

inline void maskMAC(const char* original, char* masked) {
    if (original && masked) strcpy(masked, original);
}
inline void maskName(const char* original, char* masked, int maxLen) {
    if (original && masked) {
        strncpy(masked, original, maxLen);
        masked[maxLen] = '\0';
    }
}
inline void maskNameEvilPortal(const char* original, char* masked, int maxLen, const char* customSSIDs[], int customSSIDCount) {
    (void)customSSIDs;
    (void)customSSIDCount;
    if (original && masked) {
        strncpy(masked, original, maxLen);
        masked[maxLen] = '\0';
    }
}

#define u8g2_font_6x10_tr  ((const void*)1)
#define u8g2_font_5x8_tr   ((const void*)2)
#define u8g2_font_7x13B_tr ((const void*)3)
#define u8g2_font_4x6_tr   ((const void*)4)
#define u8g2_font_profont11_tf ((const void*)5)
#define u8g2_font_ncenB08_tr   ((const void*)6)
#define u8g2_font_5x7_tr       ((const void*)7)
#define u8g2_font_helvB14_tr   ((const void*)8)
#define u8g2_font_helvB10_tr   ((const void*)9)
#define u8g2_font_helvB08_tr   ((const void*)10)
#define u8g2_font_helvR08_tr   ((const void*)11)
#define u8g2_font_6x12_tr      ((const void*)12)

#endif // NYANBOX_COMPAT_H
