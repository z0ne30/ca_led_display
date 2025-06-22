# Performance Testing for Multi-Panel RGB Matrix Display

This guide will help you test and optimize the performance of your 4-panel RGB LED matrix setup. These tests will be particularly useful when you have your panels connected and want to maximize refresh rates and visual quality.

## 1. Measuring Performance

### 1.1 FPS Measurement

Add this code to your main loop to measure frames-per-second:

```cpp
// In global scope
unsigned long frameCount = 0;
unsigned long lastFPSCheck = 0;
float fps = 0;

// In loop()
void loop() {
  // Start timing
  unsigned long startTime = micros();
  
  // Your drawing code here
  // ...
  
  // Show the frame
  matrix.show();
  
  // Count frames
  frameCount++;
  
  // Calculate FPS every second
  if (millis() - lastFPSCheck >= 1000) {
    fps = frameCount;
    frameCount = 0;
    lastFPSCheck = millis();
    
    // Print FPS
    Serial1.print("FPS: ");
    Serial1.println(fps);
  }
  
  // Optionally measure individual frame time
  unsigned long frameTime = micros() - startTime;
  Serial1.print("Frame time (us): ");
  Serial1.println(frameTime);
}
```

### 1.2 Memory Usage

Monitor available RAM to avoid crashes:

```cpp
void printMemoryUsage() {
  // Get available RAM (this method is specific to RP2040)
  extern char *__sbrk(int incr);
  char top;
  Serial1.print("Free RAM: ");
  Serial1.print(&top - __sbrk(0));
  Serial1.println(" bytes");
}
```

## 2. Performance Optimization Techniques

### 2.1 Bit Depth Adjustment

Bit depth significantly affects refresh rate. Lower bit depth means faster updates but fewer colors:

```cpp
// High bit depth (more colors, slower refresh)
Adafruit_Protomatter matrix(
  PANEL_WIDTH, 6, /* 6-bit color = 262,144 colors */
  ...
);

// Medium bit depth (good balance)
Adafruit_Protomatter matrix(
  PANEL_WIDTH, 4, /* 4-bit color = 4,096 colors */
  ...
);

// Low bit depth (fewer colors, faster refresh)
Adafruit_Protomatter matrix(
  PANEL_WIDTH, 3, /* 3-bit color = 512 colors */
  ...
);
```

Test different bit depths to find the best balance for your application.

### 2.2 Buffer Management

Double buffering is essential for smooth animations but uses more RAM:

```cpp
// With double buffering (smoother animations)
Adafruit_Protomatter matrix(
  ..., true, ...
);

// Without double buffering (less RAM usage)
Adafruit_Protomatter matrix(
  ..., false, ...
);
```

### 2.3 Partial Updates

For better performance with animations, only update changed regions:

```cpp
// Create a tracking array to detect changes
bool pixelChanged[TOTAL_WIDTH][TOTAL_HEIGHT] = {false};

// When drawing, mark changed pixels
void customDrawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x >= 0 && x < TOTAL_WIDTH && y >= 0 && y < TOTAL_HEIGHT) {
    matrix.drawPixel(x, y, color);
    pixelChanged[x][y] = true;
  }
}

// Reset change tracking after update
void showAndReset() {
  matrix.show();
  
  // Reset change tracking
  for (int x = 0; x < TOTAL_WIDTH; x++) {
    for (int y = 0; y < TOTAL_HEIGHT; y++) {
      pixelChanged[x][y] = false;
    }
  }
}
```

## 3. Performance Test Patterns

### 3.1 Full Screen Update Test

Tests raw refresh capability:

```cpp
void testFullScreenRefresh() {
  uint16_t colors[] = {
    matrix.color565(255, 0, 0),    // Red
    matrix.color565(0, 255, 0),    // Green
    matrix.color565(0, 0, 255),    // Blue
    matrix.color565(255, 255, 255) // White
  };
  
  unsigned long startTime = millis();
  int frames = 0;
  
  // Run test for 5 seconds
  while (millis() - startTime < 5000) {
    // Fill screen with alternating colors
    matrix.fillScreen(colors[frames % 4]);
    matrix.show();
    frames++;
  }
  
  float fps = frames / 5.0;
  Serial1.print("Full screen refresh rate: ");
  Serial1.print(fps);
  Serial1.println(" FPS");
}
```

### 3.2 Scrolling Text Test

Tests performance with realistic content:

```cpp
void testScrollingText() {
  matrix.fillScreen(0);
  
  int16_t textWidth = 6 * 16; // Approximate width of text
  int16_t x = TOTAL_WIDTH;
  
  unsigned long startTime = millis();
  int frames = 0;
  
  // Scroll for 5 seconds
  while (millis() - startTime < 5000) {
    matrix.fillScreen(0);
    matrix.setCursor(x, TOTAL_HEIGHT/2 - 4);
    matrix.setTextColor(matrix.color565(255, 255, 0));
    matrix.print("SCROLL TEST");
    matrix.show();
    
    x--;
    if (x < -textWidth) x = TOTAL_WIDTH;
    frames++;
  }
  
  float fps = frames / 5.0;
  Serial1.print("Scrolling text refresh rate: ");
  Serial1.print(fps);
  Serial1.println(" FPS");
}
```

### 3.3 Pixel-by-Pixel Animation Test

Tests performance with many small updates:

```cpp
void testPixelAnimation() {
  const int NUM_DOTS = 100;
  int16_t x[NUM_DOTS], y[NUM_DOTS];
  int8_t vx[NUM_DOTS], vy[NUM_DOTS];
  uint16_t color[NUM_DOTS];
  
  // Initialize random dots
  for (int i = 0; i < NUM_DOTS; i++) {
    x[i] = random(TOTAL_WIDTH);
    y[i] = random(TOTAL_HEIGHT);
    vx[i] = random(1, 4) * (random(2) ? 1 : -1);
    vy[i] = random(1, 4) * (random(2) ? 1 : -1);
    color[i] = matrix.color565(random(256), random(256), random(256));
  }
  
  unsigned long startTime = millis();
  int frames = 0;
  
  // Run test for 5 seconds
  while (millis() - startTime < 5000) {
    matrix.fillScreen(0);
    
    // Update and draw dots
    for (int i = 0; i < NUM_DOTS; i++) {
      x[i] += vx[i];
      y[i] += vy[i];
      
      // Bounce off edges
      if (x[i] < 0 || x[i] >= TOTAL_WIDTH) vx[i] = -vx[i];
      if (y[i] < 0 || y[i] >= TOTAL_HEIGHT) vy[i] = -vy[i];
      
      matrix.drawPixel(x[i], y[i], color[i]);
    }
    
    matrix.show();
    frames++;
  }
  
  float fps = frames / 5.0;
  Serial1.print("Pixel animation refresh rate: ");
  Serial1.print(fps);
  Serial1.println(" FPS");
}
```

## 4. Troubleshooting Performance Issues

### 4.1 Power-Related Issues

Insufficient power causes display glitches and reduced brightness:

1. **Symptoms**:
   - Flickering or dim panels
   - Random pixels or colors
   - Display corruption when showing bright colors

2. **Solutions**:
   - Use a high-quality 5V power supply rated for at least 4A per panel
   - Use thick power wires to minimize voltage drop
   - Add power injection wires for each panel
   - Reduce maximum brightness in software

### 4.2 Data Signal Issues

Poor signal integrity causes display glitches:

1. **Symptoms**:
   - Panel data corruption
   - "Ghosting" effects
   - Wrong colors or patterns

2. **Solutions**:
   - Keep data cables short
   - Use quality cables
   - Reduce clock speed if needed:

```cpp
// If you see data corruption, you can try reducing the clock speed
// by adding this parameter when creating the matrix:
Adafruit_Protomatter matrix(
  PANEL_WIDTH, 4, 1, rgbPins, 5, addrPins,
  CLK_PIN, LAT_PIN, OE_PIN, true, 0,
  /* Lower clock speed: */ 5000000  // Default is 12MHz
);
```

### 4.3 Code Optimization

1. **Minimize `show()` calls** - Each call updates the entire display
2. **Reduce drawing operations** - Complex shapes are expensive
3. **Use hardware acceleration** when available
4. **Precompute values** rather than calculating on-the-fly
5. **Batch drawing operations** to minimize pixel changes

## 5. Advanced Applications

### 5.1 Video Playback

For simple video playback, you'll need to:
1. Store frames in optimized format
2. Pre-process to match bit depth
3. Stream from SD card or Flash memory

### 5.2 Dynamic Content

For social media feeds, weather, or other dynamic content:
1. Use a second core for data fetching
2. Buffer content to avoid display glitches
3. Update only changed portions of screen

### 5.3 Interactive Applications

For games or touch interfaces:
1. Prioritize input handling
2. Use simple graphics for higher frame rates
3. Implement efficient collision detection  int frames = 0;
  
  // Run test for 5 seconds
  while (millis() - startTime < 5000) {
    matrix.fillScreen(0);
    
    // Update and draw dots
    for (int i = 0; i < NUM_DOTS; i++) {
      x[i] += vx[i];
      y[i] += vy[i];
      
      // Bounce off edges
      if (x[i] < 0 || x[i] >= TOTAL_WIDTH) vx[i] = -vx[i];
      if (y[i] < 0 || y[i] >= TOTAL_HEIGHT) vy[i] = -vy[i];
      
      matrix.drawPixel(x[i], y[i], color[i]);
    }
    
    matrix.show();
    frames++;
  }
  
  float fps = frames / 5.0;
  Serial1.print("Pixel animation refresh rate: ");
  Serial1.print(fps);
  Serial1.println(" FPS");
}
```

## 4. Troubleshooting Performance Issues

### 4.1 Power-Related Issues

Insufficient power causes display glitches and reduced brightness:

1. **Symptoms**:
   - Flickering or dim panels
   - Random pixels or colors
   - Display corruption when showing bright colors

2. **Solutions**:
   - Use a high-quality 5V power supply rated for at least 4A per panel
   - Use thick power wires to minimize voltage drop
   - Add power injection wires for each panel
   - Reduce maximum brightness in software

### 4.2 Data Signal Issues

Poor signal integrity causes display glitches:

1. **Symptoms**:
   - Panel data corruption
   - "Ghosting" effects
   - Wrong colors or patterns

2. **Solutions**:
   - Keep data cables short
   - Use quality cables
   - Reduce clock speed if needed:

```cpp
// If you see data corruption, you can try reducing the clock speed
// by adding this parameter when creating the matrix:
Adafruit_Protomatter matrix(
  PANEL_WIDTH, 4, 1, rgbPins, 5, addrPins,
  CLK_PIN, LAT_PIN, OE_PIN, true, 0,
  /* Lower clock speed: */ 5000000  // Default is 12MHz
);
```

### 4.3 Code Optimization

1. **Minimize `show()` calls** - Each call updates the entire display
2. **Reduce drawing operations** - Complex shapes are expensive
3. **Use hardware acceleration** when available
4. **Precompute values** rather than calculating on-the-fly
5. **Batch drawing operations** to minimize pixel changes

## 5. Advanced Applications

### 5.1 Video Playback

For simple video playback, you'll need to:
1. Store frames in optimized format
2. Pre-process to match bit depth
3. Stream from SD card or Flash memory

### 5.2 Dynamic Content

For social media feeds, weather, or other dynamic content:
1. Use a second core for data fetching
2. Buffer content to avoid display glitches
3. Update only changed portions of screen

### 5.3 Interactive Applications

For games or touch interfaces:
1. Prioritize input handling
2. Use simple graphics for higher frame rates
3. Implement efficient collision detection
