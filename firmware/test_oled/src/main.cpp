#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Toggle between SSD1306 and SH1106
const bool useSH1106 = true;

// Force the library constructor to target 100kHz
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 100000UL, 100000UL);

unsigned long counter = 0;
bool oledReady = false;

bool checkHardwareConnection() {
  Wire.beginTransmission(0x3C);
  return (Wire.endTransmission() == 0);
}

// Custom SH1106 Initialization Sequence
void initSH1106() {
  Wire.beginTransmission(0x3C);
  Wire.write(0x00); // Command stream
  Wire.write(0xAE); // Display OFF
  Wire.write(0xD5); // Set Display Clock Divide Ratio
  Wire.write(0x80);
  Wire.write(0xA8); // Set Multiplex Ratio
  Wire.write(0x3F); // 1/64 Duty
  Wire.write(0xD3); // Set Display Offset
  Wire.write(0x00); // No offset
  Wire.write(0x40); // Set Display Start Line
  Wire.write(0xAD); // Set Charge Pump
  Wire.write(0x8B); // Enable Charge Pump
  Wire.write(0xA1); // Set Segment Re-map (Mirror X)
  Wire.write(0xC8); // Set COM Output Scan Direction (Mirror Y)
  Wire.write(0xDA); // Set COM Pins Config
  Wire.write(0x12);
  Wire.write(0x81); // Set Contrast Control
  Wire.write(0xCF);
  Wire.write(0xD9); // Set Pre-charge Period
  Wire.write(0xF1);
  Wire.write(0xDB); // Set VCOMH Deselect Level
  Wire.write(0x40);
  Wire.write(0xA4); // Entire Display ON (Resume to RAM content)
  Wire.write(0xA6); // Normal Display (non-inverted)
  Wire.write(0xAF); // Display ON
  Wire.endTransmission();
}

// Custom Page-by-Page Draw for SH1106
void customDisplaySH1106() {
  uint8_t *buffer = display.getBuffer();
  
  for (uint8_t page = 0; page < 8; page++) {
    // 1. Send page address and column address commands
    Wire.beginTransmission(0x3C);
    Wire.write(0x00);              // Command stream
    Wire.write(0xB0 + page);       // Set page address
    Wire.write(0x02);              // Set lower column address (2-pixel offset for SH1106)
    Wire.write(0x10);              // Set higher column address
    Wire.endTransmission();
    
    // 2. Send 128 bytes of data for this page in 32-byte chunks to fit standard I2C buffers
    for (uint8_t chunk = 0; chunk < 4; chunk++) {
      Wire.beginTransmission(0x3C);
      Wire.write(0x40);            // Data stream
      Wire.write(&buffer[page * 128 + chunk * 32], 32);
      Wire.endTransmission();
    }
  }
}

void initDisplay() {
  Serial.println("Re-initializing I2C bus...");
  
  // 1. Reset/Start Wire
  Wire.begin(5, 6);
  Wire.setClock(100000);
  
  // 2. Perform a real hardware check
  if (!checkHardwareConnection()) {
    Serial.println("[ERROR] No physical contact with OLED at 0x3C! Check wiring.");
    oledReady = false;
    return;
  }
  
  // 3. Initialize display object
  // (periphBegin = false, reset = false to prevent pins from being overridden)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
  
  if (useSH1106) {
    initSH1106();
  }
  
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  
  // Clear display buffer
  display.clearDisplay();
  if (useSH1106) {
    customDisplaySH1106();
  } else {
    display.display();
  }
  
  Serial.println("[SUCCESS] OLED detected and initialized!");
  oledReady = true;
}

void setup() {
  Serial.begin(115200);
  
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) {
    delay(10);
  }
  
  Serial.println("\n--- Watchdog-Protected OLED Test (No False Positives) Start ---");
  initDisplay();
}

void loop() {
  // If the OLED is not ready or has lost connection, try to recover it
  if (!oledReady || !checkHardwareConnection()) {
    if (oledReady) {
      Serial.println("Connection lost! Resetting I2C...");
    }
    
    oledReady = false;
    Wire.end();
    delay(200);
    
    initDisplay();
    delay(1000); // Wait 1 second before trying again
    return;
  }
  
  counter++;
  Serial.printf("Uptime: %lu seconds (Display Active)\n", counter);
  
  display.clearDisplay();
  
  // Draw LUMINA logo
  display.setTextSize(2);
  display.setCursor(25, 10);
  display.print("LUMINA");
  
  // Draw counter
  display.setTextSize(1);
  display.setCursor(35, 45);
  display.printf("Uptime: %lu s", counter);
  
  if (useSH1106) {
    customDisplaySH1106();
  } else {
    display.display();
  }
  
  delay(1000);
}
