# LED Matrix Panel Mapping Guide

This document explains how to correctly map and scale RGB LED matrix panel arrays using the Adafruit Protomatter library with microcontrollers like the Raspberry Pi Pico.

## Understanding Panel Configurations

When working with multiple LED matrix panels, there are two critical concepts to understand:

### 1. Physical Chain Order vs. Logical Panel Positions

- **Physical Chain Order**: The order in which data flows through the panels based on how they are wired together.
- **Logical Panel Positions**: The actual positions where content appears on the display.

These two mappings often don't align intuitively, causing confusion when content appears in unexpected locations.

## Basic Library Configuration

### Initializing for Different Panel Arrangements

For the Adafruit_Protomatter library, proper initialization is critical:

```cpp
Adafruit_Protomatter matrix(
  PANEL_WIDTH * columns,  // Total width of the display
  4,                      // Bit depth
  1, rgbPins,             // RGB pins
  5, addrPins,            // Address pins
  CLK_PIN, LAT_PIN, OE_PIN,
  true,                   // Double-buffering
  rows                    // Number of vertical tiles
);
```

Key parameters:
- The first parameter must be the **total width** (single panel width × number of columns)
- The last parameter specifies vertical tiling (number of rows)

### Tiling Options

- **Positive value** (e.g., `2`): Progressive arrangement (left-to-right, top-to-bottom)
- **Negative value** (e.g., `-2`): Serpentine arrangement (alternating 180° orientation)

## Scaling Panel Arrays

### Adding More Panels Horizontally

When adding panels horizontally:
1. Adjust the width parameter to `PANEL_WIDTH * columns`
2. Keep the vertical tiles parameter the same

```cpp
// For a 3×2 arrangement (3 columns, 2 rows)
Adafruit_Protomatter matrix(
  PANEL_WIDTH * 3,  // 3 panels wide
  4,
  1, rgbPins,
  5, addrPins,
  CLK_PIN, LAT_PIN, OE_PIN,
  true,
  2               // 2 rows
);
```

### Adding More Panels Vertically

When adding panels vertically:
1. Keep the width parameter the same
2. Adjust the vertical tiles parameter to match the number of rows

```cpp
// For a 2×3 arrangement (2 columns, 3 rows)
Adafruit_Protomatter matrix(
  PANEL_WIDTH * 2,  // 2 panels wide
  4,
  1, rgbPins,
  5, addrPins,
  CLK_PIN, LAT_PIN, OE_PIN,
  true,
  3               // 3 rows
);
```

## Content Mapping Strategy

### Step 1: Create a Diagram of Your Physical Layout

Document both the physical chain order and the desired logical layout:

```
Physical Chain Order:    Desired Logical Layout:
┌─────┬─────┐           ┌─────┬─────┐
│     │     │           │     │     │
│  1  │  2  │           │  0  │  1  │
│     │     │           │     │     │
├─────┼─────┤           ├─────┼─────┤
│     │     │           │     │     │
│  0  │  3  │           │  2  │  3  │
│     │     │           │     │     │
└─────┴─────┘           └─────┴─────┘
```

### Step 2: Create Test Patterns to Identify Panel Positions

Develop a test pattern that shows different colors and labels on each panel to identify their actual positions:

```cpp
// Fill each quadrant with a different color
matrix.fillRect(0, 0, PANEL_WIDTH, PANEL_HEIGHT, RED);
matrix.fillRect(PANEL_WIDTH, 0, PANEL_WIDTH, PANEL_HEIGHT, GREEN);
matrix.fillRect(0, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT, BLUE);
matrix.fillRect(PANEL_WIDTH, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT, YELLOW);

// Add position labels
matrix.setTextColor(WHITE);
matrix.setCursor(20, 20);
matrix.print("P0");
matrix.setCursor(PANEL_WIDTH + 20, 20);
matrix.print("P1");
// etc.
```

### Step 3: Map Content to Observed Positions

After identifying the actual positions, adjust your content drawing code to place content in the correct positions:

```cpp
// If panel P0 should display at position (1,0) but appears at (0,0):
// Place P0 content at (0,0) coordinates
```

## Text Orientation and Positioning

### Handling Rotation

Use the rotation setting to correct upside-down text:

```cpp
matrix.setRotation(2);  // 180 degrees rotation
```

### Centering Text

Calculate the center position for each panel:

```cpp
int charWidth = 18;  // For a 3x font size
int charHeight = 24;
int centerX = PANEL_WIDTH/2 - charWidth;
int centerY = PANEL_HEIGHT/2 - charHeight/2;
matrix.setCursor(centerX, centerY);
```

## Scaling Best Practices

1. **Document your panel layout** with both physical chain order and logical positions
2. **Test incrementally** when adding new panels
3. **Use helper functions** to abstract coordinate mapping logic
4. **Create a coordinate mapping function** for larger arrays:

```cpp
void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
  // Determine which logical panel the coordinates are in
  int panel_x = x / PANEL_WIDTH;
  int panel_y = y / PANEL_HEIGHT;
  int logical_panel = panel_y * COLUMNS + panel_x;
  
  // Local coordinates within the panel
  int local_x = x % PANEL_WIDTH;
  int local_y = y % PANEL_HEIGHT;
  
  // Get physical panel position based on your mapping
  int physical_panel = your_custom_mapping_function(logical_panel);
  int physical_x = (physical_panel % COLUMNS) * PANEL_WIDTH;
  int physical_y = (physical_panel / COLUMNS) * PANEL_HEIGHT;
  
  // Apply any needed rotation to local coordinates
  // ...
  
  // Final coordinates
  *mapped_x = physical_x + local_x;
  *mapped_y = physical_y + local_y;
}
```

## Conclusion

Properly mapping LED matrix panels requires understanding both the physical chain order and the logical positioning. Through careful initialization, testing, and content mapping, you can create displays with any number of panels that correctly show your intended content.