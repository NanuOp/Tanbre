/*
 * Snake Game - Ported from nyanBOX to Tanbre Watch
 * Original by jbohack (MIT License)
 */
#include <Arduino.h>
#include <EEPROM.h>
#include "snake.h"
#include "../detectors/nyanbox_compat.h"
#include <esp_system.h>

static uint8_t snakeX[SNAKE_MAX];
static uint8_t snakeY[SNAKE_MAX];
static uint16_t snakeLen;
static uint8_t dir;
static uint8_t appleX, appleY;
static unsigned long lastMove;
static const unsigned long INTERVAL = 200;
static uint16_t score = 0;
static uint16_t highScore = 0;
static uint16_t savedHighScore = 0;

static bool needsRedraw = true;
static uint8_t lastDir = 0;

// Highscore storage addresses
#define EEPROM_ADDRESS_HIGHSCORE_LOW 103
#define EEPROM_ADDRESS_HIGHSCORE_HIGH 104

static void loadHighScore() {
  uint8_t scoreLow = EEPROM.read(EEPROM_ADDRESS_HIGHSCORE_LOW);
  uint8_t scoreHigh = EEPROM.read(EEPROM_ADDRESS_HIGHSCORE_HIGH);
  uint16_t loadedScore = (scoreHigh << 8) | scoreLow;

  if (loadedScore == 0xFFFF || loadedScore > 512) {
    highScore = 0;
    savedHighScore = 0;
  } else {
    highScore = loadedScore;
    savedHighScore = loadedScore;
  }
}

static void saveHighScore() {
  if (highScore > savedHighScore && highScore <= 512) {
    EEPROM.write(EEPROM_ADDRESS_HIGHSCORE_LOW, highScore & 0xFF);
    EEPROM.write(EEPROM_ADDRESS_HIGHSCORE_HIGH, (highScore >> 8) & 0xFF);
    EEPROM.commit();
    savedHighScore = highScore;
  }
}

static void resetSnake() {
  snakeLen = 3;
  score = 0;
  snakeX[0] = SNAKE_COLS/2; snakeY[0] = SNAKE_ROWS/2;
  snakeX[1] = snakeX[0] - 1; snakeY[1] = snakeY[0];
  snakeX[2] = snakeX[1] - 1; snakeY[2] = snakeY[0];
  dir = 1;
  appleX = random(SNAKE_COLS);
  appleY = random(SNAKE_ROWS);
  lastMove = millis();
  needsRedraw = true;
}

void snakeSetup() {
  randomSeed((uint32_t)esp_random());
  loadHighScore();
  resetSnake();
  needsRedraw = true;
  lastDir = dir;
}

void snakeLoop() {
  if (digitalRead(BTN_UP) == LOW && dir != 2) {
    dir = 0;
  }
  else if (digitalRead(BTN_RIGHT) == LOW && dir != 3) {
    dir = 1;
  }
  else if (digitalRead(BTN_DOWN) == LOW && dir != 0) {
    dir = 2;
  }
  else if (digitalRead(BTN_BACK) == LOW && dir != 1) {
    dir = 3;
  }

  if (lastDir != dir) {
    lastDir = dir;
    needsRedraw = true;
  }

  if (millis() - lastMove >= INTERVAL) {
    lastMove = millis();
    needsRedraw = true;
    for (int i = snakeLen; i > 0; i--) {
      snakeX[i] = snakeX[i-1];
      snakeY[i] = snakeY[i-1];
    }
    switch (dir) {
      case 0: snakeY[0] = snakeY[0] ? snakeY[0]-1 : SNAKE_ROWS-1; break;
      case 1: snakeX[0] = snakeX[0] < SNAKE_COLS-1 ? snakeX[0]+1 : 0; break;
      case 2: snakeY[0] = snakeY[0] < SNAKE_ROWS-1 ? snakeY[0]+1 : 0; break;
      case 3: snakeX[0] = snakeX[0] ? snakeX[0]-1 : SNAKE_COLS-1; break;
    }
    for (int i = 1; i < snakeLen; i++) {
      if (snakeX[i] == snakeX[0] && snakeY[i] == snakeY[0]) {
        if (score > highScore) {
          highScore = score;
        }
        resetSnake();
        return;
      }
    }
    if (snakeX[0] == appleX && snakeY[0] == appleY) {
      score++;
      if (score > highScore) {
        highScore = score;
      }
      if (snakeLen < SNAKE_MAX-1) snakeLen++;
      appleX = random(SNAKE_COLS);
      appleY = random(SNAKE_ROWS);
    }
  }

  if (!needsRedraw) {
    return;
  }

  needsRedraw = false;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  
  // Custom print logic using shim
  char buf[32];
  snprintf(buf, sizeof(buf), "Score:%d", score);
  u8g2.drawStr(0, 10, buf);
  snprintf(buf, sizeof(buf), "Hi:%d", highScore);
  u8g2.drawStr(64, 10, buf);
  
  u8g2.drawBox(appleX*SNAKE_CELL, appleY*SNAKE_CELL, SNAKE_CELL, SNAKE_CELL);
  for (int i = 0; i < snakeLen; i++) {
    u8g2.drawFrame(snakeX[i]*SNAKE_CELL, snakeY[i]*SNAKE_CELL, SNAKE_CELL, SNAKE_CELL);
  }
  u8g2.sendBuffer();
}

void snakeCleanup() {
  saveHighScore();
}

void runSnake() {
  runNyanModule(snakeSetup, snakeLoop, snakeCleanup);
}
