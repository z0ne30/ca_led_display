# Custom Panel Mapping Guide

This guide will help you determine the correct mapping for your custom panel arrangement and implement it in code.

## 1. Identifying Your Panel Configuration

### Step 1: Label Your Panels

First, number your panels from 1 to 4 based on their data connection order:

- Panel 1: First panel (connected directly to the Pico)
- Panel 2: Connected to the output of Panel 1
- Panel 3: Connected to the output of Panel 2
- Panel 4: Connected to the output of Panel 3

### Step 2: Run the Panel Identification Test

Use this code to identify how your panels are physically arranged:

```cpp
void identifyPanels() {
  // Clear the display
  matrix.fillScreen(0);
  
  // Draw panel numbers on each panel
  // Adjust these coordinates based on your actual panel arrangement
  drawPanelNumber(0, 0, 1);          // Panel 1 (top-left in a 2x2 grid)
  drawPanelNumber(PANEL_WIDTH, 0, 2); // Panel 2 (top-right in a 2x2 grid)
  drawPanelNumber(0, PANEL_HEIGHT, 3); // Panel 3 (bottom-left in a 2x2 grid)
  drawPanelNumber(PANEL_WIDTH, PANEL_HEIGHT, 4); // Panel 4 (bottom-right in a 2x2 grid)
  
  matrix.show();
  delay(10000); // Display for 10 seconds
}

void drawPanelNumber(int x, int y, int number) {
  // Choose a color for each panel number
  uint16_t color;
  switch(number) {
    case 1: color = matrix.color565(255, 0, 0); break;    // Red
    case 2: color = matrix.color565(0, 255, 0); break;    // Green
    case 3: color = matrix.color565(0, 0, 255); break;    // Blue
    case 4: color = matrix.color565(255, 255, 0); break;  // Yellow
    default: color = matrix.color565(255, 255, 255);      // White
  }
  
  // Draw panel number
  matrix.setCursor(x + PANEL_WIDTH/2 - 8, y + PANEL_HEIGHT/2 - 8);
  matrix.setTextColor(color);
  matrix.setTextSize(2);
  matrix.print(number);
  
  // Draw border around panel
  matrix.drawRect(x, y, PANEL_WIDTH, PANEL_HEIGHT, color);
}
```

### Step 3: Map Your Physical Layout

Based on where the numbers appear on your physical display, sketch your panel arrangement:

```
Example layout (numbers indicate panel number):

┌─────┬─────┐
│     │     │
│  1  │  2  │
│     │     │
├─────┼─────┤
│     │     │
│  3  │  4  │
│     │     │
└─────┴─────┘
```

## 2. Determining Panel Orientations

Some panels might be rotated or flipped based on how they're mounted. To determine this:

### Step 1: Run the Orientation Test

```cpp
void testPanelOrientations() {
  matrix.fillScreen(0);
  
  // Draw arrows pointing to top-right on each panel
  drawArrow(0, 0, PANEL_WIDTH, PANEL_HEIGHT);
  drawArrow(PANEL_WIDTH, 0, PANEL_WIDTH, PANEL_HEIGHT);
  drawArrow(0, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT);
  drawArrow(PANEL_WIDTH, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT);
  
  matrix.show();
  delay(10000);
}

void drawArrow(int panel_x, int panel_y, int panel_w, int panel_h) {
  int center_x = panel_x + panel_w/2;
  int center_y = panel_y + panel_h/2;
  
  // Draw arrow pointing to top-right
  matrix.drawLine(center_x, center_y, center_x + 20, center_y - 20, matrix.WHITE);
  matrix.drawLine(center_x + 20, center_y - 20, center_x + 10, center_y - 20, matrix.WHITE);
  matrix.drawLine(center_x + 20, center_y - 20, center_x + 20, center_y - 10, matrix.WHITE);
}
```

### Step 2: Note Any Rotations

For each panel, note if the arrow is pointing:
- Top-right (normal orientation)
- Top-left (90° clockwise)
- Bottom-left (180° rotation)
- Bottom-right (90° counter-clockwise)

## 3. Creating Your Custom Mapping Function

Based on your observations, create a mapping function. Here's a template:

```cpp
void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
  const int16_t PANEL_W = 64;
  const int16_t PANEL_H = 64;
  
  // Determine which logical panel the coordinates are in
  int panel_x = x / PANEL_W;
  int panel_y = y / PANEL_H;
  
  // Get the logical panel number (0-3)
  int logical_panel = panel_y * 2 + panel_x;
  
  // Local coordinates within the panel
  int local_x = x % PANEL_W;
  int local_y = y % PANEL_H;
  
  // Physical panel index and orientation based on your arrangement
  int physical_panel = logical_panel;  // Default: no remapping
  int orientation = 0;  // 0=normal, 1=90° CW, 2=180°, 3=270° CW
  
  // Map logical panels to physical panels based on your arrangement
  // CUSTOMIZE THIS PART based on your observations
  switch (logical_panel) {
    case 0:  // Top-left logical position
      physical_panel = 0;  // Map to physical panel 0 (or whichever is in this position)
      orientation = 0;     // Normal orientation
      break;
    case 1:  // Top-right logical position
      physical_panel = 1;  // Map to physical panel 1
      orientation = 0;     // Normal orientation
      break;
    case 2:  // Bottom-left logical position
      physical_panel = 2;  // Map to physical panel 2
      orientation = 0;     // Normal orientation
      break;
    case 3:  // Bottom-right logical position
      physical_panel = 3;  // Map to physical panel 3
      orientation = 0;     // Normal orientation
      break;
  }
  
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
  
  // Calculate physical panel position
  int physical_x = (physical_panel % 2) * PANEL_W;
  int physical_y = (physical_panel / 2) * PANEL_H;
  
  // Final coordinates
  *mapped_x = physical_x + rotated_x;
  *mapped_y = physical_y + rotated_y;
}
```

## 4. Common Custom Arrangements

Here are mapping functions for some common custom arrangements:

### 4.1 Linear Row (1×4)

```
┌─────┬─────┬─────┬─────┐
│     │     │     │     │
│  1  │  2  │  3  │  4  │
│     │     │     │     │
└─────┴─────┴─────┴─────┘
```

```cpp
void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
  const int16_t PANEL_W = 64;
  const int16_t PANEL_H = 64;
  
  // In a 1×4 arrangement, we need to determine which panel
  int panel_num = x / PANEL_W;  // Panel 0-3 from left to right
  
  // Local coordinates within the panel
  int local_x = x % PANEL_W;
  
  // Map to physical coordinates (adjust panel order if needed)
  int physical_panel = panel_num;  // Change this if panels are in different order
  
  // Final coordinates (assuming normal orientation)
  *mapped_x = (physical_panel * PANEL_W) + local_x;
  *mapped_y = y;
}
```

### 4.2 Linear Column (4×1)

```
┌─────┐
│     │
│  1  │
│     │
├─────┤
│     │
│  2  │
│     │
├─────┤
│     │
│  3  │
│     │
├─────┤
│     │
│  4  │
│     │
└─────┘
```

```cpp
void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
  const int16_t PANEL_W = 64;
  const int16_t PANEL_H = 64;
  
  // In a 4×1 arrangement, we need to determine which panel
  int panel_num = y / PANEL_H;  // Panel 0-3 from top to bottom
  
  // Local coordinates within the panel
  int local_y = y % PANEL_H;
  
  // Map to physical coordinates (adjust panel order if needed)
  int physical_panel = panel_num;  // Change this if panels are in different order
  
  // Final coordinates (assuming normal orientation)
  *mapped_x = x;
  *mapped_y = (physical_panel * PANEL_H) + local_y;
}
```

### 4.3 Square With One Rotated Panel

```
┌─────┬─────┐
│     │     │
│  1  │  2  │
│     │     │
├─────┼─────┤
│     │ 4   │
│  3  │  ↻  │
│     │     │
└─────┴─────┘
```

```cpp
void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
  const int16_t PANEL_W = 64;
  const int16_t PANEL_H = 64;
  
  // Determine which logical panel the coordinates are in
  int panel_x = x / PANEL_W;
  int panel_y = y / PANEL_H;
  
  // Get the logical panel number (0-3)
  int logical_panel = panel_y * 2 + panel_x;
  
  // Local coordinates within the panel
  int local_x = x % PANEL_W;
  int local_y = y % PANEL_H;
  
  // For panel 3 (bottom-right), apply 90° rotation
  if (logical_panel == 3) {
    // 90° clockwise rotation
    int temp_x = local_x;
    local_x = PANEL_W - 1 - local_y;
    local_y = temp_x;
  }
  
  // Calculate physical panel position
  int physical_x = (panel_x) * PANEL_W;
  int physical_y = (panel_y) * PANEL_H;
  
  // Final coordinates
  *mapped_x = physical_x + local_x;
  *mapped_y = physical_y + local_y;
}
```

## 5. Testing Your Mapping

After implementing your custom mapping, run these tests:

### 5.1 Grid Test

Draw a grid across the entire display to verify continuity:

```cpp
void testGrid() {
  matrix.fillScreen(0);
  
  // Draw a grid with lines every 8 pixels
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
}
```

### 5.2 Boundary Test

Draw lines at panel boundaries to verify alignment:

```cpp
void testBoundaries() {
  matrix.fillScreen(0);
  
  // Draw panel boundaries
  for (int i = 0; i <= 4; i++) {
    // Vertical boundaries (assuming 2x2 grid - adjust as needed)
    int x = i * (TOTAL_WIDTH / 2);
    if (x < TOTAL_WIDTH) {
      for (int y = 0; y < TOTAL_HEIGHT; y++) {
        matrix.drawPixel(x, y, matrix.color565(0, 255, 0));
      }
    }
    
    // Horizontal boundaries (assuming 2x2 grid - adjust as needed)
    int y = i * (TOTAL_HEIGHT / 2);
    if (y < TOTAL_HEIGHT) {
      for (int x = 0; x < TOTAL_WIDTH; x++) {
        matrix.drawPixel(x, y, matrix.color565(0, 255, 0));
      }
    }
  }
  
  matrix.show();
}
```

### 5.3 Text Spanning Test

Display text that spans across panel boundaries:

```cpp
void testSpanningText() {
  matrix.fillScreen(0);
  matrix.setTextSize(2);
  matrix.setTextColor(matrix.color565(255, 255, 255));
  
  // Center text across the entire display
  matrix.setCursor(TOTAL_WIDTH/2 - 80, TOTAL_HEIGHT/2 - 8);
  matrix.print("SPANNING TEXT");
  
  matrix.show();
}
```

Remember to adjust these tests based on your specific panel arrangement!