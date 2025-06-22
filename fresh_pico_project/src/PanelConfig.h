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

// CONFIRMED configuration based on final testing
// The physical panel layout and actual positions observed:
//  ┌─────┬─────┐
//  │     │     │
//  │  P0 │  P1 │
//  │     │     │
//  ├─────┼─────┤
//  │     │     │
//  │  P2 │  P3 │
//  │     │     │
//  └─────┴─────┘
//
// The physical/electrical chain ordering is:
//  ┌─────┬─────┐
//  │     │     │
//  │  1  │  2  │
//  │     │     │
//  ├─────┼─────┤
//  │     │     │
//  │  0  │  3  │
//  │     │     │
//  └─────┴─────┘
//
// All panels need 180° rotation to fix upside-down text
//
// We need to place our desired content in the observed positions:
// - For P0 (RED) content: Place in bottom-right quadrant
// - For P1 (GREEN) content: Place in top-left quadrant
// - For P2 (BLUE) content: Place in bottom-left quadrant
// - For P3 (YELLOW) content: Place in top-right quadrant
//
const PanelConfig PANEL_CONFIGS[PANEL_COUNT] = {
    {0, 1, 2},  // Logical position 0 (top-left) -> Physical position 1 (second in chain), 180° rotation
    {1, 2, 2},  // Logical position 1 (top-right) -> Physical position 2 (third in chain), 180° rotation
    {2, 0, 2},  // Logical position 2 (bottom-left) -> Physical position 0 (first in chain), 180° rotation
    {3, 3, 2}   // Logical position 3 (bottom-right) -> Physical position 3 (fourth in chain), 180° rotation
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
    // This assumes a 2x2 arrangement - modify for your specific arrangement
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
    int physical_x = (config->physicalPosition % 2) * PANEL_W;
    int physical_y = (config->physicalPosition / 2) * PANEL_H;
    
    // Final coordinates
    *mapped_x = physical_x + rotated_x;
    *mapped_y = physical_y + rotated_y;
}

#endif // PANEL_CONFIG_H