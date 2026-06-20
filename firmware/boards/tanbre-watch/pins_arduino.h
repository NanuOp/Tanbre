#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "soc/soc_caps.h"
#include <stdint.h>

// Main I2C Bus for SSD1306/SH1106 Display
#define GROVE_SDA 5   // D4 / GPIO 5
#define GROVE_SCL 6   // D5 / GPIO 6
static const uint8_t SDA = GROVE_SDA;
static const uint8_t SCL = GROVE_SCL;

// No external SPI peripherals are populated on this hardware revision.
#define SPI_SS_PIN -1
#define SPI_MOSI_PIN -1
#define SPI_MISO_PIN -1
#define SPI_SCK_PIN -1
static const uint8_t SS = SPI_SS_PIN;
static const uint8_t MOSI = SPI_MOSI_PIN;
static const uint8_t MISO = SPI_MISO_PIN;
static const uint8_t SCK = SPI_SCK_PIN;

// Display config (LovyanGFX SSD1306/SH1106 I2C Driver)
#define USE_LOVYANGFX 1
#define LOVYAN_PANEL Panel_SH110x // LovyanGFX uses Panel_SH110x for I2C OLEDs
#define LOVYAN_BUS Bus_I2C
#define LOVYAN_I2C_BUS 1

#define TFT_I2C_PORT  0
#define TFT_I2C_WRITE 100000
#define TFT_I2C_READ  100000
#define TFT_SDA GROVE_SDA
#define TFT_SCL GROVE_SCL
#define TFT_ADDR 0x3C

#define TFT_CS        -1
#define TFT_RST       -1
#define TFT_BUSY_PIN  -1
#define TFT_WIDTH     128
#define TFT_HEIGHT    64
#define TFT_OFFSET_X  2
#define TFT_OFFSET_Y  0
#define TFT_INVERTION 1
#define TFT_RGB_ORDER 0
#define TFT_MEM_WIDTH  132
#define TFT_MEM_HEIGHT 64

// Display settings for Bruce
#define HAS_SCREEN
#define ROTATION 0
#define MINBRIGHT (uint8_t)1
#define PIN_POWER_ON -1

// Font Sizes scaled for 128x64 display
#define FP 1
#define FM 1
#define FG 2


// NRF24 Configuration (not populated)
#define NRF24_CE_PIN -1
#define NRF24_SS_PIN -1
#define NRF24_MOSI_PIN -1
#define NRF24_SCK_PIN -1
#define NRF24_MISO_PIN -1

// SD Card Configuration (not populated)
#define SDCARD_CS -1
#define SDCARD_MOSI -1
#define SDCARD_MISO -1
#define SDCARD_SCK -1

// UART/GPS peripherals (not populated)
#define SERIAL_TX -1
#define SERIAL_RX -1
#define GPS_SERIAL_TX -1
#define GPS_SERIAL_RX -1

// Haptic feedback motor (not populated)
#define HAPTIC_MOTOR_PIN -1

// IR Receiver/Transmitter (not populated)
#define RXLED -1
#define TXLED -1

// Analog microphone (not populated)
#define ANALOG_MIC_PIN -1

// Physical buttons (not populated)
#define SEL_BTN -1
#define UP_BTN -1
#define DW_BTN -1
#define BTN_ACT LOW

// BadUSB Dummy Pins (Not used)
#define BAD_RX -1
#define BAD_TX -1

#endif /* Pins_Arduino_h */
