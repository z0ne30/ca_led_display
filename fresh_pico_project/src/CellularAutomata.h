#ifndef CELLULAR_AUTOMATA_H
#define CELLULAR_AUTOMATA_H

#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include "PanelConfig.h"

// Number of distinct automata implementations
#define NUM_AUTOMATA 7

// Forward declarations of automata classes
class CellularAutomaton;
class ElementaryAutomaton;
class GameOfLife;
class BriansBrain;
class LangtonsAnt;
class CyclicAutomaton;
class BubblingLava;
class OrderAndChaos;

/**
 * Base class for all cellular automata
 */
class CellularAutomaton {
public:
    // Constructor
    CellularAutomaton(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height)
        : matrix(matrix), width(width), height(height), frameCount(0) {}
    
    // Destructor
    virtual ~CellularAutomaton() {}
    
    // Initialize the automaton with random or preset values
    virtual void init() = 0;
    
    // Update the automaton state
    virtual void update() = 0;
    
    // Render the current state to the matrix
    virtual void render() = 0;
    
    // Run a single step (update and render)
    void step() {
        update();
        render();
        frameCount++;
    }
    
    // Get the name of this automaton
    virtual const char* getName() const = 0;
    
protected:
    // Helper function for consistent coordinate mapping across all automata
    // Uses the mapCoordinates function from PanelConfig.h to ensure consistent mapping
    void drawMappedPixel(int16_t x, int16_t y, uint16_t color) {
        // Map the logical coordinates to physical coordinates using the global mapping function
        int16_t mapped_x, mapped_y;
        mapCoordinates(x, y, &mapped_x, &mapped_y);
        
        // Draw the pixel at the mapped position
        matrix->drawPixel(mapped_x, mapped_y, color);
    }
    
    Adafruit_Protomatter* matrix;  // Pointer to the LED matrix
    uint16_t width;                // Width of the matrix
    uint16_t height;               // Height of the matrix
    uint32_t frameCount;           // Current frame count
};

/**
 * 1D Elementary Cellular Automaton (Rule 30, 90, 110, etc.)
 * 
 * These are the simplest cellular automata, where each cell has 2 possible states (0 or 1)
 * and the state of each cell in the next generation depends only on its current state
 * and the state of its two immediate neighbors.
 */
class ElementaryAutomaton : public CellularAutomaton {
public:
    // Initialization patterns
    enum InitPattern {
        SINGLE_CELL,    // Single cell in the middle
        RANDOM_CELLS,   // Random cells across the top row
        ALTERNATING,    // Alternating 0-1 pattern
        TWO_CELLS,      // Two adjacent cells in the middle
        THREE_CELLS     // Three adjacent cells in the middle
    };
    
    ElementaryAutomaton(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height, uint8_t rule = 30) 
        : CellularAutomaton(matrix, width, height), rule(rule), initPattern(SINGLE_CELL) {
        cells = new uint8_t[width * height];
        tempCells = new uint8_t[width];
        
        // Initialize color based on rule
        updateColor();
    }
    
    ~ElementaryAutomaton() {
        delete[] cells;
        delete[] tempCells;
    }
    
    void init() override {
        // Clear the cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Choose a random initialization pattern if not specified
        if (random(100) < 70) {
            // 70% chance to use the default pattern for this rule
            initWithDefaultPattern();
        } else {
            // 30% chance to use a random pattern
            InitPattern randomPattern = static_cast<InitPattern>(random(5));
            initWithPattern(randomPattern);
        }
        
        // Reset current row
        currentRow = 0;
    }
    
    void update() override {
        // Check if we've filled the screen
        if (currentRow >= height - 1) {
            // Reset to the top with a new random rule
            randomRule();
            init();
            return;
        }
        
        // Increment row
        currentRow++;
        
        // Calculate next generation based on the rule
        for (uint16_t x = 0; x < width; x++) {
            // Get the left, center, and right cells
            uint8_t left = cells[(currentRow - 1) * width + ((x + width - 1) % width)];
            uint8_t center = cells[(currentRow - 1) * width + x];
            uint8_t right = cells[(currentRow - 1) * width + ((x + 1) % width)];
            
            // Compute the pattern index (0-7)
            uint8_t pattern = (left << 2) | (center << 1) | right;
            
            // Apply the rule
            tempCells[x] = (rule >> pattern) & 1;
        }
        
        // Copy temp cells to the current row
        for (uint16_t x = 0; x < width; x++) {
            cells[currentRow * width + x] = tempCells[x];
        }
    }
    
    void render() override {
        // Optimized rendering with consistent panel mapping
        
        // For Elementary Automaton, we need to be careful with how we map coordinates
        // as the animation direction should follow the physical panel layout
        
        // First fill the entire display with black
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                drawMappedPixel(x, y, 0);  // Black background
            }
        }
        
        // Then draw only the active cells up to the current row
        for (uint16_t y = 0; y <= currentRow && y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[y * width + x]) {
                    drawMappedPixel(x, y, cellColor);  // Colored cells based on rule
                }
            }
        }
        
        matrix->show();
    }
    
    void setRule(uint8_t newRule) {
        rule = newRule;
        updateColor();
        init();  // Reinitialize with the new rule
    }
    
    // Set a specific initialization pattern
    void setInitPattern(InitPattern pattern) {
        initPattern = pattern;
        init();  // Reinitialize with the new pattern
    }
    
    const char* getName() const override {
        static char name[32];
        
        // Add descriptive names for well-known rules
        switch (rule) {
            case 30:
                sprintf(name, "Rule 30 (Chaos)");
                break;
            case 90:
                sprintf(name, "Rule 90 (Sierpinski)");
                break;
            case 110:
                sprintf(name, "Rule 110 (Universal)");
                break;
            case 184:
                sprintf(name, "Rule 184 (Traffic)");
                break;
            default:
                sprintf(name, "Rule %d", rule);
                break;
        }
        
        return name;
    }
    
    // Change to a random interesting rule
    void randomRule() {
        // Some interesting rules
        const uint8_t interestingRules[] = {
            30,   // Chaotic patterns
            54,   // Stable patterns with complex boundaries
            60,   // Stable patterns
            90,   // Sierpinski triangle
            102,  // Complex patterns
            110,  // Turing complete
            150,  // Symmetric patterns
            158,  // Complex patterns
            182,  // Complex patterns
            184,  // Traffic flow model
            190   // Complex patterns
        };
        
        rule = interestingRules[random(sizeof(interestingRules) / sizeof(interestingRules[0]))];
        updateColor();
    }
    
private:
    uint8_t* cells;       // Cell states
    uint8_t* tempCells;   // Temporary buffer for computation
    uint8_t rule;         // The rule to apply (0-255)
    uint16_t currentRow;  // Current row being calculated
    InitPattern initPattern; // Current initialization pattern
    uint16_t cellColor;   // Color for active cells
    
    // Update cell color based on rule
    void updateColor() {
        // Randomly select a color scheme
        uint8_t colorScheme = random(5);
        
        switch (colorScheme) {
            case 0:
                // Random vibrant color
                cellColor = matrix->color565(random(150, 256), random(150, 256), random(150, 256));
                break;
                
            case 1:
                // Rule-based colors (original behavior)
                if (rule == 30 || rule == 45 || rule == 73 || rule == 75) {
                    // Chaotic rules - red tones
                    cellColor = matrix->color565(255, 100, 100);
                } else if (rule == 90 || rule == 150 || rule == 182) {
                    // Fractal/symmetric rules - blue tones
                    cellColor = matrix->color565(100, 100, 255);
                } else if (rule == 110 || rule == 124 || rule == 137 || rule == 193) {
                    // Complex/universal rules - green tones
                    cellColor = matrix->color565(100, 255, 100);
                } else if (rule == 184 || rule == 232) {
                    // Traffic/flow rules - yellow tones
                    cellColor = matrix->color565(255, 255, 100);
                } else {
                    // Other rules - white
                    cellColor = matrix->color565(255, 255, 255);
                }
                break;
                
            case 2:
                // Random pastel color
                cellColor = matrix->color565(random(180, 256), random(180, 256), random(180, 256));
                break;
                
            case 3:
                // Random primary color (R, G, or B dominant)
                switch (random(3)) {
                    case 0: cellColor = matrix->color565(255, random(100), random(100)); break; // Red
                    case 1: cellColor = matrix->color565(random(100), 255, random(100)); break; // Green
                    case 2: cellColor = matrix->color565(random(100), random(100), 255); break; // Blue
                }
                break;
                
            case 4:
                // Random warm or cool color
                if (random(2)) {
                    // Warm (red/yellow/orange)
                    cellColor = matrix->color565(random(200, 256), random(100, 200), random(50));
                } else {
                    // Cool (blue/green/purple)
                    cellColor = matrix->color565(random(50), random(100, 200), random(200, 256));
                }
                break;
        }
    }
    
    // Initialize with the default pattern for this rule
    void initWithDefaultPattern() {
        // Choose initialization pattern based on the rule
        switch (rule) {
            case 30:
            case 45:
            case 73:
            case 75:
                // Chaotic rules - single cell works well
                initWithPattern(SINGLE_CELL);
                break;
                
            case 90:
            case 150:
            case 182:
                // Fractal/symmetric rules - single cell for Sierpinski
                initWithPattern(SINGLE_CELL);
                break;
                
            case 110:
            case 124:
            case 137:
            case 193:
                // Complex/universal rules - random cells work well
                initWithPattern(RANDOM_CELLS);
                break;
                
            case 184:
                // Traffic rule - optimized initialization
                initTrafficRule();
                break;
                
            case 232:
                // Other flow rule - alternating pattern
                initWithPattern(ALTERNATING);
                break;
                
            default:
                // Default to single cell for other rules
                initWithPattern(SINGLE_CELL);
                break;
        }
    }
    
    // Special initialization for traffic rule (Rule 184)
    void initTrafficRule() {
        // Clear the cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // For traffic rule, we want a mix of vehicles and spaces
        // Density around 40-60% works well for interesting traffic patterns
        uint8_t density = random(40, 61);
        
        // Initialize top row with random cells based on density
        for (uint16_t x = 0; x < width; x++) {
            cells[x] = (random(100) < density) ? 1 : 0;
        }
        
        // Optionally add a traffic jam section
        if (random(100) < 70) {  // 70% chance to add a traffic jam
            uint16_t jamStart = random(width / 4);
            uint16_t jamLength = random(width / 4, width / 2);
            
            // Create a dense section (traffic jam)
            for (uint16_t x = jamStart; x < jamStart + jamLength && x < width; x++) {
                cells[x] = (random(100) < 80) ? 1 : 0;  // 80% density in jam
            }
            
            // Create a sparse section (open road) after the jam
            uint16_t openStart = (jamStart + jamLength) % width;
            uint16_t openLength = random(width / 4, width / 2);
            
            for (uint16_t x = openStart; x < openStart + openLength && x < width; x++) {
                cells[x] = (random(100) < 20) ? 1 : 0;  // 20% density in open road
            }
        }
        
        // Save the current pattern
        initPattern = RANDOM_CELLS;  // Closest match
    }
    
    // Initialize with a specific pattern
    void initWithPattern(InitPattern pattern) {
        // Clear the cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Apply the specified pattern to the top row
        switch (pattern) {
            case SINGLE_CELL:
                // Single cell in the middle
                cells[width / 2] = 1;
                break;
                
            case RANDOM_CELLS:
                // Random cells across the top row
                for (uint16_t x = 0; x < width; x++) {
                    cells[x] = random(2);
                }
                break;
                
            case ALTERNATING:
                // Alternating 0-1 pattern
                for (uint16_t x = 0; x < width; x++) {
                    cells[x] = x % 2;
                }
                break;
                
            case TWO_CELLS:
                // Two adjacent cells in the middle
                cells[width / 2] = 1;
                cells[width / 2 + 1] = 1;
                break;
                
            case THREE_CELLS:
                // Three adjacent cells in the middle
                cells[width / 2 - 1] = 1;
                cells[width / 2] = 1;
                cells[width / 2 + 1] = 1;
                break;
        }
        
        // Save the current pattern
        initPattern = pattern;
    }
};

/**
 * Life-like Cellular Automata
 * 
 * This class implements Conway's Game of Life and other Life-like cellular automata
 * using the Bx/Sy notation, where:
 * - B = Birth - dead cells with these numbers of neighbors become live
 * - S = Survival - live cells with these numbers of neighbors stay live
 */
class GameOfLife : public CellularAutomaton {
public:
    // Predefined rule sets for different Life-like cellular automata
    enum RuleSet {
        CONWAY,      // B3/S23 - Conway's Game of Life
        DAY_NIGHT,   // B3678/S34678 - Day and Night
        MAZE,        // B3/S12345 - Maze
        MAZECTRIC,   // B3/S1234 - Mazectric
        ANNEAL,      // B4678/S35678 - Anneal
        DIAMOEBA     // B35678/S5678 - Diamoeba
    };
    
    GameOfLife(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height, RuleSet ruleSet = CONWAY) 
        : CellularAutomaton(matrix, width, height) {
        cells = new uint8_t[width * height];
        nextCells = new uint8_t[width * height];
        
        // Set the rule set
        setRuleSet(ruleSet);
        
        // Initialize color palette for different rule sets
        initColorPalette();
    }
    
    ~GameOfLife() {
        delete[] cells;
        delete[] nextCells;
    }
    
    void init() override {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Choose initialization method based on rule set
        switch (currentRuleSet) {
            case CONWAY:
                // Conway's Game of Life - random cells or patterns
                initConway();
                break;
                
            case DAY_NIGHT:
                // Day and Night - higher density of random cells (40-60%)
                initRandom(random(40, 60));
                break;
                
            case MAZE:
            case MAZECTRIC:
                // Maze/Mazectric - small random seed in center
                initCenterSeed();
                break;
                
            case ANNEAL:
                // Anneal - random cells (50%)
                initRandom(50);
                break;
                
            case DIAMOEBA:
                // Diamoeba - random cells (30-40%)
                initRandom(random(30, 40));
                break;
                
            default:
                // Default to random initialization
                initRandom(25);
                break;
        }
    }
    
    void update() override {
        // Calculate the next generation
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Count live neighbors (wrap around edges)
                uint8_t neighbors = 0;
                for (int8_t dy = -1; dy <= 1; dy++) {
                    for (int8_t dx = -1; dx <= 1; dx++) {
                        // Skip the cell itself
                        if (dx == 0 && dy == 0) continue;
                        
                        // Calculate neighbor coordinates with wrapping
                        uint16_t nx = (x + dx + width) % width;
                        uint16_t ny = (y + dy + height) % height;
                        
                        // Add to neighbor count if alive
                        neighbors += cells[ny * width + nx];
                    }
                }
                
                // Apply rules based on birth and survival conditions
                uint8_t current = cells[y * width + x];
                uint8_t next = 0;
                
                if (current) {
                    // Cell is alive - check survival conditions
                    next = (survivalRules & (1 << neighbors)) ? 1 : 0;
                } else {
                    // Cell is dead - check birth conditions
                    next = (birthRules & (1 << neighbors)) ? 1 : 0;
                }
                
                nextCells[y * width + x] = next;
            }
        }
        
        // Swap cell buffers
        uint8_t* temp = cells;
        cells = nextCells;
        nextCells = temp;
    }
    
    void render() override {
        // Draw all cells using our consistent mapping function
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[y * width + x]) {
                    // Live cells - use color based on rule set
                    drawMappedPixel(x, y, cellColor);
                } else {
                    // Dead cells - black
                    drawMappedPixel(x, y, 0);
                }
            }
        }
        
        matrix->show();
    }
    
    // Set a specific rule set
    void setRuleSet(RuleSet ruleSet) {
        currentRuleSet = ruleSet;
        
        // Configure birth and survival rules based on the rule set
        switch (ruleSet) {
            case CONWAY:
                // B3/S23 - Conway's Game of Life
                birthRules = (1 << 3);
                survivalRules = (1 << 2) | (1 << 3);
                break;
                
            case DAY_NIGHT:
                // B3678/S34678 - Day and Night
                birthRules = (1 << 3) | (1 << 6) | (1 << 7) | (1 << 8);
                survivalRules = (1 << 3) | (1 << 4) | (1 << 6) | (1 << 7) | (1 << 8);
                break;
                
            case MAZE:
                // B3/S12345 - Maze
                birthRules = (1 << 3);
                survivalRules = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
                break;
                
            case MAZECTRIC:
                // B3/S1234 - Mazectric
                birthRules = (1 << 3);
                survivalRules = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
                break;
                
            case ANNEAL:
                // B4678/S35678 - Anneal
                birthRules = (1 << 4) | (1 << 6) | (1 << 7) | (1 << 8);
                survivalRules = (1 << 3) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
                break;
                
            case DIAMOEBA:
                // B35678/S5678 - Diamoeba
                birthRules = (1 << 3) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
                survivalRules = (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
                break;
                
            default:
                // Default to Conway's Game of Life
                birthRules = (1 << 3);
                survivalRules = (1 << 2) | (1 << 3);
                break;
        }
        
        // Update cell color based on rule set
        updateCellColor();
    }
    
    // Set custom rules using the Bx/Sy notation
    void setCustomRules(const char* ruleString) {
        // Parse rule string in format "Bx/Sy"
        birthRules = 0;
        survivalRules = 0;
        
        // Find the 'B' and 'S' sections
        const char* birthPart = strchr(ruleString, 'B');
        const char* survivalPart = strchr(ruleString, 'S');
        
        if (birthPart && survivalPart) {
            // Parse birth rules
            birthPart++; // Skip 'B'
            while (*birthPart && *birthPart != '/') {
                if (*birthPart >= '0' && *birthPart <= '8') {
                    birthRules |= (1 << (*birthPart - '0'));
                }
                birthPart++;
            }
            
            // Parse survival rules
            survivalPart++; // Skip 'S'
            while (*survivalPart && *survivalPart != ' ') {
                if (*survivalPart >= '0' && *survivalPart <= '8') {
                    survivalRules |= (1 << (*survivalPart - '0'));
                }
                survivalPart++;
            }
        }
        
        // Set current rule set to custom
        currentRuleSet = static_cast<RuleSet>(-1); // Custom rule set
        
        // Update cell color for custom rules
        cellColor = matrix->color565(200, 200, 200); // Default gray for custom rules
    }
    
    const char* getName() const override {
        static char name[32];
        
        // Return name based on the current rule set
        switch (currentRuleSet) {
            case CONWAY:
                sprintf(name, "Game of Life (B3/S23)");
                break;
            case DAY_NIGHT:
                sprintf(name, "Day and Night (B3678/S34678)");
                break;
            case MAZE:
                sprintf(name, "Maze (B3/S12345)");
                break;
            case MAZECTRIC:
                sprintf(name, "Mazectric (B3/S1234)");
                break;
            case ANNEAL:
                sprintf(name, "Anneal (B4678/S35678)");
                break;
            case DIAMOEBA:
                sprintf(name, "Diamoeba (B35678/S5678)");
                break;
            default:
                // Custom rule set - construct name from rules
                char birthStr[10] = "B";
                char survStr[10] = "S";
                
                // Convert birth rules to string
                int birthIdx = 1;
                for (int i = 0; i <= 8; i++) {
                    if (birthRules & (1 << i)) {
                        birthStr[birthIdx++] = '0' + i;
                    }
                }
                birthStr[birthIdx] = '\0';
                
                // Convert survival rules to string
                int survIdx = 1;
                for (int i = 0; i <= 8; i++) {
                    if (survivalRules & (1 << i)) {
                        survStr[survIdx++] = '0' + i;
                    }
                }
                survStr[survIdx] = '\0';
                
                sprintf(name, "Custom (%s/%s)", birthStr, survStr);
                break;
        }
        
        return name;
    }
    
private:
    uint8_t* cells;          // Current generation
    uint8_t* nextCells;      // Next generation
    uint16_t birthRules;     // Bit field for birth rules (1 << neighbors)
    uint16_t survivalRules;  // Bit field for survival rules (1 << neighbors)
    RuleSet currentRuleSet;  // Current rule set
    uint16_t cellColor;      // Color for live cells
    uint16_t colorPalette[6]; // Color palette for different rule sets
    
    // Initialize color palette for different rule sets
    void initColorPalette() {
        colorPalette[CONWAY] = matrix->color565(255, 255, 255);      // White
        colorPalette[DAY_NIGHT] = matrix->color565(255, 255, 0);     // Yellow
        colorPalette[MAZE] = matrix->color565(255, 255, 255);        // White
        colorPalette[MAZECTRIC] = matrix->color565(255, 255, 100);   // Light yellow
        colorPalette[ANNEAL] = matrix->color565(255, 100, 0);        // Orange
        colorPalette[DIAMOEBA] = matrix->color565(0, 100, 255);      // Blue
        
        // Set initial cell color
        updateCellColor();
    }
    
    // Update cell color based on current rule set
    void updateCellColor() {
        if (currentRuleSet >= 0 && currentRuleSet < 6) {
            cellColor = colorPalette[currentRuleSet];
        } else {
            cellColor = matrix->color565(200, 200, 200); // Default gray for custom rules
        }
    }
    
    // Initialize with random cells
    void initRandom(uint8_t density) {
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                cells[y * width + x] = random(100) < density;
            }
        }
    }
    
    // Initialize with a small random seed in the center
    void initCenterSeed() {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Add random cells in the center
        uint16_t centerX = width / 2;
        uint16_t centerY = height / 2;
        uint16_t radius = min(width, height) / 6;
        
        for (int16_t y = -radius; y <= radius; y++) {
            for (int16_t x = -radius; x <= radius; x++) {
                if (x*x + y*y <= radius*radius) {
                    int16_t px = centerX + x;
                    int16_t py = centerY + y;
                    
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        cells[py * width + px] = random(100) < 50;
                    }
                }
            }
        }
    }
    
    // Initialize Conway's Game of Life with random cells or patterns
    void initConway() {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Choose initialization method
        uint8_t method = random(100);
        
        if (method < 40) {
            // 40% chance for random cells (increased density from 25% to 30-35%)
            initRandom(random(30, 36));
        } else if (method < 80) {
            // 40% chance for multiple patterns
            // Add 3-5 patterns to ensure more activity
            uint8_t numPatterns = random(3, 6);
            for (int i = 0; i < numPatterns; i++) {
                addPattern();
            }
        } else {
            // 20% chance for a "soup" of random cells and patterns
            // Start with some random cells
            initRandom(15);
            
            // Add 2-3 patterns
            uint8_t numPatterns = random(2, 4);
            for (int i = 0; i < numPatterns; i++) {
                addPattern();
            }
        }
        
        // Always ensure there's at least one oscillator to prevent stagnation
        // Add a blinker in a random location
        uint16_t bx = random(width - 4) + 2;
        uint16_t by = random(height - 4) + 2;
        cells[by * width + bx] = 1;
        cells[by * width + (bx+1)] = 1;
        cells[by * width + (bx+2)] = 1;
    }
    
    // Add a common Game of Life pattern
    void addPattern() {
        // Choose a random pattern
        uint8_t pattern = random(8);
        
        // Calculate a random position for the pattern
        uint16_t px = random(width - 10) + 5;
        uint16_t py = random(height - 10) + 5;
        
        switch (pattern) {
            case 0: {
                // Glider
                cells[py * width + px] = 1;
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                cells[(py+1) * width + px] = 0;
                cells[(py+1) * width + (px+1)] = 0;
                cells[(py+1) * width + (px+2)] = 1;
                cells[(py+2) * width + px] = 1;
                cells[(py+2) * width + (px+1)] = 0;
                cells[(py+2) * width + (px+2)] = 0;
                break;
            }
            case 1: {
                // Blinker
                cells[py * width + px] = 1;
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                break;
            }
            case 2: {
                // Block
                cells[py * width + px] = 1;
                cells[py * width + (px+1)] = 1;
                cells[(py+1) * width + px] = 1;
                cells[(py+1) * width + (px+1)] = 1;
                break;
            }
            case 3: {
                // Gosper glider gun (if there's room)
                if (px < width - 36 && py < height - 9) {
                    // Block on left
                    cells[(py+4) * width + (px+0)] = 1;
                    cells[(py+4) * width + (px+1)] = 1;
                    cells[(py+5) * width + (px+0)] = 1;
                    cells[(py+5) * width + (px+1)] = 1;
                    
                    // Left structure
                    cells[(py+2) * width + (px+12)] = 1;
                    cells[(py+2) * width + (px+13)] = 1;
                    cells[(py+3) * width + (px+11)] = 1;
                    cells[(py+3) * width + (px+15)] = 1;
                    cells[(py+4) * width + (px+10)] = 1;
                    cells[(py+4) * width + (px+16)] = 1;
                    cells[(py+5) * width + (px+10)] = 1;
                    cells[(py+5) * width + (px+14)] = 1;
                    cells[(py+5) * width + (px+16)] = 1;
                    cells[(py+5) * width + (px+17)] = 1;
                    cells[(py+6) * width + (px+10)] = 1;
                    cells[(py+6) * width + (px+16)] = 1;
                    cells[(py+7) * width + (px+11)] = 1;
                    cells[(py+7) * width + (px+15)] = 1;
                    cells[(py+8) * width + (px+12)] = 1;
                    cells[(py+8) * width + (px+13)] = 1;
                    
                    // Right structure
                    cells[(py+0) * width + (px+24)] = 1;
                    cells[(py+1) * width + (px+22)] = 1;
                    cells[(py+1) * width + (px+24)] = 1;
                    cells[(py+2) * width + (px+20)] = 1;
                    cells[(py+2) * width + (px+21)] = 1;
                    cells[(py+3) * width + (px+20)] = 1;
                    cells[(py+3) * width + (px+21)] = 1;
                    cells[(py+4) * width + (px+20)] = 1;
                    cells[(py+4) * width + (px+21)] = 1;
                    cells[(py+5) * width + (px+22)] = 1;
                    cells[(py+5) * width + (px+24)] = 1;
                    cells[(py+6) * width + (px+24)] = 1;
                    
                    // Block on right
                    cells[(py+2) * width + (px+34)] = 1;
                    cells[(py+2) * width + (px+35)] = 1;
                    cells[(py+3) * width + (px+34)] = 1;
                    cells[(py+3) * width + (px+35)] = 1;
                }
                break;
            }
            case 4: {
                // Pulsar (period 3 oscillator)
                if (px < width - 15 && py < height - 15) {
                    // Horizontal bars
                    for (int i = 2; i <= 4; i++) {
                        for (int j = 0; j < 3; j++) {
                            // Top left
                            cells[(py+i) * width + (px+j+1)] = 1;
                            // Top right
                            cells[(py+i) * width + (px+j+8)] = 1;
                            // Bottom left
                            cells[(py+i+8) * width + (px+j+1)] = 1;
                            // Bottom right
                            cells[(py+i+8) * width + (px+j+8)] = 1;
                        }
                    }
                    
                    // Vertical bars
                    for (int i = 0; i < 3; i++) {
                        for (int j = 2; j <= 4; j++) {
                            // Top left
                            cells[(py+i+1) * width + (px+j)] = 1;
                            // Top right
                            cells[(py+i+1) * width + (px+j+8)] = 1;
                            // Bottom left
                            cells[(py+i+8) * width + (px+j)] = 1;
                            // Bottom right
                            cells[(py+i+8) * width + (px+j+8)] = 1;
                        }
                    }
                }
                break;
            }
            case 5: {
                // Pentadecathlon (period 15 oscillator)
                if (px < width - 10 && py < height - 10) {
                    // Main body
                    for (int i = 0; i < 8; i++) {
                        cells[(py+1) * width + (px+i+1)] = 1;
                    }
                    
                    // Top and bottom cells
                    cells[py * width + (px+3)] = 1;
                    cells[py * width + (px+6)] = 1;
                    cells[(py+2) * width + (px+3)] = 1;
                    cells[(py+2) * width + (px+6)] = 1;
                }
                break;
            }
            case 6: {
                // R-pentomino (methuselah)
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                cells[(py+1) * width + (px)] = 1;
                cells[(py+1) * width + (px+1)] = 1;
                cells[(py+2) * width + (px+1)] = 1;
                break;
            }
            case 7: {
                // Acorn (methuselah)
                cells[py * width + (px+1)] = 1;
                cells[(py+1) * width + (px+3)] = 1;
                cells[(py+2) * width + (px)] = 1;
                cells[(py+2) * width + (px+1)] = 1;
                cells[(py+2) * width + (px+4)] = 1;
                cells[(py+2) * width + (px+5)] = 1;
                cells[(py+2) * width + (px+6)] = 1;
                break;
            }
        }
    }
};

/**
 * Brian's Brain
 * 
 * This is a cellular automaton with three states:
 * - 0: Off (dead)
 * - 1: On (alive)
 * - 2: Dying
 */
class BriansBrain : public CellularAutomaton {
public:
    BriansBrain(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height) 
        : CellularAutomaton(matrix, width, height) {
        cells = new uint8_t[width * height];
        nextCells = new uint8_t[width * height];
        
        // Initialize with random colors
        randomizeColors();
    }
    
    ~BriansBrain() {
        delete[] cells;
        delete[] nextCells;
    }
    
    void init() override {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Randomly seed cells (about 30% on)
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                cells[y * width + x] = random(100) < 30 ? 1 : 0;
            }
        }
        
        // Randomize colors for variety
        randomizeColors();
    }
    
    void update() override {
        // Calculate the next generation
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                uint8_t current = cells[y * width + x];
                
                if (current == 2) {
                    // Dying cells become off
                    nextCells[y * width + x] = 0;
                } else if (current == 1) {
                    // On cells become dying
                    nextCells[y * width + x] = 2;
                } else {
                    // Off cells check for neighbors
                    uint8_t neighbors = 0;
                    for (int8_t dy = -1; dy <= 1; dy++) {
                        for (int8_t dx = -1; dx <= 1; dx++) {
                            // Skip the cell itself
                            if (dx == 0 && dy == 0) continue;
                            
                            // Calculate neighbor coordinates with wrapping
                            uint16_t nx = (x + dx + width) % width;
                            uint16_t ny = (y + dy + height) % height;
                            
                            // Count only "on" neighbors
                            if (cells[ny * width + nx] == 1) neighbors++;
                        }
                    }
                    
                    // Apply Brian's Brain rule: Off cells become on if exactly 2 neighbors are on
                    nextCells[y * width + x] = (neighbors == 2) ? 1 : 0;
                }
            }
        }
        
        // Swap cell buffers
        uint8_t* temp = cells;
        cells = nextCells;
        nextCells = temp;
    }
    
    void render() override {
        // Optimized rendering using our consistent mapping function
        
        // Draw all cells
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                uint8_t state = cells[y * width + x];
                if (state == 1) {
                    // On cells - use onColor
                    drawMappedPixel(x, y, onColor);
                } else if (state == 2) {
                    // Dying cells - use dyingColor
                    drawMappedPixel(x, y, dyingColor);
                } else {
                    // Off cells - black
                    drawMappedPixel(x, y, 0);
                }
            }
        }
        
        matrix->show();
    }
    
    const char* getName() const override {
        return "Brian's Brain";
    }
    
private:
    uint8_t* cells;      // Current generation
    uint8_t* nextCells;  // Next generation
    uint16_t onColor;    // Color for on cells
    uint16_t dyingColor; // Color for dying cells
    
    // Randomize the colors used for rendering
    void randomizeColors() {
        // Generate a random base hue (0-255)
        uint8_t baseHue = random(256);
        uint8_t r, g, b;
        
        // Choose a random color scheme type
        uint8_t schemeType = random(4);
        
        switch (schemeType) {
            case 0: {
                // Complementary color scheme
                // "On" cells - bright, fully saturated color at the base hue
                hsvToRgb(baseHue, 255, 255, &r, &g, &b);
                onColor = matrix->color565(r, g, b);
                
                // "Dying" cells - complementary color (opposite on color wheel) with lower brightness
                hsvToRgb((baseHue + 128) % 256, 255, 180, &r, &g, &b);
                dyingColor = matrix->color565(r, g, b);
                break;
            }
            
            case 1: {
                // Analogous color scheme
                // "On" cells - bright, fully saturated color at the base hue
                hsvToRgb(baseHue, 255, 255, &r, &g, &b);
                onColor = matrix->color565(r, g, b);
                
                // "Dying" cells - nearby hue with lower brightness
                hsvToRgb((baseHue + 30) % 256, 255, 180, &r, &g, &b);
                dyingColor = matrix->color565(r, g, b);
                break;
            }
            
            case 2: {
                // Brightness gradient (same hue)
                // "On" cells - bright, fully saturated color at the base hue
                hsvToRgb(baseHue, 255, 255, &r, &g, &b);
                onColor = matrix->color565(r, g, b);
                
                // "Dying" cells - same hue but lower brightness
                hsvToRgb(baseHue, 255, 150, &r, &g, &b);
                dyingColor = matrix->color565(r, g, b);
                break;
            }
            
            case 3: {
                // High contrast scheme
                // Choose one of several high-contrast combinations
                uint8_t contrastType = random(5);
                
                switch (contrastType) {
                    case 0: // White/Blue
                        onColor = matrix->color565(255, 255, 255);
                        dyingColor = matrix->color565(0, 0, 255);
                        break;
                    case 1: // Yellow/Red
                        onColor = matrix->color565(255, 255, 0);
                        dyingColor = matrix->color565(255, 0, 0);
                        break;
                    case 2: // Green/Purple
                        onColor = matrix->color565(0, 255, 0);
                        dyingColor = matrix->color565(180, 0, 255);
                        break;
                    case 3: // Cyan/Blue
                        onColor = matrix->color565(0, 255, 255);
                        dyingColor = matrix->color565(0, 80, 255);
                        break;
                    case 4: // Orange/Green
                        onColor = matrix->color565(255, 150, 0);
                        dyingColor = matrix->color565(0, 180, 0);
                        break;
                }
                break;
            }
        }
        
        // Ensure the "on" color is always brighter than the "dying" color
        // This maintains the visual distinction between states that is key to Brian's Brain
        uint8_t r1, g1, b1, r2, g2, b2;
        r1 = (onColor >> 11) & 0x1F;
        g1 = (onColor >> 5) & 0x3F;
        b1 = onColor & 0x1F;
        
        r2 = (dyingColor >> 11) & 0x1F;
        g2 = (dyingColor >> 5) & 0x3F;
        b2 = dyingColor & 0x1F;
        
        // Calculate perceived brightness (rough approximation)
        uint16_t brightness1 = r1 + g1 + b1;
        uint16_t brightness2 = r2 + g2 + b2;
        
        // If dying color is brighter than on color, swap them
        if (brightness2 > brightness1) {
            uint16_t temp = onColor;
            onColor = dyingColor;
            dyingColor = temp;
        }
    }
    
    // Helper function to convert HSV to RGB
    void hsvToRgb(uint8_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b) {
        uint8_t region, remainder, p, q, t;
        
        if (s == 0) {
            *r = *g = *b = v;
            return;
        }
        
        region = h / 43;
        remainder = (h - (region * 43)) * 6;
        
        p = (v * (255 - s)) >> 8;
        q = (v * (255 - ((s * remainder) >> 8))) >> 8;
        t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
        
        switch (region) {
            case 0: *r = v; *g = t; *b = p; break;
            case 1: *r = q; *g = v; *b = p; break;
            case 2: *r = p; *g = v; *b = t; break;
            case 3: *r = p; *g = q; *b = v; break;
            case 4: *r = t; *g = p; *b = v; break;
            default: *r = v; *g = p; *b = q; break;
        }
    }
};

/**
 * Langton's Ant
 */
class LangtonsAnt : public CellularAutomaton {
public:
    // Direction constants
    enum Direction { UP, RIGHT, DOWN, LEFT };
    
    LangtonsAnt(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height, uint8_t numAnts = 1)
        : CellularAutomaton(matrix, width, height), numAnts(numAnts) {
        
        cells = new uint8_t[width * height];
        ants = new Ant[numAnts];
    }
    
    ~LangtonsAnt() {
        delete[] cells;
        delete[] ants;
    }
    
    void init() override {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Initialize ants at random positions
        for (uint8_t i = 0; i < numAnts; i++) {
            ants[i].x = random(width);
            ants[i].y = random(height);
            ants[i].dir = static_cast<Direction>(random(4));
            
            // Assign colors to ants
            switch (i % 6) {
                case 0: ants[i].color = matrix->color565(255, 0, 0); break;     // Red
                case 1: ants[i].color = matrix->color565(0, 255, 0); break;     // Green
                case 2: ants[i].color = matrix->color565(0, 0, 255); break;     // Blue
                case 3: ants[i].color = matrix->color565(255, 255, 0); break;   // Yellow
                case 4: ants[i].color = matrix->color565(255, 0, 255); break;   // Magenta
                case 5: ants[i].color = matrix->color565(0, 255, 255); break;   // Cyan
            }
        }
    }
    
    void update() override {
        // Move each ant
        for (uint8_t i = 0; i < numAnts; i++) {
            Ant& ant = ants[i];
            
            // Get current cell state
            uint8_t cellState = cells[ant.y * width + ant.x];
            
            // Toggle cell state
            cells[ant.y * width + ant.x] = !cellState;
            
            // Turn based on cell state (was white or black before toggling)
            if (cellState) {
                // Turn left on white cell
                ant.dir = static_cast<Direction>((ant.dir + 3) % 4);
            } else {
                // Turn right on black cell
                ant.dir = static_cast<Direction>((ant.dir + 1) % 4);
            }
            
            // Move forward
            switch (ant.dir) {
                case UP:    ant.y = (ant.y - 1 + height) % height; break;
                case RIGHT: ant.x = (ant.x + 1) % width; break;
                case DOWN:  ant.y = (ant.y + 1) % height; break;
                case LEFT:  ant.x = (ant.x - 1 + width) % width; break;
            }
        }
    }
    
    void render() override {
        // Optimized rendering using our consistent mapping function
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[y * width + x]) {
                    drawMappedPixel(x, y, matrix->color565(160, 160, 160));  // Light gray for on cells
                } else {
                    drawMappedPixel(x, y, 0);  // Black for off cells
                }
            }
        }
        
        // Draw all ants on top using our consistent mapping function
        for (uint8_t i = 0; i < numAnts; i++) {
            drawMappedPixel(ants[i].x, ants[i].y, ants[i].color);
        }
        
        matrix->show();
    }
    
    const char* getName() const override {
        static char name[20];
        sprintf(name, "Langton's Ant (%d)", numAnts);
        return name;
    }
    
private:
    struct Ant {
        uint16_t x, y;   // Position
        Direction dir;   // Direction
        uint16_t color;  // Ant color
    };
    
    uint8_t* cells;    // Cell states (0 = black, 1 = white)
    Ant* ants;         // Array of ants
    uint8_t numAnts;   // Number of ants
};

/**
 * Cyclic Cellular Automaton (Rock-Paper-Scissors)
 * 
 * Each cell has a state from 0 to numStates-1.
 * A cell in state s changes to state (s+1) % numStates if enough
 * neighbors are in state (s+1) % numStates.
 * 
 * This is also known as the "Rock-Paper-Scissors" automaton when using 3 states.
 */
class CyclicAutomaton : public CellularAutomaton {
public:
    // Initialization patterns
    enum InitPattern {
        RANDOM,       // Random cells throughout
        CENTER_SEED,  // Small random seed in the center
        QUADRANTS,    // Four quadrants with different states
        STRIPES,      // Horizontal stripes
        SPIRAL        // Spiral pattern
    };
    
    // Preset configurations
    enum Preset {
        SPIRAL_WAVES,    // 8 states, threshold 2 - creates spiral waves
        ROCK_PAPER_SCISSORS, // 3 states, threshold 3 - classic RPS
        COMPLEX_SPIRALS, // 16 states, threshold 1 - complex spiral patterns
        CRYSTAL_GROWTH,  // 6 states, threshold 2 - crystal-like growth
        LABYRINTH,      // 4 states, threshold 2 - labyrinth patterns
        VARIABLE_THRESHOLD, // Variable threshold based on state
        HIGH_STATE_COUNT, // 24-32 states for complex patterns
        SKIP_STATES     // Skip states for discontinuous transitions
    };
    
    CyclicAutomaton(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height, 
                   uint8_t numStates = 16, uint8_t threshold = 2)
        : CellularAutomaton(matrix, width, height), 
          numStates(numStates), threshold(threshold), 
          initPattern(RANDOM), colorScheme(0), range(1),
          variableThreshold(false), stateSkip(1) {
        
        cells = new uint8_t[width * height];
        nextCells = new uint8_t[width * height];
        
        // Initialize color palette
        colorPalette = new uint16_t[numStates];
        generateColorPalette();
    }
    
    ~CyclicAutomaton() {
        delete[] cells;
        delete[] nextCells;
        delete[] colorPalette;
    }
    
    void init() override {
        // Randomize the color scheme for variety
        colorScheme = random(5);
        
        // Create a new color palette based on the random scheme
        generateColorPalette();
        
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
            // Randomize parameters to create interesting patterns
            
            // Threshold is critical for spiral formation and reactions
            // Lower thresholds (1-2) tend to create more active, spreading patterns with more reactions
            // Higher thresholds (3+) create more stable, structured patterns
            if (random(100) < 85) {  // Increased from 70% to 85% to favor more dynamic patterns
                // 85% chance for threshold values that create good spirals and reactions
                threshold = random(100) < 70 ? 1 : 2;  // Favor threshold 1 more (70% vs 30%) for more reactions
            } else {
                // 15% chance for other thresholds
                threshold = random(1, 4);
            }
            
            // Range affects how far the influence spreads
            // Range 1 creates tighter, more detailed patterns
            // Range 2 creates larger, more sweeping patterns
            // Range 3 creates very large influence areas (new option)
            uint8_t rangeRoll = random(100);
            if (rangeRoll < 70) {
                range = 1;  // 70% chance for detailed patterns
            } else if (rangeRoll < 90) {
                range = 2;  // 20% chance for sweeping patterns
            } else {
                range = 3;  // 10% chance for very large influence areas (new option)
            }
            
            // Number of states affects the complexity and color variety
            // 8-16 states work well for spiral patterns
            // 3-6 states work well for rock-paper-scissors dynamics
            // 17-32 states create complex, colorful patterns
            uint8_t stateRoll = random(100);
            if (stateRoll < 40) {  // Reduced from 60% to 40%
                // 40% chance for spiral-friendly state counts
                numStates = random(8, 17);
            } else if (stateRoll < 65) {  // Increased from 20% to 25%
                // 25% chance for rock-paper-scissors dynamics
                numStates = random(3, 7);
            } else {  // Increased from 20% to 35%
                // 35% chance for high state counts (more colorful visuals)
                numStates = random(17, 33);
            }
            
            // Reallocate color palette if number of states changed
            delete[] colorPalette;
            colorPalette = new uint16_t[numStates];
            generateColorPalette();
            
            // Variable threshold can create interesting effects
            // but works best with specific state counts
            variableThreshold = (random(100) < 45) && (numStates >= 8);  // Increased from 30% to 45%
            
            // State skipping creates discontinuous patterns with more dramatic reactions
            // Works best with higher state counts
            if (numStates >= 8) {
                uint8_t skipRoll = random(100);
                if (skipRoll < 35) {  // Increased from 20% to 35%
                    // 35% chance to skip states with higher state counts
                    stateSkip = random(2, min(numStates / 3, 4) + 1);
                } else if (skipRoll < 45) {  // New 10% chance for larger skips
                    // 10% chance for larger state skips (creates more dramatic transitions)
                    stateSkip = random(min(numStates / 3, 4) + 1, min(numStates / 2, 8) + 1);
                } else {
                    stateSkip = 1; // 55% chance for normal sequential states
                }
            } else {
                stateSkip = 1; // Normal sequential states for low state counts
            }
        
        // Choose a random initialization pattern if not specified
        if (random(100) < 70) {
            // 70% chance to use the current pattern
            initWithPattern(initPattern);
        } else {
            // 30% chance to use a random pattern
            InitPattern randomPattern = static_cast<InitPattern>(random(5));
            initWithPattern(randomPattern);
        }
    }
    
    void update() override {
        // Calculate the next generation
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Get current state
                uint8_t currentState = cells[y * width + x];
                
                // Calculate next state (cyclically) with possible state skipping
                uint8_t nextState = (currentState + stateSkip) % numStates;
                
                // Determine threshold based on settings
                uint8_t currentThreshold;
                if (variableThreshold) {
                    // Variable threshold based on state
                    // Lower states have lower thresholds, higher states have higher thresholds
                    currentThreshold = 1 + (currentState * 3) / numStates;
                } else {
                    currentThreshold = threshold;
                }
                
                // Count neighbors in the next state within the specified range
                uint8_t neighbors = 0;
                for (int8_t dy = -range; dy <= range; dy++) {
                    for (int8_t dx = -range; dx <= range; dx++) {
                        // Skip the cell itself
                        if (dx == 0 && dy == 0) continue;
                        
                        // Calculate neighbor coordinates with wrapping
                        uint16_t nx = (x + dx + width) % width;
                        uint16_t ny = (y + dy + height) % height;
                        
                        // Count neighbors in the next state
                        if (cells[ny * width + nx] == nextState) {
                            neighbors++;
                        }
                    }
                }
                
                // Apply rule: change to next state if enough neighbors are in next state
                if (neighbors >= currentThreshold) {
                    nextCells[y * width + x] = nextState;
                } else {
                    nextCells[y * width + x] = currentState;
                }
            }
        }
        
        // Swap cell buffers
        uint8_t* temp = cells;
        cells = nextCells;
        nextCells = temp;
    }
    
    void render() override {
        // Draw all cells using our consistent mapping function
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Get cell state from logical coordinates
                uint8_t state = cells[y * width + x];
                
                // Draw using our common mapping function
                drawMappedPixel(x, y, colorPalette[state]);
            }
        }
        
        matrix->show();
    }
    
    // Set a specific preset configuration
    void setPreset(Preset preset) {
        // Store original values to check if we need to reallocate
        uint8_t oldNumStates = numStates;
        switch (preset) {
            case SPIRAL_WAVES:
                numStates = 8;
                threshold = 2;
                range = 1;
                colorScheme = 0; // Rainbow
                initPattern = CENTER_SEED;
                variableThreshold = false;
                stateSkip = 1;
                break;
                
            case ROCK_PAPER_SCISSORS:
                numStates = 3;
                threshold = 3;
                range = 1;
                colorScheme = 4; // RGB
                initPattern = RANDOM;
                variableThreshold = false;
                stateSkip = 1;
                break;
                
            case COMPLEX_SPIRALS:
                numStates = 16;
                threshold = 1;
                range = 1;
                colorScheme = 0; // Rainbow
                initPattern = QUADRANTS;
                variableThreshold = false;
                stateSkip = 1;
                break;
                
            case CRYSTAL_GROWTH:
                numStates = 6;
                threshold = 2;
                range = 2;
                colorScheme = 2; // Ocean
                initPattern = CENTER_SEED;
                variableThreshold = false;
                stateSkip = 1;
                break;
                
            case LABYRINTH:
                numStates = 4;
                threshold = 2;
                range = 1;
                colorScheme = 1; // Fire
                initPattern = RANDOM;
                variableThreshold = false;
                stateSkip = 1;
                break;
                
            case VARIABLE_THRESHOLD:
                numStates = 12;
                threshold = 2; // Base threshold
                range = 1;
                colorScheme = 0; // Rainbow
                initPattern = RANDOM;
                variableThreshold = true;
                stateSkip = 1;
                break;
                
            case HIGH_STATE_COUNT:
                numStates = random(24, 33); // 24-32 states
                threshold = 1;
                range = 1;
                colorScheme = 0; // Rainbow
                initPattern = CENTER_SEED;
                variableThreshold = false;
                stateSkip = 1;
                break;
                
            case SKIP_STATES:
                numStates = 16;
                threshold = 2;
                range = 1;
                colorScheme = 0; // Rainbow
                initPattern = RANDOM;
                variableThreshold = false;
                stateSkip = random(2, 5); // Skip 2-4 states
                break;
        }
        
        // Reallocate color palette only if number of states changed
        if (numStates != oldNumStates) {
            delete[] colorPalette;
            colorPalette = new uint16_t[numStates];
        }
        
        // Generate new color palette and initialize
        generateColorPalette();
        init();
    }
    
    // Set a specific initialization pattern
    void setInitPattern(InitPattern pattern) {
        initPattern = pattern;
        init();
    }
    
    // Set the number of states
    void setNumStates(uint8_t states) {
        if (states < 2) states = 2;
        if (states > 32) states = 32;
        
        numStates = states;
        
        // Reallocate color palette
        delete[] colorPalette;
        colorPalette = new uint16_t[numStates];
        
        // Generate new color palette and initialize
        generateColorPalette();
        init();
    }
    
    // Set the threshold for state change
    void setThreshold(uint8_t newThreshold) {
        threshold = newThreshold;
    }
    
    // Set the neighborhood range
    void setRange(uint8_t newRange) {
        if (newRange < 1) newRange = 1;
        if (newRange > 3) newRange = 3;
        range = newRange;
    }
    
    const char* getName() const override {
        static char name[64];
        
        // Add descriptive names for presets
        if (numStates == 8 && threshold == 2 && range == 1 && initPattern == CENTER_SEED && !variableThreshold && stateSkip == 1) {
            sprintf(name, "Spiral Waves (%d states)", numStates);
        } else if (numStates == 3 && threshold == 3 && range == 1 && !variableThreshold && stateSkip == 1) {
            sprintf(name, "Rock-Paper-Scissors");
        } else if (numStates == 16 && threshold == 1 && range == 1 && !variableThreshold && stateSkip == 1) {
            sprintf(name, "Complex Spirals (%d states)", numStates);
        } else if (numStates == 6 && threshold == 2 && range == 2 && !variableThreshold && stateSkip == 1) {
            sprintf(name, "Crystal Growth (%d states)", numStates);
        } else if (numStates == 4 && threshold == 2 && range == 1 && !variableThreshold && stateSkip == 1) {
            sprintf(name, "Labyrinth (%d states)", numStates);
        } else if (variableThreshold) {
            sprintf(name, "Variable Threshold CA (%d states)", numStates);
        } else if (stateSkip > 1) {
            sprintf(name, "State-Skipping CA (%d states, skip=%d)", numStates, stateSkip);
        } else if (numStates >= 24) {
            sprintf(name, "High-State CA (%d states)", numStates);
        } else {
            sprintf(name, "Cyclic Automaton (%d states, t=%d, r=%d)", 
                   numStates, threshold, range);
        }
        
        return name;
    }
    
private:
    uint8_t* cells;        // Current generation
    uint8_t* nextCells;    // Next generation
    uint8_t numStates;     // Number of states
    uint8_t threshold;     // Threshold for state change
    uint8_t range;         // Neighborhood range
    InitPattern initPattern; // Current initialization pattern
    uint8_t colorScheme;   // Current color scheme
    uint16_t* colorPalette; // Color palette for each state
    bool variableThreshold; // Whether to use variable threshold based on state
    uint8_t stateSkip;     // Number of states to skip in transitions (1 = normal)
    
    // Initialize with a specific pattern
    void initWithPattern(InitPattern pattern) {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Apply the specified pattern
        switch (pattern) {
            case RANDOM:
                // Random cells throughout
                for (uint16_t y = 0; y < height; y++) {
                    for (uint16_t x = 0; x < width; x++) {
                        cells[y * width + x] = random(numStates);
                    }
                }
                break;
                
            case CENTER_SEED:
                // Small random seed in the center
                {
                    uint16_t centerX = width / 2;
                    uint16_t centerY = height / 2;
                    uint16_t radius = min(width, height) / 6;
                    
                    for (int16_t y = -radius; y <= radius; y++) {
                        for (int16_t x = -radius; x <= radius; x++) {
                            if (x*x + y*y <= radius*radius) {
                                int16_t px = centerX + x;
                                int16_t py = centerY + y;
                                
                                if (px >= 0 && px < width && py >= 0 && py < height) {
                                    cells[py * width + px] = random(numStates);
                                }
                            }
                        }
                    }
                }
                break;
                
            case QUADRANTS:
                // Four quadrants with different states
                {
                    uint16_t halfWidth = width / 2;
                    uint16_t halfHeight = height / 2;
                    
                    // Top-left quadrant
                    uint8_t state1 = random(numStates);
                    for (uint16_t y = 0; y < halfHeight; y++) {
                        for (uint16_t x = 0; x < halfWidth; x++) {
                            cells[y * width + x] = state1;
                        }
                    }
                    
                    // Top-right quadrant
                    uint8_t state2 = (state1 + 1) % numStates;
                    for (uint16_t y = 0; y < halfHeight; y++) {
                        for (uint16_t x = halfWidth; x < width; x++) {
                            cells[y * width + x] = state2;
                        }
                    }
                    
                    // Bottom-left quadrant
                    uint8_t state3 = (state2 + 1) % numStates;
                    for (uint16_t y = halfHeight; y < height; y++) {
                        for (uint16_t x = 0; x < halfWidth; x++) {
                            cells[y * width + x] = state3;
                        }
                    }
                    
                    // Bottom-right quadrant
                    uint8_t state4 = (state3 + 1) % numStates;
                    for (uint16_t y = halfHeight; y < height; y++) {
                        for (uint16_t x = halfWidth; x < width; x++) {
                            cells[y * width + x] = state4;
                        }
                    }
                }
                break;
                
            case STRIPES:
                // Horizontal stripes
                {
                    uint8_t stripeHeight = max(1, height / (numStates * 2));
                    
                    for (uint16_t y = 0; y < height; y++) {
                        uint8_t state = (y / stripeHeight) % numStates;
                        for (uint16_t x = 0; x < width; x++) {
                            cells[y * width + x] = state;
                        }
                    }
                }
                break;
                
            case SPIRAL:
                // Spiral pattern
                {
                    uint16_t centerX = width / 2;
                    uint16_t centerY = height / 2;
                    float maxDist = sqrt(centerX*centerX + centerY*centerY);
                    
                    for (uint16_t y = 0; y < height; y++) {
                        for (uint16_t x = 0; x < width; x++) {
                            // Calculate distance from center
                            float dx = x - centerX;
                            float dy = y - centerY;
                            float dist = sqrt(dx*dx + dy*dy);
                            
                            // Calculate angle
                            float angle = atan2(dy, dx);
                            if (angle < 0) angle += 2 * PI;
                            
                            // Calculate state based on angle and distance
                            float spiralFactor = angle / (2 * PI) + dist / maxDist;
                            uint8_t state = (uint8_t)(spiralFactor * numStates) % numStates;
                            
                            cells[y * width + x] = state;
                        }
                    }
                }
                break;
        }
        
        // Save the current pattern
        initPattern = pattern;
    }
    
    // Generate a color palette
    void generateColorPalette() {
        switch (colorScheme) {
            case 0:
                // Rainbow spectrum
                for (uint8_t i = 0; i < numStates; i++) {
                    float hue = (float)i / numStates;
                    colorPalette[i] = hueToRGB565(hue);
                }
                break;
                
            case 1:
                // Fire color scheme (black to red to yellow to white)
                for (uint8_t i = 0; i < numStates; i++) {
                    float t = (float)i / (numStates - 1);
                    uint8_t r = 255 * min(1.0f, t * 4);
                    uint8_t g = 255 * min(1.0f, max(0.0f, (t - 0.25f) * 4));
                    uint8_t b = 255 * min(1.0f, max(0.0f, (t - 0.5f) * 4));
                    colorPalette[i] = matrix->color565(r, g, b);
                }
                break;
                
            case 2:
                // Ocean color scheme (deep blue to cyan to white)
                for (uint8_t i = 0; i < numStates; i++) {
                    float t = (float)i / (numStates - 1);
                    uint8_t r = 255 * min(1.0f, max(0.0f, (t - 0.5f) * 2));
                    uint8_t g = 255 * min(1.0f, t * 2);
                    uint8_t b = 255 * min(1.0f, 0.5f + t * 0.5f);
                    colorPalette[i] = matrix->color565(r, g, b);
                }
                break;
                
            case 3:
                // Grayscale
                for (uint8_t i = 0; i < numStates; i++) {
                    uint8_t v = 255 * i / (numStates - 1);
                    colorPalette[i] = matrix->color565(v, v, v);
                }
                break;
                
            case 4:
                // RGB (for Rock-Paper-Scissors)
                if (numStates == 3) {
                    colorPalette[0] = matrix->color565(255, 0, 0);    // Red (Rock)
                    colorPalette[1] = matrix->color565(0, 255, 0);    // Green (Paper)
                    colorPalette[2] = matrix->color565(0, 0, 255);    // Blue (Scissors)
                } else {
                    // Fall back to rainbow for other state counts
                    for (uint8_t i = 0; i < numStates; i++) {
                        float hue = (float)i / numStates;
                        colorPalette[i] = hueToRGB565(hue);
                    }
                }
                break;
                
            default:
                // Default to rainbow
                for (uint8_t i = 0; i < numStates; i++) {
                    float hue = (float)i / numStates;
                    colorPalette[i] = hueToRGB565(hue);
                }
                break;
        }
    }
    
    // Convert HSV to RGB565
    uint16_t hueToRGB565(float h) {
        // Convert hue (0-1) to RGB
        // Based on HSV with S=1, V=1
        h = fmod(h, 1.0f) * 6.0f;
        int i = (int)h;
        float f = h - i;
        
        uint8_t r, g, b;
        switch (i) {
            case 0: r = 255; g = 255 * f; b = 0; break;
            case 1: r = 255 * (1 - f); g = 255; b = 0; break;
            case 2: r = 0; g = 255; b = 255 * f; break;
            case 3: r = 0; g = 255 * (1 - f); b = 255; break;
            case 4: r = 255 * f; g = 0; b = 255; break;
            case 5: r = 255; g = 0; b = 255 * (1 - f); break;
            default: r = 0; g = 0; b = 0; break;
        }
        
        return matrix->color565(r, g, b);
    }
};


/**
 * Bubbling Lava
 * 
 * The bottom half of the world is a chaotic elementary cellular automaton.
 * The top half is the Game of Life with 2D trails.
 * As each generation of the ECA reaches the middle, it provides live cells
 * to the top half, which then starts spawning new life.
 */
class BubblingLava : public CellularAutomaton {
public:
    BubblingLava(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height) 
        : CellularAutomaton(matrix, width, height) {
        // Initialize the cells for both automata
        cells = new uint8_t[width * height];
        nextCells = new uint8_t[width * height];
        ecaRow = new uint8_t[width];
        
        // Set up colors - more vibrant colors for better visibility
        lavaColor = matrix->color565(255, 80, 0);    // Brighter orange-red for lava
        bgColor = matrix->color565(100, 0, 0);       // Dark maroon background for both halves
        
        // More distinct trail colors with better gradient
        trailColors[0] = matrix->color565(255, 255, 0);  // Bright yellow for live cells
        trailColors[1] = matrix->color565(255, 200, 0);  // Yellow-orange for recent trails
        trailColors[2] = matrix->color565(255, 150, 0);  // Orange for medium trails
        trailColors[3] = matrix->color565(255, 100, 0);  // Dark orange for older trails
        trailColors[4] = matrix->color565(255, 50, 0);   // Red-orange for oldest trails
        
        // Choose a chaotic rule for the ECA
        ecaRule = 30;  // Rule 30 is chaotic
        
        // Initialize state variables
        reachedMiddle = false;
        bubbleCounter = 0;
        lastBubbleTime = 0;
        lastPatternTime = 0;
    }
    
    ~BubblingLava() {
        delete[] cells;
        delete[] nextCells;
        delete[] ecaRow;
    }
    
    void init() override {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Fill the entire bottom half with a complex pattern for the ECA
        // Start already filled up to the middle boundary
        for (uint16_t y = height/2; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Create a more dense pattern throughout the bottom half
                cells[y * width + x] = (random(100) < 40) ? 1 : 0;
            }
        }
        
        // Add some random "hot spots" in the bottom half
        for (int i = 0; i < 15; i++) {  // Increased from 10 to 15
            uint16_t cx = random(width);
            uint16_t cy = height/2 + random(height/2);  // Only in bottom half
            uint16_t radius = random(3, 8);
            
            for (int16_t y = -radius; y <= radius; y++) {
                for (int16_t x = -radius; x <= radius; x++) {
                    if (x*x + y*y <= radius*radius) {
                        int16_t px = (cx + x + width) % width;
                        int16_t py = cy + y;
                        
                        if (py >= height/2 && py < height) {
                            cells[py * width + px] = 1;
                        }
                    }
                }
            }
        }
        
        // Initialize the ECA row at the middle boundary
        for (uint16_t x = 0; x < width; x++) {
            ecaRow[x] = cells[height/2 * width + x];
        }
        
        // Current ECA row starts at the middle boundary
        currentEcaRow = height/2;
        
        // Initialize the top half with stable Game of Life patterns
        initTopHalfWithStablePatterns();
        
        // Set state variables - already at middle boundary
        reachedMiddle = true;  // Start with bubbles already forming
        bubbleCounter = 0;
        lastBubbleTime = frameCount;
        lastPatternTime = frameCount;
    }
    
    void update() override {
        // Clear the next generation buffer
        memset(nextCells, 0, width * height * sizeof(uint8_t));
        
        // Update the ECA in the bottom half
        updateECA();
        
        // Update the Game of Life in the top half
        updateGameOfLife();
        
        // Continuously create bubbles from the lava to the top half
        createBubbles();
        
        // Periodically add new stable patterns to keep the top half active
        if (frameCount - lastPatternTime > 30) { // Every ~1 second (reduced from 60 to 30)
            // Add multiple patterns at once for more activity
            for (int i = 0; i < 2; i++) {  // Add 2 patterns each time
                addStablePattern();
            }
            lastPatternTime = frameCount;
        }
        
        // Swap cell buffers
        uint8_t* temp = cells;
        cells = nextCells;
        nextCells = temp;
    }
    
    void render() override {
        // Draw all cells
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                uint8_t state = cells[y * width + x];
                uint16_t color;
                
                if (y >= height / 2) {
                    // Bottom half - ECA (lava)
                    if (state > 0) {
                        color = lavaColor;  // Bright lava for active cells
                    } else {
                        color = bgColor;    // Dark maroon background for inactive cells
                    }
                } else {
                    // Top half - Game of Life with trails
                    if (state == 0) {
                        color = bgColor;  // Dark maroon background for top half too
                    } else if (state == 1) {
                        // Live cell
                        color = trailColors[0];  // Bright yellow
                    } else if (state <= 3) {
                        // Recent trail
                        color = trailColors[1];  // Yellow-orange
                    } else if (state <= 5) {
                        // Medium trail
                        color = trailColors[2];  // Orange
                    } else if (state <= 8) {
                        // Older trail
                        color = trailColors[3];  // Dark orange
                    } else {
                        // Oldest trail
                        color = trailColors[4];  // Red-orange
                    }
                }
                
                drawMappedPixel(x, y, color);
            }
        }
        
        matrix->show();
    }
    
    const char* getName() const override {
        return "Bubbling Lava";
    }
    
private:
    uint8_t* cells;       // Current generation
    uint8_t* nextCells;   // Next generation
    uint8_t* ecaRow;      // Current ECA row for computation
    uint16_t currentEcaRow; // Current row for ECA
    uint8_t ecaRule;      // Rule for the ECA
    
    uint16_t lavaColor;   // Color for active lava cells
    uint16_t bgColor;     // Background color for lava
    uint16_t trailColors[5]; // Colors for Game of Life trails (expanded to 5 colors)
    
    bool reachedMiddle;   // Flag to track if ECA has reached the middle
    uint16_t bubbleCounter; // Counter for bubble creation
    uint32_t lastBubbleTime; // Time of last bubble creation
    uint32_t lastPatternTime; // Time of last pattern addition
    
    // Update the Elementary Cellular Automaton in the bottom half
    void updateECA() {
        // Always calculate a new ECA row at the middle boundary
        // This ensures continuous evolution of the lava
        
        // Calculate the next ECA row at the middle boundary
        for (uint16_t x = 0; x < width; x++) {
            // Get the left, center, and right cells from the current row
            uint8_t left = cells[height/2 * width + ((x + width - 1) % width)];
            uint8_t center = cells[height/2 * width + x];
            uint8_t right = cells[height/2 * width + ((x + 1) % width)];
            
            // Compute the pattern index (0-7)
            uint8_t pattern = (left << 2) | (center << 1) | right;
            
            // Apply the rule
            ecaRow[x] = (ecaRule >> pattern) & 1;
        }
        
        // Apply the calculated row to the middle boundary
        for (uint16_t x = 0; x < width; x++) {
            nextCells[height/2 * width + x] = ecaRow[x];
        }
        
        // Update the rest of the bottom half with a cellular automaton-like behavior
        for (uint16_t y = height/2 + 1; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Count live neighbors
                uint8_t neighbors = 0;
                for (int8_t dy = -1; dy <= 1; dy++) {
                    for (int8_t dx = -1; dx <= 1; dx++) {
                        // Skip the cell itself
                        if (dx == 0 && dy == 0) continue;
                        
                        // Calculate neighbor coordinates with wrapping
                        uint16_t nx = (x + dx + width) % width;
                        uint16_t ny = (y + dy + height) % height;
                        
                        // Only count neighbors in the bottom half
                        if (ny >= height/2 && cells[ny * width + nx] > 0) {
                            neighbors++;
                        }
                    }
                }
                
                // Apply a lava-like cellular automaton rule
                if (cells[y * width + x] > 0) {
                    // Cell is alive - stays alive with 2-5 neighbors
                    nextCells[y * width + x] = (neighbors >= 2 && neighbors <= 5) ? 1 : 0;
                } else {
                    // Cell is dead - becomes alive with 3 neighbors or randomly
                    nextCells[y * width + x] = (neighbors == 3 || random(100) < 2) ? 1 : 0;
                }
            }
        }
        
        // Occasionally add new hot spots to keep the lava dynamic
        if (random(100) < 15) { // 15% chance each frame (increased from 10%)
            uint16_t cx = random(width);
            uint16_t cy = height/2 + random(height/2);  // Only in bottom half
            uint16_t radius = random(2, 6);
            
            for (int16_t y = -radius; y <= radius; y++) {
                for (int16_t x = -radius; x <= radius; x++) {
                    if (x*x + y*y <= radius*radius) {
                        int16_t px = (cx + x + width) % width;
                        int16_t py = cy + y;
                        
                        if (py >= height/2 && py < height) {
                            nextCells[py * width + px] = 1;
                        }
                    }
                }
            }
        }
    }
    
    // Initialize the top half with stable Game of Life patterns
    void initTopHalfWithStablePatterns() {
        // Add more stable patterns to the top half
        for (int i = 0; i < 15; i++) {  // Increased from 8 to 15
            // For most patterns, concentrate them near the boundary with the bottom ECA
            if (i < 10) {  // 10 out of 15 patterns (67%) will be near the boundary
                // Place patterns specifically near the boundary with the bottom ECA
                uint8_t patternType = random(10);  // Same pattern types as addStablePattern
                
                // Choose a random position near the boundary with the bottom ECA
                // This is the lower part of the middle section (closer to 2*height/3)
                uint16_t px = random(width - 12) + 6;  // Ensure more space around patterns
                uint16_t py = (height/2) + (height/6) - random(height/8);  // Position in the lower part of middle section
                
                // Now add the pattern at this position using the same logic as addStablePattern
                switch (patternType) {
                    case 0:
                        // Block (2x2 square) - stable
                        cells[py * width + px] = 1;
                        cells[py * width + (px+1)] = 1;
                        cells[(py+1) * width + px] = 1;
                        cells[(py+1) * width + (px+1)] = 1;
                        break;
                        
                    case 1:
                        // Blinker (3 cells in a row) - period 2 oscillator
                        cells[py * width + px] = 1;
                        cells[py * width + (px+1)] = 1;
                        cells[py * width + (px+2)] = 1;
                        break;
                        
                    case 2:
                        // Glider - moves diagonally
                        cells[py * width + px] = 1;
                        cells[py * width + (px+1)] = 1;
                        cells[py * width + (px+2)] = 1;
                        cells[(py+1) * width + px] = 0;
                        cells[(py+1) * width + (px+1)] = 0;
                        cells[(py+1) * width + (px+2)] = 1;
                        cells[(py+2) * width + px] = 1;
                        cells[(py+2) * width + (px+1)] = 0;
                        cells[(py+2) * width + (px+2)] = 0;
                        break;
                        
                    case 3:
                        // Beehive - stable
                        cells[py * width + (px+1)] = 1;
                        cells[py * width + (px+2)] = 1;
                        cells[(py+1) * width + px] = 1;
                        cells[(py+1) * width + (px+3)] = 1;
                        cells[(py+2) * width + (px+1)] = 1;
                        cells[(py+2) * width + (px+2)] = 1;
                        break;
                        
                    case 4:
                        // Toad - period 2 oscillator
                        cells[py * width + (px+1)] = 1;
                        cells[py * width + (px+2)] = 1;
                        cells[py * width + (px+3)] = 1;
                        cells[(py+1) * width + px] = 1;
                        cells[(py+1) * width + (px+1)] = 1;
                        cells[(py+1) * width + (px+2)] = 1;
                        break;
                        
                    case 5:
                        // Beacon - period 2 oscillator
                        cells[py * width + px] = 1;
                        cells[py * width + (px+1)] = 1;
                        cells[(py+1) * width + px] = 1;
                        cells[(py+1) * width + (px+1)] = 1;
                        cells[(py+2) * width + (px+2)] = 1;
                        cells[(py+2) * width + (px+3)] = 1;
                        cells[(py+3) * width + (px+2)] = 1;
                        cells[(py+3) * width + (px+3)] = 1;
                        break;
                        
                    case 6:
                        // Pulsar - period 3 oscillator (simplified version)
                        // Top row
                        cells[(py) * width + (px+2)] = 1;
                        cells[(py) * width + (px+3)] = 1;
                        cells[(py) * width + (px+4)] = 1;
                        cells[(py) * width + (px+8)] = 1;
                        cells[(py) * width + (px+9)] = 1;
                        cells[(py) * width + (px+10)] = 1;
                        
                        // 5 cells down
                        cells[(py+5) * width + (px+2)] = 1;
                        cells[(py+5) * width + (px+3)] = 1;
                        cells[(py+5) * width + (px+4)] = 1;
                        cells[(py+5) * width + (px+8)] = 1;
                        cells[(py+5) * width + (px+9)] = 1;
                        cells[(py+5) * width + (px+10)] = 1;
                        
                        // 7 cells down
                        cells[(py+7) * width + (px+2)] = 1;
                        cells[(py+7) * width + (px+3)] = 1;
                        cells[(py+7) * width + (px+4)] = 1;
                        cells[(py+7) * width + (px+8)] = 1;
                        cells[(py+7) * width + (px+9)] = 1;
                        cells[(py+7) * width + (px+10)] = 1;
                        
                        // 12 cells down
                        cells[(py+12) * width + (px+2)] = 1;
                        cells[(py+12) * width + (px+3)] = 1;
                        cells[(py+12) * width + (px+4)] = 1;
                        cells[(py+12) * width + (px+8)] = 1;
                        cells[(py+12) * width + (px+9)] = 1;
                        cells[(py+12) * width + (px+10)] = 1;
                        
                        // Left column
                        cells[(py+2) * width + (px)] = 1;
                        cells[(py+3) * width + (px)] = 1;
                        cells[(py+4) * width + (px)] = 1;
                        cells[(py+8) * width + (px)] = 1;
                        cells[(py+9) * width + (px)] = 1;
                        cells[(py+10) * width + (px)] = 1;
                        
                        // 5 cells right
                        cells[(py+2) * width + (px+5)] = 1;
                        cells[(py+3) * width + (px+5)] = 1;
                        cells[(py+4) * width + (px+5)] = 1;
                        cells[(py+8) * width + (px+5)] = 1;
                        cells[(py+9) * width + (px+5)] = 1;
                        cells[(py+10) * width + (px+5)] = 1;
                        
                        // 7 cells right
                        cells[(py+2) * width + (px+7)] = 1;
                        cells[(py+3) * width + (px+7)] = 1;
                        cells[(py+4) * width + (px+7)] = 1;
                        cells[(py+8) * width + (px+7)] = 1;
                        cells[(py+9) * width + (px+7)] = 1;
                        cells[(py+10) * width + (px+7)] = 1;
                        
                        // 12 cells right
                        cells[(py+2) * width + (px+12)] = 1;
                        cells[(py+3) * width + (px+12)] = 1;
                        cells[(py+4) * width + (px+12)] = 1;
                        cells[(py+8) * width + (px+12)] = 1;
                        cells[(py+9) * width + (px+12)] = 1;
                        cells[(py+10) * width + (px+12)] = 1;
                        break;
                        
                    case 7:
                        // Pentadecathlon - period 15 oscillator
                        for (int i = 0; i < 8; i++) {
                            cells[(py+1) * width + (px+i+1)] = 1;
                        }
                        cells[(py) * width + (px+3)] = 1;
                        cells[(py) * width + (px+6)] = 1;
                        cells[(py+2) * width + (px+3)] = 1;
                        cells[(py+2) * width + (px+6)] = 1;
                        break;
                        
                    case 8:
                        // Clock - period 2 oscillator
                        cells[py * width + (px+1)] = 1;
                        cells[py * width + (px+2)] = 1;
                        cells[(py+1) * width + px] = 1;
                        cells[(py+1) * width + (px+3)] = 1;
                        cells[(py+2) * width + px] = 1;
                        cells[(py+2) * width + (px+3)] = 1;
                        cells[(py+3) * width + (px+1)] = 1;
                        cells[(py+3) * width + (px+2)] = 1;
                        break;
                        
                    case 9:
                        // Multiple blinkers for more activity
                        // First blinker
                        cells[py * width + px] = 1;
                        cells[py * width + (px+1)] = 1;
                        cells[py * width + (px+2)] = 1;
                        
                        // Second blinker (vertical)
                        cells[(py+3) * width + (px+4)] = 1;
                        cells[(py+4) * width + (px+4)] = 1;
                        cells[(py+5) * width + (px+4)] = 1;
                        
                        // Third blinker
                        cells[(py+7) * width + (px)] = 1;
                        cells[(py+7) * width + (px+1)] = 1;
                        cells[(py+7) * width + (px+2)] = 1;
                        break;
                }
            } else {
                // Place the remaining patterns randomly in the top half
                addStablePattern();
            }
        }
    }
    
    // Add a stable Game of Life pattern at a random location in the top half
    void addStablePattern() {
        // Choose a random pattern type - now with more oscillator patterns
        uint8_t patternType = random(10);  // Increased from 5 to 10 pattern types
        
        // Choose a random position in the top half
        uint16_t px = random(width - 12) + 6;  // Ensure more space around patterns
        uint16_t py = random(height/2 - 12) + 6;
        
        switch (patternType) {
            case 0:
                // Block (2x2 square) - stable
                cells[py * width + px] = 1;
                cells[py * width + (px+1)] = 1;
                cells[(py+1) * width + px] = 1;
                cells[(py+1) * width + (px+1)] = 1;
                break;
                
            case 1:
                // Blinker (3 cells in a row) - period 2 oscillator
                cells[py * width + px] = 1;
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                break;
                
            case 2:
                // Glider - moves diagonally
                cells[py * width + px] = 1;
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                cells[(py+1) * width + px] = 0;
                cells[(py+1) * width + (px+1)] = 0;
                cells[(py+1) * width + (px+2)] = 1;
                cells[(py+2) * width + px] = 1;
                cells[(py+2) * width + (px+1)] = 0;
                cells[(py+2) * width + (px+2)] = 0;
                break;
                
            case 3:
                // Beehive - stable
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                cells[(py+1) * width + px] = 1;
                cells[(py+1) * width + (px+3)] = 1;
                cells[(py+2) * width + (px+1)] = 1;
                cells[(py+2) * width + (px+2)] = 1;
                break;
                
            case 4:
                // Toad - period 2 oscillator
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                cells[py * width + (px+3)] = 1;
                cells[(py+1) * width + px] = 1;
                cells[(py+1) * width + (px+1)] = 1;
                cells[(py+1) * width + (px+2)] = 1;
                break;
                
            case 5:
                // Beacon - period 2 oscillator
                cells[py * width + px] = 1;
                cells[py * width + (px+1)] = 1;
                cells[(py+1) * width + px] = 1;
                cells[(py+1) * width + (px+1)] = 1;
                cells[(py+2) * width + (px+2)] = 1;
                cells[(py+2) * width + (px+3)] = 1;
                cells[(py+3) * width + (px+2)] = 1;
                cells[(py+3) * width + (px+3)] = 1;
                break;
                
            case 6:
                // Pulsar - period 3 oscillator (simplified version)
                // Top row
                cells[(py) * width + (px+2)] = 1;
                cells[(py) * width + (px+3)] = 1;
                cells[(py) * width + (px+4)] = 1;
                cells[(py) * width + (px+8)] = 1;
                cells[(py) * width + (px+9)] = 1;
                cells[(py) * width + (px+10)] = 1;
                
                // 5 cells down
                cells[(py+5) * width + (px+2)] = 1;
                cells[(py+5) * width + (px+3)] = 1;
                cells[(py+5) * width + (px+4)] = 1;
                cells[(py+5) * width + (px+8)] = 1;
                cells[(py+5) * width + (px+9)] = 1;
                cells[(py+5) * width + (px+10)] = 1;
                
                // 7 cells down
                cells[(py+7) * width + (px+2)] = 1;
                cells[(py+7) * width + (px+3)] = 1;
                cells[(py+7) * width + (px+4)] = 1;
                cells[(py+7) * width + (px+8)] = 1;
                cells[(py+7) * width + (px+9)] = 1;
                cells[(py+7) * width + (px+10)] = 1;
                
                // 12 cells down
                cells[(py+12) * width + (px+2)] = 1;
                cells[(py+12) * width + (px+3)] = 1;
                cells[(py+12) * width + (px+4)] = 1;
                cells[(py+12) * width + (px+8)] = 1;
                cells[(py+12) * width + (px+9)] = 1;
                cells[(py+12) * width + (px+10)] = 1;
                
                // Left column
                cells[(py+2) * width + (px)] = 1;
                cells[(py+3) * width + (px)] = 1;
                cells[(py+4) * width + (px)] = 1;
                cells[(py+8) * width + (px)] = 1;
                cells[(py+9) * width + (px)] = 1;
                cells[(py+10) * width + (px)] = 1;
                
                // 5 cells right
                cells[(py+2) * width + (px+5)] = 1;
                cells[(py+3) * width + (px+5)] = 1;
                cells[(py+4) * width + (px+5)] = 1;
                cells[(py+8) * width + (px+5)] = 1;
                cells[(py+9) * width + (px+5)] = 1;
                cells[(py+10) * width + (px+5)] = 1;
                
                // 7 cells right
                cells[(py+2) * width + (px+7)] = 1;
                cells[(py+3) * width + (px+7)] = 1;
                cells[(py+4) * width + (px+7)] = 1;
                cells[(py+8) * width + (px+7)] = 1;
                cells[(py+9) * width + (px+7)] = 1;
                cells[(py+10) * width + (px+7)] = 1;
                
                // 12 cells right
                cells[(py+2) * width + (px+12)] = 1;
                cells[(py+3) * width + (px+12)] = 1;
                cells[(py+4) * width + (px+12)] = 1;
                cells[(py+8) * width + (px+12)] = 1;
                cells[(py+9) * width + (px+12)] = 1;
                cells[(py+10) * width + (px+12)] = 1;
                break;
                
            case 7:
                // Pentadecathlon - period 15 oscillator
                for (int i = 0; i < 8; i++) {
                    cells[(py+1) * width + (px+i+1)] = 1;
                }
                cells[(py) * width + (px+3)] = 1;
                cells[(py) * width + (px+6)] = 1;
                cells[(py+2) * width + (px+3)] = 1;
                cells[(py+2) * width + (px+6)] = 1;
                break;
                
            case 8:
                // Clock - period 2 oscillator
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                cells[(py+1) * width + px] = 1;
                cells[(py+1) * width + (px+3)] = 1;
                cells[(py+2) * width + px] = 1;
                cells[(py+2) * width + (px+3)] = 1;
                cells[(py+3) * width + (px+1)] = 1;
                cells[(py+3) * width + (px+2)] = 1;
                break;
                
            case 9:
                // Multiple blinkers for more activity
                // First blinker
                cells[py * width + px] = 1;
                cells[py * width + (px+1)] = 1;
                cells[py * width + (px+2)] = 1;
                
                // Second blinker (vertical)
                cells[(py+3) * width + (px+4)] = 1;
                cells[(py+4) * width + (px+4)] = 1;
                cells[(py+5) * width + (px+4)] = 1;
                
                // Third blinker
                cells[(py+7) * width + (px)] = 1;
                cells[(py+7) * width + (px+1)] = 1;
                cells[(py+7) * width + (px+2)] = 1;
                break;
        }
    }
    
    // Update the Game of Life in the top half
    void updateGameOfLife() {
        // First, age all cells in the top half (for trails)
        for (uint16_t y = 0; y < height / 2; y++) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[y * width + x] > 0) {
                    // Age the cell (increase the value)
                    nextCells[y * width + x] = cells[y * width + x] + 1;
                    
                    // If it's too old, remove it
                    if (nextCells[y * width + x] > 12) {  // Longer trails for better visibility
                        nextCells[y * width + x] = 0;
                    }
                }
            }
        }
        
        // Then apply Game of Life rules to create new live cells
        for (uint16_t y = 0; y < height / 2; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Count live neighbors (cells with state 1)
                uint8_t neighbors = 0;
                for (int8_t dy = -1; dy <= 1; dy++) {
                    for (int8_t dx = -1; dx <= 1; dx++) {
                        // Skip the cell itself
                        if (dx == 0 && dy == 0) continue;
                        
                        // Calculate neighbor coordinates with wrapping
                        uint16_t nx = (x + dx + width) % width;
                        uint16_t ny = (y + dy + height / 2) % (height / 2);
                        
                        // Count only live cells (state 1)
                        if (cells[ny * width + nx] == 1) {
                            neighbors++;
                        }
                    }
                }
                
                // Apply Conway's Game of Life rules
                if (cells[y * width + x] == 1) {
                    // Cell is alive
                    if (neighbors < 2 || neighbors > 3) {
                        // Cell dies - but leave the trail
                        nextCells[y * width + x] = 2;  // Start trail
                    }
                    // If the cell survives, we already set it to 1 above
                } else if (cells[y * width + x] == 0) {
                    // Cell is dead
                    if (neighbors == 3) {
                        // Cell becomes alive
                        nextCells[y * width + x] = 1;
                    }
                }
            }
        }
    }
    
    // Create bubbles from the lava to the top half
    void createBubbles() {
        // Only create bubbles if we've reached the middle
        if (!reachedMiddle) return;
        
        // Create bubbles at regular intervals
        if (frameCount - lastBubbleTime > 5) { // Every 5 frames
            lastBubbleTime = frameCount;
            
            // Find active cells at the boundary
            for (uint16_t x = 0; x < width; x++) {
                if (cells[height / 2 * width + x] == 1) {
                    // 40% chance to create a bubble from each active cell (increased from 30%)
                    if (random(100) < 40) {
                        // Create a bubble that rises up
                        createBubbleColumn(x);
                    }
                }
            }
            
            // Occasionally create random bubbles
            if (random(100) < 25) { // 25% chance each bubble cycle (increased from 20%)
                createBubbleColumn(random(width));
            }
        }
    }
    
    // Create a column of bubbles rising from a specific x position
    void createBubbleColumn(uint16_t x) {
        // Choose a pattern type for the bubble
        uint8_t patternType = random(3);
        
        // Calculate y position - start near the middle and go up
        uint16_t y = height / 2 - 1 - random(3);
        
        // Make sure we're still in the top half
        if (y < height / 2) {
            switch (patternType) {
                case 0:
                    // Single bubble
                    nextCells[y * width + x] = 1;
                    // Add some neighboring cells for stability
                    if (random(100) < 70) {
                        int8_t dx = random(-1, 2); // -1, 0, or 1
                        uint16_t nx = (x + dx + width) % width;
                        nextCells[y * width + nx] = 1;
                    }
                    break;
                    
                case 1:
                    // Small cluster (more stable)
                    nextCells[y * width + x] = 1;
                    nextCells[y * width + ((x + 1) % width)] = 1;
                    nextCells[((y + 1) % (height/2)) * width + x] = 1;
                    nextCells[((y + 1) % (height/2)) * width + ((x + 1) % width)] = 1;
                    break;
                    
                case 2:
                    // Blinker (oscillator)
                    nextCells[y * width + x] = 1;
                    nextCells[y * width + ((x + 1) % width)] = 1;
                    nextCells[y * width + ((x + 2) % width)] = 1;
                    break;
            }
        }
    }
};

/**
 * Order and Chaos
 * 
 * The top and bottom of the world are generated with 2 different ECAs.
 * The middle of the world follows the rules of the Game of Life.
 * The top ECA generates symmetric patterns, and the bottom ECA generates chaotic patterns.
 * As the two colors collide, they form a third color.
 */
class OrderAndChaos : public CellularAutomaton {
public:
    OrderAndChaos(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height) 
        : CellularAutomaton(matrix, width, height) {
        // Initialize the cells
        cells = new uint8_t[width * height];
        nextCells = new uint8_t[width * height];
        
        // Set up the ECA rules
        topRule = 90;    // Rule 90 creates symmetric patterns (Sierpinski triangle)
        bottomRule = 30; // Rule 30 creates chaotic patterns
        
        // Set up colors - more vibrant colors for better visibility
        topColor = matrix->color565(0, 150, 255);    // Brighter blue for order
        topBgColor = matrix->color565(0, 0, 100);    // Dark blue background for order
        bottomColor = matrix->color565(255, 100, 0); // Orange-red for chaos
        bottomBgColor = matrix->color565(100, 0, 0); // Dark red background for chaos
        middleColor = matrix->color565(255, 0, 255); // Magenta for collision
        neutralColor = matrix->color565(200, 200, 200); // Light gray for neutral cells
        
        // Initialize the current rows for ECAs
        topCurrentRow = 0;
        bottomCurrentRow = height - 1;
        
        // Initialize state variables
        topReachedBoundary = false;
        bottomReachedBoundary = false;
        lastCollisionCheck = 0;
    }
    
    ~OrderAndChaos() {
        delete[] cells;
        delete[] nextCells;
    }
    
    void init() override {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Initialize the top row with a random but continuous pattern
        // Create a smooth, continuous pattern with some randomness
        uint8_t prevState = random(2); // Start with either 0 or 1
        for (uint16_t x = 0; x < width; x++) {
            // 70% chance to keep the same state, 30% chance to change
            if (random(100) < 30) {
                prevState = 1 - prevState; // Flip the state
            }
            cells[x] = prevState;
            
            // Mark as coming from top
            if (cells[x] > 0) {
                cellOrigins[x] = 1;
            }
        }
        
        // Initialize the bottom row with a different random but continuous pattern
        // Use a different approach for variety
        prevState = random(2);
        uint8_t runLength = random(3, 8); // Run length of 3-7 cells
        uint8_t currentRun = 0;
        
        for (uint16_t x = 0; x < width; x++) {
            if (currentRun >= runLength) {
                prevState = 1 - prevState; // Flip the state
                runLength = random(3, 8); // New run length
                currentRun = 0;
            }
            
            cells[(height - 1) * width + x] = prevState;
            
            // Mark as coming from bottom
            if (cells[(height - 1) * width + x] > 0) {
                cellOrigins[(height - 1) * width + x] = 2;
            }
            
            currentRun++;
        }
        
        // Add some random cells in the middle section to start the action
        for (uint16_t y = height / 3; y < 2 * height / 3; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Sparse random cells (about 5%)
                cells[y * width + x] = (random(100) < 5) ? 1 : 0;
            }
        }
        
        // Reset the current rows
        topCurrentRow = 0;
        bottomCurrentRow = height - 1;
        
        // Reset state variables
        topReachedBoundary = false;
        bottomReachedBoundary = false;
        lastCollisionCheck = frameCount;
        
        // Store cell origins for collision detection
        memset(cellOrigins, 0, width * height * sizeof(uint8_t));
        
        // Re-mark the origins for the top and bottom rows
        for (uint16_t x = 0; x < width; x++) {
            if (cells[x] > 0) {
                cellOrigins[x] = 1; // From top
            }
            if (cells[(height - 1) * width + x] > 0) {
                cellOrigins[(height - 1) * width + x] = 2; // From bottom
            }
        }
    }
    
    void update() override {
        // Clear the next generation buffer
        memset(nextCells, 0, width * height * sizeof(uint8_t));
        
        // Update the top ECA (order)
        updateTopECA();
        
        // Update the bottom ECA (chaos)
        updateBottomECA();
        
        // Update the Game of Life in the middle
        updateGameOfLife();
        
        // Check for collisions periodically
        if (frameCount - lastCollisionCheck > 5) {
            checkCollisions();
            lastCollisionCheck = frameCount;
        }
        
        // Swap cell buffers
        uint8_t* temp = cells;
        cells = nextCells;
        nextCells = temp;
    }
    
    void render() override {
        // Draw all cells
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                uint16_t color;
                uint8_t state = cells[y * width + x];
                uint8_t origin = cellOrigins[y * width + x];
                
                if (y < height / 3) {
                    // Top third - Order (blue)
                    if (state > 0) {
                        color = topColor;  // Bright blue for active cells
                    } else {
                        color = topBgColor;  // Dark blue background
                    }
                } else if (y >= 2 * height / 3) {
                    // Bottom third - Chaos (orange-red)
                    if (state > 0) {
                        color = bottomColor;  // Orange-red for active cells
                    } else {
                        color = bottomBgColor;  // Dark red background
                    }
                } else {
                    // Middle third - Game of Life
                    if (state == 0) {
                        color = 0;  // Black for dead cells
                    } else {
                        // Use color based on cell origin
                        if (origin == 1) {
                            // From top (order)
                            color = topColor;  // Blue
                        } else if (origin == 2) {
                            // From bottom (chaos)
                            color = bottomColor;  // Orange-red
                        } else if (origin == 3) {
                            // Collision point
                            color = middleColor;  // Magenta
                        } else {
                            // Default middle color
                            color = neutralColor;  // Light gray
                        }
                    }
                }
                
                drawMappedPixel(x, y, color);
            }
        }
        
        matrix->show();
    }
    
    const char* getName() const override {
        return "Order and Chaos";
    }
    
private:
    uint8_t* cells;       // Current generation
    uint8_t* nextCells;   // Next generation
    uint8_t topRule;      // Rule for the top ECA (order)
    uint8_t bottomRule;   // Rule for the bottom ECA (chaos)
    uint16_t topCurrentRow;    // Current row for top ECA
    uint16_t bottomCurrentRow; // Current row for bottom ECA
    
    uint16_t topColor;    // Color for top ECA (order) active cells
    uint16_t topBgColor;  // Background color for top ECA
    uint16_t bottomColor; // Color for bottom ECA (chaos) active cells
    uint16_t bottomBgColor; // Background color for bottom ECA
    uint16_t middleColor; // Color for collision points
    uint16_t neutralColor; // Color for neutral cells in middle
    
    bool topReachedBoundary;    // Flag to track if top ECA reached boundary
    bool bottomReachedBoundary; // Flag to track if bottom ECA reached boundary
    uint32_t lastCollisionCheck; // Time of last collision check
    
    // Cell origins for tracking where cells came from:
    // 0 = neutral/original
    // 1 = from top (order)
    // 2 = from bottom (chaos)
    // 3 = collision point
    uint8_t cellOrigins[128*128]; // Assuming max size, will only use width*height
    
    // Update the top ECA (order)
    void updateTopECA() {
        // Move the ECA down one row if not at the boundary
        if (topCurrentRow < height / 3 - 1) {
            topCurrentRow++;
            
            // Calculate the next ECA row
            for (uint16_t x = 0; x < width; x++) {
                // Get the left, center, and right cells from the row above
                uint8_t left = cells[(topCurrentRow - 1) * width + ((x + width - 1) % width)];
                uint8_t center = cells[(topCurrentRow - 1) * width + x];
                uint8_t right = cells[(topCurrentRow - 1) * width + ((x + 1) % width)];
                
                // Compute the pattern index (0-7)
                uint8_t pattern = (left << 2) | (center << 1) | right;
                
                // Apply the rule
                nextCells[topCurrentRow * width + x] = (topRule >> pattern) & 1;
                
                // Mark this cell as coming from the top
                if (nextCells[topCurrentRow * width + x] > 0) {
                    cellOrigins[topCurrentRow * width + x] = 1;
                }
            }
            
            // Copy the previous rows of the top ECA
            for (uint16_t y = 0; y < topCurrentRow; y++) {
                for (uint16_t x = 0; x < width; x++) {
                    nextCells[y * width + x] = cells[y * width + x];
                    
                    // Preserve cell origins
                    if (nextCells[y * width + x] > 0) {
                        cellOrigins[y * width + x] = 1;
                    }
                }
            }
            
            // Check if we've reached the boundary
            if (topCurrentRow == height / 3 - 1) {
                topReachedBoundary = true;
            }
        } else {
            // At the boundary - just copy the top ECA part
            for (uint16_t y = 0; y < height / 3; y++) {
                for (uint16_t x = 0; x < width; x++) {
                    nextCells[y * width + x] = cells[y * width + x];
                    
                    // Preserve cell origins
                    if (nextCells[y * width + x] > 0) {
                        cellOrigins[y * width + x] = 1;
                    }
                }
            }
            
            // Keep the top ECA moving by shifting all rows down
            for (uint16_t y = height / 3 - 1; y > 0; y--) {
                for (uint16_t x = 0; x < width; x++) {
                    nextCells[y * width + x] = cells[(y - 1) * width + x];
                    
                    // Preserve cell origins
                    if (nextCells[y * width + x] > 0) {
                        cellOrigins[y * width + x] = 1;
                    }
                }
            }
            
            // Generate a new top row with a random but continuous pattern
            uint8_t prevState = random(2); // Start with either 0 or 1
            for (uint16_t x = 0; x < width; x++) {
                // 70% chance to keep the same state, 30% chance to change
                if (random(100) < 30) {
                    prevState = 1 - prevState; // Flip the state
                }
                nextCells[x] = prevState;
                
                // Mark as coming from top
                if (nextCells[x] > 0) {
                    cellOrigins[x] = 1;
                }
            }
            
            // Maintain the boundary state
            topReachedBoundary = true;
        }
    }
    
    // Update the bottom ECA (chaos)
    void updateBottomECA() {
        // Move the ECA up one row if not at the boundary
        if (bottomCurrentRow > 2 * height / 3) {
            bottomCurrentRow--;
            
            // Calculate the next ECA row
            for (uint16_t x = 0; x < width; x++) {
                // Get the left, center, and right cells from the row below
                uint8_t left = cells[(bottomCurrentRow + 1) * width + ((x + width - 1) % width)];
                uint8_t center = cells[(bottomCurrentRow + 1) * width + x];
                uint8_t right = cells[(bottomCurrentRow + 1) * width + ((x + 1) % width)];
                
                // Compute the pattern index (0-7)
                uint8_t pattern = (left << 2) | (center << 1) | right;
                
                // Apply the rule
                nextCells[bottomCurrentRow * width + x] = (bottomRule >> pattern) & 1;
                
                // Mark this cell as coming from the bottom
                if (nextCells[bottomCurrentRow * width + x] > 0) {
                    cellOrigins[bottomCurrentRow * width + x] = 2;
                }
            }
            
            // Copy the previous rows of the bottom ECA
            for (uint16_t y = bottomCurrentRow + 1; y < height; y++) {
                for (uint16_t x = 0; x < width; x++) {
                    nextCells[y * width + x] = cells[y * width + x];
                    
                    // Preserve cell origins
                    if (nextCells[y * width + x] > 0) {
                        cellOrigins[y * width + x] = 2;
                    }
                }
            }
            
            // Check if we've reached the boundary
            if (bottomCurrentRow == 2 * height / 3) {
                bottomReachedBoundary = true;
            }
        } else {
            // At the boundary - just copy the bottom ECA part
            for (uint16_t y = 2 * height / 3; y < height; y++) {
                for (uint16_t x = 0; x < width; x++) {
                    nextCells[y * width + x] = cells[y * width + x];
                    
                    // Preserve cell origins
                    if (nextCells[y * width + x] > 0) {
                        cellOrigins[y * width + x] = 2;
                    }
                }
            }
            
            // Keep the bottom ECA moving by shifting all rows up
            for (uint16_t y = 2 * height / 3; y < height - 1; y++) {
                for (uint16_t x = 0; x < width; x++) {
                    nextCells[y * width + x] = cells[(y + 1) * width + x];
                    
                    // Preserve cell origins
                    if (nextCells[y * width + x] > 0) {
                        cellOrigins[y * width + x] = 2;
                    }
                }
            }
            
            // Generate a new bottom row with a random but continuous pattern
            // Use a run-length encoding approach for variety
            uint8_t prevState = random(2);
            uint8_t runLength = random(3, 8); // Run length of 3-7 cells
            uint8_t currentRun = 0;
            
            for (uint16_t x = 0; x < width; x++) {
                if (currentRun >= runLength) {
                    prevState = 1 - prevState; // Flip the state
                    runLength = random(3, 8); // New run length
                    currentRun = 0;
                }
                
                nextCells[(height - 1) * width + x] = prevState;
                
                // Mark as coming from bottom
                if (nextCells[(height - 1) * width + x] > 0) {
                    cellOrigins[(height - 1) * width + x] = 2;
                }
                
                currentRun++;
            }
            
            // Maintain the boundary state
            bottomReachedBoundary = true;
        }
    }
    
    // Update the Game of Life in the middle
    void updateGameOfLife() {
        // Apply Game of Life rules to the middle section
        for (uint16_t y = height / 3; y < 2 * height / 3; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Count live neighbors
                uint8_t neighbors = 0;
                uint8_t topNeighbors = 0;
                uint8_t bottomNeighbors = 0;
                
                for (int8_t dy = -1; dy <= 1; dy++) {
                    for (int8_t dx = -1; dx <= 1; dx++) {
                        // Skip the cell itself
                        if (dx == 0 && dy == 0) continue;
                        
                        // Calculate neighbor coordinates with wrapping
                        uint16_t nx = (x + dx + width) % width;
                        uint16_t ny = (y + dy + height) % height;
                        
                        // Count only if the neighbor is in the middle section
                        if (ny >= height / 3 && ny < 2 * height / 3 && cells[ny * width + nx] > 0) {
                            neighbors++;
                            
                            // Count neighbors by origin
                            if (cellOrigins[ny * width + nx] == 1) {
                                topNeighbors++;
                            } else if (cellOrigins[ny * width + nx] == 2) {
                                bottomNeighbors++;
                            }
                        }
                    }
                }
                
                // Apply Conway's Game of Life rules
                if (cells[y * width + x] > 0) {
                    // Cell is alive
                    if (neighbors == 2 || neighbors == 3) {
                        // Cell survives
                        nextCells[y * width + x] = 1;
                        
                        // Determine cell origin based on neighbors
                        if (topNeighbors > bottomNeighbors) {
                            cellOrigins[y * width + x] = 1; // From top
                        } else if (bottomNeighbors > topNeighbors) {
                            cellOrigins[y * width + x] = 2; // From bottom
                        }
                        // If equal, keep current origin
                    } else {
                        // Cell dies
                        nextCells[y * width + x] = 0;
                    }
                } else {
                    // Cell is dead
                    if (neighbors == 3) {
                        // Cell becomes alive
                        nextCells[y * width + x] = 1;
                        
                        // Determine cell origin based on neighbors
                        if (topNeighbors > bottomNeighbors) {
                            cellOrigins[y * width + x] = 1; // From top
                        } else if (bottomNeighbors > topNeighbors) {
                            cellOrigins[y * width + x] = 2; // From bottom
                        } else {
                            cellOrigins[y * width + x] = 0; // Neutral
                        }
                    }
                }
            }
        }
        
        // Seed the Game of Life from the ECAs at the boundaries
        // From top ECA
        if (topReachedBoundary) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[(height / 3 - 1) * width + x] > 0) {
                    // Create a live cell in the Game of Life
                    nextCells[height / 3 * width + x] = 1;
                    cellOrigins[height / 3 * width + x] = 1; // From top
                    
                    // Also create some neighboring cells for more activity
                    if (random(100) < 30) {
                        int8_t dx = random(-1, 2);  // -1, 0, or 1
                        uint16_t nx = (x + dx + width) % width;
                        nextCells[height / 3 * width + nx] = 1;
                        cellOrigins[height / 3 * width + nx] = 1; // From top
                    }
                }
            }
        }
        
        // From bottom ECA
        if (bottomReachedBoundary) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[(2 * height / 3) * width + x] > 0) {
                    // Create a live cell in the Game of Life
                    nextCells[(2 * height / 3 - 1) * width + x] = 1;
                    cellOrigins[(2 * height / 3 - 1) * width + x] = 2; // From bottom
                    
                    // Also create some neighboring cells for more activity
                    if (random(100) < 30) {
                        int8_t dx = random(-1, 2);  // -1, 0, or 1
                        uint16_t nx = (x + dx + width) % width;
                        nextCells[(2 * height / 3 - 1) * width + nx] = 1;
                        cellOrigins[(2 * height / 3 - 1) * width + nx] = 2; // From bottom
                    }
                }
            }
        }
    }
    
    // Check for collisions between cells from top and bottom
    void checkCollisions() {
        for (uint16_t y = height / 3; y < 2 * height / 3; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Skip dead cells
                if (cells[y * width + x] == 0) continue;
                
                // Check if this cell has neighbors from both top and bottom
                bool hasTopNeighbor = false;
                bool hasBottomNeighbor = false;
                
                for (int8_t dy = -1; dy <= 1; dy++) {
                    for (int8_t dx = -1; dx <= 1; dx++) {
                        // Skip the cell itself
                        if (dx == 0 && dy == 0) continue;
                        
                        // Calculate neighbor coordinates with wrapping
                        uint16_t nx = (x + dx + width) % width;
                        uint16_t ny = (y + dy + height) % height;
                        
                        // Only check neighbors in the middle section
                        if (ny >= height / 3 && ny < 2 * height / 3 && cells[ny * width + nx] > 0) {
                            if (cellOrigins[ny * width + nx] == 1) {
                                hasTopNeighbor = true;
                            } else if (cellOrigins[ny * width + nx] == 2) {
                                hasBottomNeighbor = true;
                            }
                        }
                    }
                }
                
                // If this cell has neighbors from both top and bottom, mark it as a collision point
                if (hasTopNeighbor && hasBottomNeighbor) {
                    cellOrigins[y * width + x] = 3; // Collision point
                }
            }
        }
    }
};

/**
 * Factory function to create a random automaton
 */

/**
 * Factory function to create a random automaton
 */
CellularAutomaton* createRandomAutomaton(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height) {
    uint8_t type = random(NUM_AUTOMATA);
    
    switch (type) {
        case 0: {
            ElementaryAutomaton* automaton = new ElementaryAutomaton(matrix, width, height);
            automaton->randomRule();
            return automaton;
        }
        case 1:
            return new GameOfLife(matrix, width, height);
        case 2:
            return new BriansBrain(matrix, width, height);
        case 3: {
            uint8_t antCount = random(7, 13);  // 7-12 ants
            return new LangtonsAnt(matrix, width, height, antCount);
        }
        case 4: {
            CyclicAutomaton* automaton = new CyclicAutomaton(matrix, width, height);
            
            // Randomize the cyclic automaton parameters for more variety
            if (random(100) < 60) {
                // 60% chance to use a preset known to create interesting patterns
                CyclicAutomaton::Preset preset;
                
                // Choose from presets that create the most interesting visual effects
                uint8_t presetChoice = random(100);
                if (presetChoice < 35) {
                    // 35% chance for spiral waves - the most visually appealing
                    preset = CyclicAutomaton::SPIRAL_WAVES;
                } else if (presetChoice < 60) {
                    // 25% chance for complex spirals - also very interesting
                    preset = CyclicAutomaton::COMPLEX_SPIRALS;
                } else if (presetChoice < 75) {
                    // 15% chance for rock-paper-scissors - creates dynamic patterns
                    preset = CyclicAutomaton::ROCK_PAPER_SCISSORS;
                } else if (presetChoice < 80) {
                    // 5% chance for crystal growth - creates expanding patterns
                    preset = CyclicAutomaton::CRYSTAL_GROWTH;
                } else if (presetChoice < 85) {
                    // 5% chance for labyrinth - creates maze-like patterns
                    preset = CyclicAutomaton::LABYRINTH;
                } else if (presetChoice < 95) {
                    // 10% chance for variable threshold - creates unpredictable patterns
                    preset = CyclicAutomaton::VARIABLE_THRESHOLD;
                } else {
                    // 5% chance for state skipping - creates discontinuous transitions
                    preset = CyclicAutomaton::SKIP_STATES;
                }
                
                automaton->setPreset(preset);
            } else {
                // 40% chance for custom parameters optimized for interesting patterns
                
                // Choose a pattern type
                uint8_t patternType = random(100);
                
                if (patternType < 50) {
                    // 50% chance for spiral-generating parameters
                    uint8_t states = random(8, 17);  // 8-16 states work well for spirals
                    uint8_t thresh = random(100) < 70 ? 1 : 2;  // Threshold 1-2 works best for spirals
                    uint8_t rng = 1;  // Range 1 for tight spirals
                    
                    automaton->setNumStates(states);
                    automaton->setThreshold(thresh);
                    automaton->setRange(rng);
                    
                    // Set a center seed initialization for better spiral formation
                    automaton->setInitPattern(CyclicAutomaton::CENTER_SEED);
                    
                } else if (patternType < 75) {
                    // 25% chance for rock-paper-scissors dynamics
                    uint8_t states = random(3, 7);  // 3-6 states
                    uint8_t thresh = random(2, 4);  // Higher threshold (2-3)
                    uint8_t rng = 1;  // Range 1
                    
                    automaton->setNumStates(states);
                    automaton->setThreshold(thresh);
                    automaton->setRange(rng);
                    
                    // Random initialization works best for RPS dynamics
                    automaton->setInitPattern(CyclicAutomaton::RANDOM);
                    
                } else {
                    // 25% chance for crystal/labyrinth patterns
                    uint8_t states = random(4, 9);  // 4-8 states
                    uint8_t thresh = 2;  // Threshold 2 works well for these
                    uint8_t rng = random(100) < 70 ? 1 : 2;  // Mostly range 1, sometimes 2
                    
                    automaton->setNumStates(states);
                    automaton->setThreshold(thresh);
                    automaton->setRange(rng);
                    
                    // Quadrants initialization works well for these patterns
                    automaton->setInitPattern(CyclicAutomaton::QUADRANTS);
                }
            }
            
            return automaton;
        }
        case 5:
            return new BubblingLava(matrix, width, height);
        case 6:
            return new OrderAndChaos(matrix, width, height);
        default:
            return new ElementaryAutomaton(matrix, width, height);
    }
}

#endif // CELLULAR_AUTOMATA_H
