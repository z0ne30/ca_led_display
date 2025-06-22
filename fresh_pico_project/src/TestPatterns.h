#ifndef TEST_PATTERNS_H
#define TEST_PATTERNS_H

#include "PanelConfig.h"
#include <Adafruit_GFX.h>

// Collection of test patterns for RGB LED matrix
class TestPatterns {
private:
    // Reference to the matrix object
    Adafruit_GFX* matrix;
    uint16_t (*color565)(uint8_t, uint8_t, uint8_t);
    void (*showDisplay)();

public:
    // Constructor
    TestPatterns(Adafruit_GFX* m, uint16_t (*colorFn)(uint8_t, uint8_t, uint8_t), void (*showFn)()) {
        matrix = m;
        color565 = colorFn;
        showDisplay = showFn;
    }

    // Panel identification test
    void panelIdentification(uint16_t duration = 5000) {
        matrix->fillScreen(0);
        
        // Draw different colors on each panel to identify panel order
        
        // Panel 0 (top-left) - Red
        for (int y = 0; y < PANEL_HEIGHT; y++) {
            for (int x = 0; x < PANEL_WIDTH; x++) {
                int16_t mapped_x, mapped_y;
                mapCoordinates(x, y, &mapped_x, &mapped_y);
                matrix->drawPixel(mapped_x, mapped_y, color565(255, 0, 0));
            }
        }
        
        // Panel 1 (top-right) - Green
        for (int y = 0; y < PANEL_HEIGHT; y++) {
            for (int x = 0; x < PANEL_WIDTH; x++) {
                int16_t mapped_x, mapped_y;
                mapCoordinates(PANEL_WIDTH + x, y, &mapped_x, &mapped_y);
                matrix->drawPixel(mapped_x, mapped_y, color565(0, 255, 0));
            }
        }
        
        // Panel 2 (bottom-left) - Blue
        for (int y = 0; y < PANEL_HEIGHT; y++) {
            for (int x = 0; x < PANEL_WIDTH; x++) {
                int16_t mapped_x, mapped_y;
                mapCoordinates(x, PANEL_HEIGHT + y, &mapped_x, &mapped_y);
                matrix->drawPixel(mapped_x, mapped_y, color565(0, 0, 255));
            }
        }
        
        // Panel 3 (bottom-right) - Yellow
        for (int y = 0; y < PANEL_HEIGHT; y++) {
            for (int x = 0; x < PANEL_WIDTH; x++) {
                int16_t mapped_x, mapped_y;
                mapCoordinates(PANEL_WIDTH + x, PANEL_HEIGHT + y, &mapped_x, &mapped_y);
                matrix->drawPixel(mapped_x, mapped_y, color565(255, 255, 0));
            }
        }
        
        showDisplay();
        delay(duration);
    }

    // Panel numbers test
    void panelNumbers(uint16_t duration = 5000) {
        matrix->fillScreen(0);
        
        // Draw panel numbers on each panel
        drawNumber(0, 0, 1);                    // Panel 0 - Top Left
        drawNumber(PANEL_WIDTH, 0, 2);           // Panel 1 - Top Right
        drawNumber(0, PANEL_HEIGHT, 3);          // Panel 2 - Bottom Left
        drawNumber(PANEL_WIDTH, PANEL_HEIGHT, 4); // Panel 3 - Bottom Right
        
        showDisplay();
        delay(duration);
    }

    // Grid test
    void gridTest(uint16_t duration = 5000) {
        matrix->fillScreen(0);
        
        // Draw grid lines
        for (int x = 0; x < TOTAL_WIDTH; x += 8) {
            for (int y = 0; y < TOTAL_HEIGHT; y++) {
                int16_t mapped_x, mapped_y;
                mapCoordinates(x, y, &mapped_x, &mapped_y);
                matrix->drawPixel(mapped_x, mapped_y, color565(64, 64, 64));
            }
        }
        
        for (int y = 0; y < TOTAL_HEIGHT; y += 8) {
            for (int x = 0; x < TOTAL_WIDTH; x++) {
                int16_t mapped_x, mapped_y;
                mapCoordinates(x, y, &mapped_x, &mapped_y);
                matrix->drawPixel(mapped_x, mapped_y, color565(64, 64, 64));
            }
        }
        
        // Draw panel boundaries in a different color
        for (int x = 0; x < TOTAL_WIDTH; x++) {
            int16_t mapped_x, mapped_y;
            
            // Horizontal panel boundaries
            mapCoordinates(x, PANEL_HEIGHT-1, &mapped_x, &mapped_y);
            matrix->drawPixel(mapped_x, mapped_y, color565(0, 255, 0));
            
            mapCoordinates(x, PANEL_HEIGHT, &mapped_x, &mapped_y);
            matrix->drawPixel(mapped_x, mapped_y, color565(0, 255, 0));
        }
        
        for (int y = 0; y < TOTAL_HEIGHT; y++) {
            int16_t mapped_x, mapped_y;
            
            // Vertical panel boundaries
            mapCoordinates(PANEL_WIDTH-1, y, &mapped_x, &mapped_y);
            matrix->drawPixel(mapped_x, mapped_y, color565(0, 255, 0));
            
            mapCoordinates(PANEL_WIDTH, y, &mapped_x, &mapped_y);
            matrix->drawPixel(mapped_x, mapped_y, color565(0, 255, 0));
        }
        
        showDisplay();
        delay(duration);
    }

    // Cross-panel line test
    void crossPanelLines(uint16_t duration = 5000) {
        matrix->fillScreen(0);
        
        // Draw diagonal line across entire display
        for (int i = 0; i < TOTAL_WIDTH; i++) {
            int y = i * TOTAL_HEIGHT / TOTAL_WIDTH;
            int16_t mapped_x, mapped_y;
            mapCoordinates(i, y, &mapped_x, &mapped_y);
            matrix->drawPixel(mapped_x, mapped_y, color565(255, 255, 255));
        }
        
        // Draw cross in the center
        for (int x = 0; x < TOTAL_WIDTH; x++) {
            int16_t mapped_x, mapped_y;
            mapCoordinates(x, TOTAL_HEIGHT/2, &mapped_x, &mapped_y);
            matrix->drawPixel(mapped_x, mapped_y, color565(255, 0, 0));
        }
        
        for (int y = 0; y < TOTAL_HEIGHT; y++) {
            int16_t mapped_x, mapped_y;
            mapCoordinates(TOTAL_WIDTH/2, y, &mapped_x, &mapped_y);
            matrix->drawPixel(mapped_x, mapped_y, color565(0, 255, 0));
        }
        
        showDisplay();
        delay(duration);
    }

    // Text test
    void textTest(uint16_t duration = 5000) {
        matrix->fillScreen(0);
        
        matrix->setTextSize(1);
        matrix->setTextColor(color565(255, 255, 255));
        
        // First map the text start position to physical coordinates
        int16_t mapped_x, mapped_y;
        mapCoordinates(TOTAL_WIDTH/2 - 30, TOTAL_HEIGHT/2 - 4, &mapped_x, &mapped_y);
        
        // Position text to span across panels
        matrix->setCursor(mapped_x, mapped_y);
        matrix->print("RGB MATRIX");
        
        showDisplay();
        delay(duration);
    }

    // Color cycle test
    void colorCycle(uint16_t duration = 1000) {
        // Cycle through colors
        uint16_t colors[] = {
            color565(255, 0, 0),     // Red
            color565(0, 255, 0),     // Green
            color565(0, 0, 255),     // Blue
            color565(255, 255, 0),   // Yellow
            color565(0, 255, 255),   // Cyan
            color565(255, 0, 255),   // Magenta
            color565(255, 255, 255)  // White
        };
        
        for (int i = 0; i < 7; i++) {
            matrix->fillScreen(colors[i]);
            showDisplay();
            delay(duration);
        }
    }
    
    // Animated pattern test
    void animatedPattern(uint16_t frames = 60, uint16_t frameDelay = 33) {
        for (int frame = 0; frame < frames; frame++) {
            matrix->fillScreen(0);
            
            // Create a moving pattern
            for (int x = 0; x < TOTAL_WIDTH; x++) {
                for (int y = 0; y < TOTAL_HEIGHT; y++) {
                    int16_t mapped_x, mapped_y;
                    
                    // Create a circular wave pattern
                    int cx = TOTAL_WIDTH / 2;
                    int cy = TOTAL_HEIGHT / 2;
                    int dx = x - cx;
                    int dy = y - cy;
                    int distance = sqrt(dx*dx + dy*dy);
                    
                    // Animate the wave based on frame number
                    if ((distance + frame) % 16 < 8) {
                        mapCoordinates(x, y, &mapped_x, &mapped_y);
                        
                        // Create a rainbow color based on angle
                        float angle = atan2(dy, dx) * 180 / 3.14159;
                        if (angle < 0) angle += 360;
                        
                        uint8_t r = (angle < 120) ? 255 * (120 - angle) / 120 : (angle > 240) ? 255 * (angle - 240) / 120 : 0;
                        uint8_t g = (angle < 120) ? 255 * angle / 120 : (angle < 240) ? 255 * (240 - angle) / 120 : 0;
                        uint8_t b = (angle < 240) ? 0 : 255 * (angle - 240) / 120;
                        
                        matrix->drawPixel(mapped_x, mapped_y, color565(r, g, b));
                    }
                }
            }
            
            showDisplay();
            delay(frameDelay);
        }
    }

private:
    // Helper function to draw a number in the center of a panel
    void drawNumber(int16_t panel_x, int16_t panel_y, int number) {
        int16_t x = panel_x + PANEL_WIDTH/2 - 4;
        int16_t y = panel_y + PANEL_HEIGHT/2 - 4;
        
        int16_t mapped_x, mapped_y;
        mapCoordinates(x, y, &mapped_x, &mapped_y);
        
        matrix->setTextSize(1);
        matrix->setTextColor(color565(255, 255, 255));
        matrix->setCursor(mapped_x, mapped_y);
        matrix->print(number);
        
        // Draw a border around the panel
        for (int i = 0; i < PANEL_WIDTH; i++) {
            int16_t mx1, my1, mx2, my2;
            
            mapCoordinates(panel_x + i, panel_y, &mx1, &my1);
            matrix->drawPixel(mx1, my1, color565(255, 255, 255));
            
            mapCoordinates(panel_x + i, panel_y + PANEL_HEIGHT - 1, &mx2, &my2);
            matrix->drawPixel(mx2, my2, color565(255, 255, 255));
        }
        
        for (int i = 0; i < PANEL_HEIGHT; i++) {
            int16_t mx1, my1, mx2, my2;
            
            mapCoordinates(panel_x, panel_y + i, &mx1, &my1);
            matrix->drawPixel(mx1, my1, color565(255, 255, 255));
            
            mapCoordinates(panel_x + PANEL_WIDTH - 1, panel_y + i, &mx2, &my2);
            matrix->drawPixel(mx2, my2, color565(255, 255, 255));
        }
    }
};

#endif // TEST_PATTERNS_H