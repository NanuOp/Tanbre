/*
 * Snake Game - Ported from nyanBOX to Tanbre Watch
 * Original by jbohack (MIT License)
 */
#ifndef SNAKE_H
#define SNAKE_H

#include <Arduino.h>

#define SNAKE_CELL   4
#define SNAKE_COLS  (128 / SNAKE_CELL)
#define SNAKE_ROWS  ( 64 / SNAKE_CELL)
#define SNAKE_MAX   (SNAKE_COLS * SNAKE_ROWS)

void snakeSetup();
void snakeLoop();
void snakeCleanup();
void runSnake();

#endif
