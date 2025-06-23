#ifndef PANEL_CONFIG_H
#define PANEL_CONFIG_H

#include <stdint.h>

// Matrix configuration
#ifndef PANEL_WIDTH
#define PANEL_WIDTH 64
#endif

#ifndef PANEL_HEIGHT
#define PANEL_HEIGHT 64
#endif

#ifndef PANEL_COUNT
#define PANEL_COUNT 4
#endif

// Assume a 2x2 grid layout by default - modify based on your arrangement
#define TOTAL_WIDTH (PANEL_WIDTH * 2)
#define TOTAL_HEIGHT (PANEL_HEIGHT * 2)

// Panel layout configuration
// Change these values to match your specific panel arrangement
typedef struct {
    uint8_t logicalPosition;  // Position in the logical grid (0-3 for a 2x2 grid)
    uint8_t physicalPosition; // Position in the physical chain (0-3 for 4 panels)
    uint8_t rotation;         // 0=normal, 1=90° CW, 2=180°, 3=270° CW
} PanelConfig;

// CORRECTED configuration based on observed panel layout in image
// The logical panel layout (how we want to address them in code):
//  ┌─────┬─────┐
//  │     │     │
//  │  P0 │  P1 │ (TL)  (TR)
//  │     │     │
//  ├─────┼─────┤
//  │     │     │
//  │  P2 │  P3 │ (BL)  (BR)
//  │     │     │
//  └─────┴─────┘
//
// The actual physical arrangement observed in the image:
//  ┌─────┬─────┐
//  │     │     │
//  │  BR │  TR │ (P3)  (P1)
//  │     │     │
//  ├─────┼─────┤
//  │     │     │
//  │  TL │  BL │ (P0)  (P2)
//  │     │     │
//  └─────┴─────┘
//
// Signal flow: Pico sends signal to TR panel first (Physical position 1),
// then the chain continues through the other panels.
//
const PanelConfig PANEL_CONFIGS[PANEL_COUNT] = {
    {0, 3, 0},  // Logical position 0 (TL) -> Physical position 3 (bottom-left)
    {1, 1, 0},  // Logical position 1 (TR) -> Physical position 1 (top-right)
    {2, 2, 0},  // Logical position 2 (BL) -> Physical position 2 (bottom-right)
    {3, 0, 0}   // Logical position 3 (BR) -> Physical position 0 (top-left)
};

// Function to map a logical panel number to a physical panel configuration
inline const PanelConfig* getPanelConfig(uint8_t logicalPanel) {
    for (uint8_t i = 0; i < PANEL_COUNT; i++) {
        if (PANEL_CONFIGS[i].logicalPosition == logicalPanel) {
            return &PANEL_CONFIGS[i];
        }
    }
    // Default to first panel if not found
    return &PANEL_CONFIGS[0];
}

// Custom mapping function
inline void mapCoordinates(int16_t x, int16_t y, int16_t* mapped_x, int16_t* mapped_y) {
    const int16_t PANEL_W = PANEL_WIDTH;
    const int16_t PANEL_H = PANEL_HEIGHT;
    
    // Determine which logical panel the coordinates are in
    // This assumes a 2x2 arrangement
    int panel_x = x / PANEL_W;
    int panel_y = y / PANEL_H;
    
    // Get the logical panel number (0-3 for a 2x2 grid)
    int logical_panel = panel_y * 2 + panel_x;
    
    // Local coordinates within the panel
    int local_x = x % PANEL_W;
    int local_y = y % PANEL_H;
    
    // Get panel configuration
    const PanelConfig* config = getPanelConfig(logical_panel);
    
    // Apply rotation based on panel configuration
    int rotated_x = local_x;
    int rotated_y = local_y;
    
    switch (config->rotation) {
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
    
    // Calculate physical panel position based on its position in the chain
    // Physical positions according to the actual hardware arrangement:
    // Physical positions: 0=top-left, 1=top-right, 2=bottom-right, 3=bottom-left
    int physical_x, physical_y;
    
    switch (config->physicalPosition) {
        case 0:  // Top-left
            physical_x = 0;
            physical_y = 0;
            break;
        case 1:  // Top-right
            physical_x = PANEL_W;
            physical_y = 0;
            break;
        case 2:  // Bottom-right
            physical_x = PANEL_W;
            physical_y = PANEL_H;
            break;
        case 3:  // Bottom-left
            physical_x = 0;
            physical_y = PANEL_H;
            break;
    }
    
    // Final coordinates
    *mapped_x = physical_x + rotated_x;
    *mapped_y = physical_y + rotated_y;
}

#endif // PANEL_CONFIG_H