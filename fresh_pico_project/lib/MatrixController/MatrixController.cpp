#include "MatrixController.h"

MatrixController::MatrixController(
  uint8_t rgbPins[],
  uint8_t addrPins[],
  uint8_t clockPin,
  uint8_t latchPin,
  uint8_t oePin,
  uint8_t width,
  uint8_t height,
  uint8_t panels,
  bool doubleBuffer,
  int8_t tileMode
) {
  matrixWidth = width;
  matrixHeight = height;
  matrixPanels = panels;
  
  matrix = new Adafruit_Protomatter(
    width,             // Width of matrix (or matrix chain) IN PIXELS
    4,                 // Bit depth per channel (4 = 16 shades of each R,G,B)
    1, rgbPins,        // # of data pin pairs, array of RGB pins
    5, addrPins,       // # of address pins (height is 2^n), array of address pins
    clockPin,          // Clock pin
    latchPin,          // Latch pin
    oePin,             // Output enable pin
    doubleBuffer,      // Use double-buffering
    tileMode,          // Tiling mode (0=none, 1=serpentine, 2=progressive)
    NULL               // Timer (NULL = default)
  );
}

bool MatrixController::begin() {
  // Initialize the matrix
  ProtomatterStatus status = matrix->begin();
  if (status != PROTOMATTER_OK) {
    Serial1.print("Protomatter init failed: ");
    Serial1.println((int)status);
    return false;
  }
  return true;
}

void MatrixController::clear() {
  matrix->fillScreen(0);
}

void MatrixController::show() {
  matrix->show();
}

void MatrixController::drawPixel(int16_t x, int16_t y, uint16_t color) {
  matrix->drawPixel(x, y, color);
}

void MatrixController::fillScreen(uint16_t color) {
  matrix->fillScreen(color);
}

Adafruit_Protomatter* MatrixController::getDisplay() {
  return matrix;
}

void MatrixController::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  matrix->drawRect(x, y, w, h, color);
}

void MatrixController::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  matrix->fillRect(x, y, w, h, color);
}

int16_t MatrixController::width() {
  return matrixWidth * matrixPanels;
}

int16_t MatrixController::height() {
  return matrixHeight;
}

uint16_t MatrixController::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
