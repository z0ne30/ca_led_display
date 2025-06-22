# RGB Matrix Test Code

Below is a complete test program you can use to test your 4-panel RGB LED matrix setup. Copy this code to your `main.cpp` file in your PlatformIO project.

```cpp
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Protomatter.h>

// RGB Matrix pinout for Raspberry Pi Pico
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

// Globally-defined pin arrays
uint8_t rgbPins[] = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN};
uint8_t addrPins[] = {A_PIN, B_PIN, C_PIN, D_PIN, E_PIN};

// Matrix configuration
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANEL_COUNT 4
#define TOTAL_WIDTH (PANEL_WIDTH * 2)   // Assuming 2x2 grid - adjust for your layout
#define TOTAL_HEIGHT (PANEL_HEIGHT * 2) // Assuming 2x2 grid - adjust for your layout

// Custom matrix mapper class for custom panel arrangements
class CustomMatrixMapper {
public:
  // Map logical coordinates to physical coordinates based on panel arrangement
  static void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
    const int16_t PANEL_W = PANEL_WIDTH;
    const int16_t PANEL_H = PANEL_HEIGHT;
    
    // Determine which logical panel the coordinates are in
    // This assumes a 2x2 arrangement - modify for your specific arrangement
    int panel_x = x / PANEL_W;
    int panel_y = y / PANEL_H;
    
    // Local coordinates within the panel
    int local_x = x % PANEL_W;
    int local_y = y % PANEL_H;
    
    // Get the logical panel number (0-3)
    int logical_panel = panel_y * 2 + panel_x;
    
    // Map logical panels to physical panels based on your arrangement
    // CUSTOMIZE THIS PART based on your observations
    int physical_panel = logical_panel;  // Default mapping (no rearrangement)
    int orientation = 0;  // 0=normal, 1=90° CW, 2=180°, 3=270° CW
    
    // Example: If your panels are in a different order or rotated
    // Uncomment and modify this section based on your panel arrangement
    /*
    switch (logical_panel) {
      case 0:  // Top-left logical position
        physical_panel = 0;  // Maps to first panel in chain
        orientation = 0;     // Normal orientation
        break;
      case 1:  // Top-right logical position
        physical_panel = 1;  // Maps to second panel in chain
        orientation = 0;     // Normal orientation
        break;
      case 2:  // Bottom-left logical position
        physical_panel = 2;  // Maps to third panel in chain
        orientation = 0;     // Normal orientation
        break;
      case 3:  // Bottom-right logical position
        physical_panel = 3;  // Maps to fourth panel in chain
        orientation = 0;     // Normal orientation
        break;
    }
    */
    
    // Apply rotation based on orientation
    int rotated_x = local_x;
    int rotated_y = local_y;
    
    switch (orientation) {
      case 0:  // Normal
        rotated_x = local_x;
        rotated_y = local_y;
        break;
      case 1:  // 90° clockwise
        rotated_x = PANEL_W - 1 - local_y;
        rotated_y = local_x;
        break;
      case 2:  // 180°
        rotated_x = PANEL_W - 1 - local_x;
        rotated_y = PANEL_H - 1 - local_y;
        break;
      case 3:  // 270° clockwise
        rotated_x = local_y;
        rotated_y = PANEL_H - 1 - local_x;
        break;
    }
    
    // Calculate physical panel coordinates
    // This assumes a 2x2 arrangement - modify for your specific arrangement
    int physical_x = (physical_panel % 2) * PANEL_W;
    int physical_y = (physical_panel / 2) * PANEL_H;
    
    // Final coordinates
    *mapped_x = physical_x + rotated_x;
    *mapped_y = physical_y + rotated_y;
  }
};

// Wrapper class for Adafruit_Protomatter that uses our custom mapper
class CustomMappedMatrix {
private:
  Adafruit_Protomatter* matrix;
  
public:
  // Common colors
  static const uint16_t BLACK = 0x0000;
  static const uint16_t WHITE = 0xFFFF;
  static const uint16_t RED = 0xF800;
  static const uint16_t GREEN = 0x07E0;
  static const uint16_t BLUE = 0x001F;
  static const uint16_t YELLOW = 0xFFE0;
  static const uint16_t CYAN = 0x07FF;
  static const uint16_t MAGENTA = 0xF81F;
  
  // Constructor
  CustomMappedMatrix(uint8_t rgbPins[], uint8_t addrPins[], 
                   uint8_t clockPin, uint8_t latchPin, uint8_t oePin) {
    matrix = new Adafruit_Protomatter(
      PANEL_WIDTH,   // Width of EACH PANEL
      6,             // Bit depth (4 = 16 shades of each R/G/B)
      1, rgbPins,    // Data pin count, RGB pins
      5, addrPins,   // Address pin count, address pins
      clockPin, latchPin, oePin,  // Clock, latch, output enable pins
      true,          // Double-buffering
      0              // No tiling (custom mapping handled by our code)
    );
  }
  
  // Initialize the matrix
  bool begin() {
    return matrix->begin() == PROTOMATTER_OK;
  }
  
  // Clear the screen
  void fillScreen(uint16_t color) {
    matrix->fillScreen(color);
  }
  
  // Update the display
  void show() {
    matrix->show();
  }
  
  // Draw a pixel with custom coordinate mapping
  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= TOTAL_WIDTH || y < 0 || y >= TOTAL_HEIGHT) return;
    
    int16_t mapped_x, mapped_y;
    CustomMatrixMapper::mapCoordinates(x, y, &mapped_x, &mapped_y);
    matrix->drawPixel(mapped_x, mapped_y, color);
  }
  
  // Draw a line
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
      _swap_int16_t(x0, y0);
      _swap_int16_t(x1, y1);
    }
    
    if (x0 > x1) {
      _swap_int16_t(x0, x1);
      _swap_int16_t(y0, y1);
    }
    
    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;
    
    for (; x0 <= x1; x0++) {
      if (steep) {
        drawPixel(y0, x0, color);
      } else {
        drawPixel(x0, y0, color);
      }
      err -= dy;
      if (err < 0) {
        y0 += ystep;
        err += dx;
      }
    }
  }
  
  // Draw a rectangle
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawLine(x, y, x+w-1, y, color);
    drawLine(x, y+h-1, x+w-1, y+h-1, color);
    drawLine(x, y, x, y+h-1, color);
    drawLine(x+w-1, y, x+w-1, y+h-1, color);
  }
  
  // Fill a rectangle
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    for (int16_t i=x; i<x+w; i++) {
      for (int16_t j=y; j<y+h; j++) {
        drawPixel(i, j, color);
      }
    }
  }
  
  // Set text position
  void setCursor(int16_t x, int16_t y) {
    matrix->setCursor(x, y);
  }
  
  // Set text color
  void setTextColor(uint16_t color) {
    matrix->setTextColor(color);
  }
  
  // Set text size
  void setTextSize(uint8_t size) {
    matrix->setTextSize(size);
  }
  
  // Print text (using Adafruit_GFX's print functions)
  size_t print(const char* text) {
    return matrix->print(text);
  }
  
  // Create a color from RGB values
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return matrix->color565(r, g, b);
  }
  
  // Helper function to swap values
  void _swap_int16_t(int16_t& a, int16_t& b) {
    int16_t t = a;
    a = b;
    b = t;
  }
};

// Create our matrix instance
CustomMappedMatrix matrix(rgbPins, addrPins, CLK_PIN, LAT_PIN, OE_PIN);

// For FPS calculation
unsigned long frameCount = 0;
unsigned long lastFPSCheck = 0;
float fps = 0;

// Hardware initialization for matrix panels
void Reginit() {
  pinMode(R1_PIN, OUTPUT);
  pinMode(G1_PIN, OUTPUT);
  pinMode(B1_PIN, OUTPUT);
  pinMode(R2_PIN, OUTPUT);
  pinMode(G2_PIN, OUTPUT);
  pinMode(B2_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);
  pinMode(LAT_PIN, OUTPUT);

  digitalWrite(OE_PIN, HIGH);
  digitalWrite(LAT_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);
  int MaxLed = 64;

  int C12[16] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int C13[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0};

  for (int l = 0; l < MaxLed; l++) {
    int y = l % 16;
    digitalWrite(R1_PIN, LOW);
    digitalWrite(G1_PIN, LOW);
    digitalWrite(B1_PIN, LOW);
    digitalWrite(R2_PIN, LOW);
    digitalWrite(G2_PIN, LOW);
    digitalWrite(B2_PIN, LOW);
    if (C12[y] == 1) {
      digitalWrite(R1_PIN, HIGH);
      digitalWrite(G1_PIN, HIGH);
      digitalWrite(B1_PIN, HIGH);
      digitalWrite(R2_PIN, HIGH);
      digitalWrite(G2_PIN, HIGH);
      digitalWrite(B2_PIN, HIGH);
    }
    if (l > MaxLed - 12) {
      digitalWrite(LAT_PIN, HIGH);
    } else {
      digitalWrite(LAT_PIN, LOW);
    }
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(CLK_PIN, LOW);
  }
  digitalWrite(LAT_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);

  // Send Data to control register 12
  for (int l = 0; l < MaxLed; l++) {
    int y = l % 16;
    digitalWrite(R1_PIN, LOW);
    digitalWrite(G1_PIN, LOW);
    digitalWrite(B1_PIN, LOW);
    digitalWrite(R2_PIN, LOW);
    digitalWrite(G2_PIN, LOW);
    digitalWrite(B2_PIN, LOW);
    if (C13[y] == 1) {
      digitalWrite(R1_PIN, HIGH);
      digitalWrite(G1_PIN, HIGH);
      digitalWrite(B1_PIN, HIGH);
      digitalWrite(R2_PIN, HIGH);
      digitalWrite(G2_PIN, HIGH);
      digitalWrite(B2_PIN, HIGH);
    }
    if (l > MaxLed - 13) {
      digitalWrite(LAT_PIN, HIGH);
    } else {
      digitalWrite(LAT_PIN, LOW);
    }
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(CLK_PIN, LOW);
  }
  digitalWrite(LAT_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);
}

// Test patterns
void testPanelIdentification() {
  matrix.fillScreen(0);
  
  // Draw panel numbers on each panel (assumes 2x2 arrangement - adjust for your layout)
  // Panel 1 (top-left) - Red
  matrix.fillRect(0, 0, PANEL_WIDTH, PANEL_HEIGHT, matrix.RED);
  
  // Panel 2 (top-right) - Green
  matrix.fillRect(PANEL_WIDTH, 0, PANEL_WIDTH, PANEL_HEIGHT, matrix.GREEN);
  
  // Panel 3 (bottom-left) - Blue
  matrix.fillRect(0, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT, matrix.BLUE);
  
  // Panel 4 (bottom-right) - Yellow
  matrix.fillRect(PANEL_WIDTH, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT, matrix.YELLOW);
  
  matrix.show();
  delay(5000);
}

void testGrid() {
  matrix.fillScreen(0);
  
  // Draw grid lines
  for (int x = 0; x < TOTAL_WIDTH; x += 8) {
    for (int y = 0; y < TOTAL_HEIGHT; y++) {
      matrix.drawPixel(x, y, matrix.color565(64, 64, 64));
    }
  }
  
  for (int y = 0; y < TOTAL_HEIGHT; y += 8) {
    for (int x = 0; x < TOTAL_WIDTH; x++) {
      matrix.drawPixel(x, y, matrix.color565(64, 64, 64));
    }
  }
  
  matrix.show();
  delay(5000);
}

void testCrossPanelLine() {
  matrix.fillScreen(0);
  
  // Draw diagonal line across entire display
  matrix.drawLine(0, 0, TOTAL_WIDTH-1, TOTAL_HEIGHT-1, matrix.WHITE);
  
  // Draw cross in the center
  matrix.drawLine(0, TOTAL_HEIGHT/2, TOTAL_WIDTH-1, TOTAL_HEIGHT/2, matrix.RED);
  matrix.drawLine(TOTAL_WIDTH/2, 0, TOTAL_WIDTH/2, TOTAL_HEIGHT-1, matrix.GREEN);
  
  matrix.show();
  delay(5000);
}

void testText() {
  matrix.fillScreen(0);
  
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.WHITE);
  
  // Position text to span across panels
  matrix.setCursor(TOTAL_WIDTH/2 - 30, TOTAL_HEIGHT/2 - 4);
  matrix.print("RGB MATRIX");
  
  matrix.show();
  delay(5000);
}

void testColors() {
  // Cycle through colors
  uint16_t colors[] = {
    matrix.RED,
    matrix.GREEN, 
    matrix.BLUE,
    matrix.YELLOW,
    matrix.CYAN,
    matrix.MAGENTA,
    matrix.WHITE
  };
  
  for (int i = 0; i < 7; i++) {
    matrix.fillScreen(colors[i]);
    matrix.show();
    delay(1000);
  }
}

void setup() {
  Serial1.begin(115200);
  Serial1.println("RGB Matrix Test");
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // LED on during setup
  
  Reginit(); // Special initialization sequence
  
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
  
  // Reset FPS counter
  frameCount = 0;
  lastFPSCheck = millis();
}

void loop() {
  // Run all test patterns in sequence
  testPanelIdentification();
  testGrid();
  testCrossPanelLine();
  testText();
  testColors();
  
  // Update FPS count
  frameCount++;
  if (millis() - lastFPSCheck >= 1000) {
    fps = frameCount;
    frameCount = 0;
    lastFPSCheck = millis();
    
    Serial1.print("FPS: ");
    Serial1.println(fps);
  }
}
```

## How to Use This Code

1. Copy the entire code above to your `main.cpp` file in your PlatformIO project
2. Adjust the `TOTAL_WIDTH` and `TOTAL_HEIGHT` constants based on your panel arrangement
3. Modify the `mapCoordinates` method in the `CustomMatrixMapper` class to match your specific panel arrangement
4. Upload the code to your Raspberry Pi Pico
5. Observe the test patterns on your display

The code includes several test patterns that will help you verify your panel configuration:

1. **Panel Identification Test**: Colors each panel differently to identify panel order
2. **Grid Test**: Draws a grid to verify coordinate mapping
3. **Cross-Panel Line Test**: Draws lines across panel boundaries to verify continuity
4. **Text Test**: Displays text that spans across panels
5. **Color Test**: Cycles through basic colors to verify color reproduction

If any test doesn't display correctly, adjust the mapping function to match your specific panel arrangement.