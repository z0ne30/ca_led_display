#ifndef MATRIX_CONTROLLER_H
#define MATRIX_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Protomatter.h>

class MatrixController {
  public:
    // Constructor - sets up matrix display with specific pin configurations
    MatrixController(
      uint8_t rgbPins[],
      uint8_t addrPins[],
      uint8_t clockPin,
      uint8_t latchPin,
      uint8_t oePin,
      uint8_t width,
      uint8_t height,
      uint8_t panels = 1,
      bool doubleBuffer = true,
      int8_t tileMode = 0
    );
    
    // Initialize the display
    bool begin();
    
    // Clear the display
    void clear();
    
    // Show buffer on display (when double buffering)
    void show();
    
    // Set a pixel at specific coordinates with RGB color
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    
    // Fill the entire matrix with a color
    void fillScreen(uint16_t color);
    
    // Get a reference to the underlying display object
    Adafruit_Protomatter* getDisplay();
    
    // Draw a rectangle on the display
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    
    // Method to get matrix dimensions
    int16_t width();
    int16_t height();
    
    // 16-bit color conversion functions
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    
    // Common colors
    static const uint16_t BLACK = 0x0000;
    static const uint16_t WHITE = 0xFFFF;
    static const uint16_t RED = 0xF800;
    static const uint16_t GREEN = 0x07E0;
    static const uint16_t BLUE = 0x001F;
    static const uint16_t YELLOW = 0xFFE0;
    static const uint16_t CYAN = 0x07FF;
    static const uint16_t MAGENTA = 0xF81F;
    
  private:
    Adafruit_Protomatter* matrix; // Our LED matrix display object
    uint8_t matrixWidth;
    uint8_t matrixHeight;
    uint8_t matrixPanels;
};

#endif
