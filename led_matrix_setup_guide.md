# RGB LED Matrix Setup and Testing Guide

This guide will help you set up PlatformIO for your Raspberry Pi Pico and configure your 4-panel RGB matrix display in a custom arrangement.

## Part 1: PlatformIO Setup

### 1.1 Install PlatformIO

1. **Install Visual Studio Code** (if not already installed)
   - Download from [https://code.visualstudio.com/](https://code.visualstudio.com/)

2. **Install PlatformIO Extension**
   - Open VS Code
   - Go to Extensions (Ctrl+Shift+X or Cmd+Shift+X)
   - Search for "PlatformIO IDE"
   - Click Install

3. **Restart VS Code** after installation

### 1.2 Set Up the Project

1. **Clone or Download the Project**
   - If starting fresh, you can use the `fresh_pico_project` as a template

2. **Open the Project in PlatformIO**
   - In VS Code, click the PlatformIO icon in the sidebar
   - Select "Open Project"
   - Navigate to the project folder
   - Open the project

3. **Configure platformio.ini**
   - The existing configuration looks good, but we'll add multi-panel support:

```ini
[env:pico]
platform = https://github.com/earlephilhower/platform-raspberrypi.git
board = rpipico
framework = arduino

; Libraries for RGB matrix
lib_deps = 
	adafruit/Adafruit Protomatter@^1.5.1
	adafruit/Adafruit GFX Library@^1.11.9

lib_ignore = 
    SdFat - Adafruit Fork

; For bootloader mode (initial flash)
upload_protocol = picotool

build_flags = 
	-D USE_TINYUSB
	-D NO_SDCARD
	-D ADAFRUIT_PROTOMATTER_NO_SDCARD
	-D ADAFRUIT_NEOPIXEL_SUPPORT_ONLY
	-D PANEL_COUNT=4
```

4. **Connect Your Pico**
   - For first-time programming:
     - Press and hold BOOTSEL button while connecting USB
     - Release after connecting
   - For subsequent uploads:
     - Just connect via USB (no BOOTSEL needed)

## Part 2: Custom Panel Arrangement Configuration

### 2.1 Understand Panel Chaining Options

RGB LED panels can be arranged in different configurations:

1. **Linear Chain** (simplest):
   ```
   [P1] → [P2] → [P3] → [P4]
   ```

2. **Serpentine** (zigzag pattern):
   ```
   [P1] → [P2]
    ↑     ↓
   [P4] ← [P3]
   ```

3. **Custom Arrangements**:
   - You'll need to map coordinates in software

### 2.2 Define Your Custom Arrangement

Please sketch your custom arrangement indicating:
- Panel order (1-4)
- Data flow direction (which panel connects to which)
- Physical orientation (any rotated panels?)

Then we can create the appropriate mapping code.

### 2.3 Update Matrix Controller Code

Create a new file `CustomMatrixController.h` with mapping functions for your arrangement:

```cpp
#ifndef CUSTOM_MATRIX_CONTROLLER_H
#define CUSTOM_MATRIX_CONTROLLER_H

#include "MatrixController.h"

class CustomMatrixController : public MatrixController {
public:
    // Inherit constructor from parent
    using MatrixController::MatrixController;
    
    // Override drawPixel to handle custom mapping
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        // Map the coordinates based on your custom arrangement
        int16_t mapped_x, mapped_y;
        mapCoordinates(x, y, &mapped_x, &mapped_y);
        
        // Call parent implementation with mapped coordinates
        MatrixController::drawPixel(mapped_x, mapped_y, color);
    }
    
private:
    // Implement based on your specific arrangement
    void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
        // Panel size
        const int16_t PANEL_W = 64;
        const int16_t PANEL_H = 64;
        
        // Default pass-through implementation
        // TODO: Modify based on your specific arrangement
        *mapped_x = x;
        *mapped_y = y;
    }
};

#endif
```

## Part 3: Testing Your Panel Configuration

### 3.1 Initial Test Code

Create a new `test_pattern.cpp` file:

```cpp
#include <Arduino.h>
#include "CustomMatrixController.h"

// Pin definitions
#define R1_PIN 2
#define G1_PIN 3
#define B1_PIN 4
#define R2_PIN 5
#define G2_PIN 8
#define B2_PIN 9

#define A_PIN 10
#define B_PIN 16
#define C_PIN 18
#define D_PIN 20
#define E_PIN 22

#define CLK_PIN 11
#define LAT_PIN 12
#define OE_PIN 13

// RGB and Address pins
uint8_t rgbPins[] = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN};
uint8_t addrPins[] = {A_PIN, B_PIN, C_PIN, D_PIN, E_PIN};

// Panel configuration
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANEL_COUNT 4
#define TOTAL_WIDTH (PANEL_WIDTH * 2)  // Assuming 2x2 arrangement
#define TOTAL_HEIGHT (PANEL_HEIGHT * 2)

// Create matrix controller with your custom arrangement
CustomMatrixController matrix(
  rgbPins,
  addrPins,
  CLK_PIN, LAT_PIN, OE_PIN,
  PANEL_WIDTH,
  PANEL_HEIGHT,
  PANEL_COUNT,
  true,  // Double buffering
  0      // No tiling - we'll handle mapping ourselves
);

void setup() {
  Serial1.begin(115200);
  Serial1.println("RGB Matrix Test Pattern");
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // LED on during setup
  
  // Initialize panels with special sequence
  // Reginit(); // Uncomment if needed
  
  if (!matrix.begin()) {
    Serial1.println("Matrix initialization failed!");
    // Error blink pattern
    while (1) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }
  }
  
  Serial1.println("Matrix initialized successfully!");
  digitalWrite(LED_BUILTIN, LOW);  // LED off on success
}

void testPanelIdentification() {
  // Fill each panel with a different color to identify
  matrix.fillRect(0, 0, PANEL_WIDTH, PANEL_HEIGHT, matrix.color565(255, 0, 0));          // Panel 1: Red
  matrix.fillRect(PANEL_WIDTH, 0, PANEL_WIDTH, PANEL_HEIGHT, matrix.color565(0, 255, 0));  // Panel 2: Green
  matrix.fillRect(0, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT, matrix.color565(0, 0, 255));  // Panel 3: Blue
  matrix.fillRect(PANEL_WIDTH, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT, matrix.color565(255, 255, 0));  // Panel 4: Yellow
  matrix.show();
  delay(5000);
}

void testGrid() {
  // Draw grid lines to verify alignment
  matrix.fillScreen(0);
  
  // Vertical lines
  for (int x = 0; x < TOTAL_WIDTH; x += 8) {
    for (int y = 0; y < TOTAL_HEIGHT; y++) {
      matrix.drawPixel(x, y, matrix.color565(100, 100, 100));
    }
  }
  
  // Horizontal lines
  for (int y = 0; y < TOTAL_HEIGHT; y += 8) {
    for (int x = 0; x < TOTAL_WIDTH; x++) {
      matrix.drawPixel(x, y, matrix.color565(100, 100, 100));
    }
  }
  
  matrix.show();
  delay(5000);
}

void testCrossPanelLine() {
  // Draw lines that cross panel boundaries
  matrix.fillScreen(0);
  
  // Diagonal line across entire display
  for (int i = 0; i < TOTAL_WIDTH; i++) {
    int y = i * TOTAL_HEIGHT / TOTAL_WIDTH;
    matrix.drawPixel(i, y, matrix.color565(255, 255, 255));
  }
  
  // Cross in the center
  matrix.drawLine(0, TOTAL_HEIGHT/2, TOTAL_WIDTH, TOTAL_HEIGHT/2, matrix.color565(255, 0, 0));
  matrix.drawLine(TOTAL_WIDTH/2, 0, TOTAL_WIDTH/2, TOTAL_HEIGHT, matrix.color565(0, 255, 0));
  
  matrix.show();
  delay(5000);
}

void loop() {
  // Run test patterns in sequence
  testPanelIdentification();
  testGrid();
  testCrossPanelLine();
  
  // Cycle through basic colors
  matrix.fillScreen(matrix.color565(255, 0, 0));  // Red
  matrix.show();
  delay(1000);
  
  matrix.fillScreen(matrix.color565(0, 255, 0));  // Green
  matrix.show();
  delay(1000);
  
  matrix.fillScreen(matrix.color565(0, 0, 255));  // Blue
  matrix.show();
  delay(1000);
}
```

### 3.2 Testing Methodology

1. **Identify Panel Order**
   - Run the panel identification test
   - Note which color appears on which physical panel
   - This helps determine the wiring order

2. **Test Coordinate System**
   - The grid test will show how coordinates map across panels
   - Look for discontinuities or mirroring

3. **Cross-Panel Continuity**
   - The line test will show if drawings can cross panel boundaries
   - Check if lines appear continuous across panels

4. **Adjust Mapping as Needed**
   - Based on test results, update the `mapCoordinates()` function
   - Rerun tests until display works as expected

## Part 4: Common Panel Arrangements

Here are example mapping functions for common arrangements:

### 4.1 Simple 2×2 Grid
```cpp
void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
    // No mapping needed - standard layout
    *mapped_x = x;
    *mapped_y = y;
}
```

### 4.2 Serpentine 2×2 Grid
```cpp
void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
    const int16_t PANEL_W = 64;
    const int16_t PANEL_H = 64;
    
    // Determine which panel the coordinates fall in
    int panel_x = x / PANEL_W;
    int panel_y = y / PANEL_H;
    
    // Local coordinates within the panel
    int local_x = x % PANEL_W;
    int local_y = y % PANEL_H;
    
    // In a serpentine layout, every other row is reversed
    if (panel_y % 2 == 1) {
        // For odd rows, reverse the x order
        panel_x = 1 - panel_x;
        // May also need to flip coordinates within the panel
        local_x = PANEL_W - 1 - local_x;
    }
    
    // Recombine to get final coordinates
    *mapped_x = panel_x * PANEL_W + local_x;
    *mapped_y = panel_y * PANEL_H + local_y;
}
```

### 4.3 Custom "L" Shape (3-up, 1-right)
```cpp
void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
    const int16_t PANEL_W = 64;
    const int16_t PANEL_H = 64;
    
    // Determine logical panel position
    int panel_num;
    if (x < PANEL_W) {
        // Left column
        panel_num = y / PANEL_H;  // 0, 1, or 2 from top to bottom
    } else {
        // Right column (only top position)
        panel_num = 3;
    }
    
    // Map to physical panel and get local coordinates
    int local_x, local_y;
    
    switch (panel_num) {
        case 0: // Top-left panel
            local_x = x;
            local_y = y;
            break;
        case 1: // Middle-left panel
            local_x = x;
            local_y = y - PANEL_H;
            break;
        case 2: // Bottom-left panel
            local_x = x;
            local_y = y - 2*PANEL_H;
            break;
        case 3: // Top-right panel
            local_x = x - PANEL_W;
            local_y = y;
            break;
    }
    
    // Add mapping based on physical wiring here
    // This example assumes standard wiring
    *mapped_x = local_x;
    *mapped_y = local_y;
}
```

Once you provide details about your specific arrangement, we can create a custom mapping function tailored to your setup.