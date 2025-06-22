#ifndef CELLULAR_AUTOMATA_H
#define CELLULAR_AUTOMATA_H

#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include "PanelConfig.h"

// Number of distinct automata implementations
#define NUM_AUTOMATA 5

// Forward declarations of automata classes
class CellularAutomaton;
class ElementaryAutomaton;
class GameOfLife;
class BriansBrain;
class LangtonsAnt;
class CyclicAutomaton;

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
    Adafruit_Protomatter* matrix;  // Pointer to the LED matrix
    uint16_t width;                // Width of the matrix
    uint16_t height;               // Height of the matrix
    uint32_t frameCount;           // Current frame count
};

/**
 * 1D Elementary Cellular Automaton (Rule 30, 90, 110, etc.)
 */
class ElementaryAutomaton : public CellularAutomaton {
public:
    ElementaryAutomaton(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height, uint8_t rule = 30) 
        : CellularAutomaton(matrix, width, height), rule(rule) {
        cells = new uint8_t[width * height];
        tempCells = new uint8_t[width];
    }
    
    ~ElementaryAutomaton() {
        delete[] cells;
        delete[] tempCells;
    }
    
    void init() override {
        // Clear the cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Start with a single cell in the middle of the top row
        cells[width / 2] = 1;
        
        // Or start with random cells in the top row
        if (random(100) < 30) {  // 30% chance for random start
            for (uint16_t x = 0; x < width; x++) {
                cells[x] = random(2);
            }
        }
        
        // Reset current row
        currentRow = 0;
    }
    
    void update() override {
        // Only update if we haven't filled the screen
        if (currentRow >= height - 1) return;
        
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
        matrix->fillScreen(0);  // Clear the screen
        
        // Draw all cells
        for (uint16_t y = 0; y < height && y <= currentRow; y++) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[y * width + x]) {
                    matrix->drawPixel(x, y, matrix->color565(255, 255, 255));  // White cells
                }
            }
        }
        
        matrix->show();
    }
    
    void setRule(uint8_t newRule) {
        rule = newRule;
        init();  // Reinitialize with the new rule
    }
    
    const char* getName() const override {
        static char name[20];
        sprintf(name, "Rule %d", rule);
        return name;
    }
    
    // Change to a random interesting rule
    void randomRule() {
        // Some interesting rules
        const uint8_t interestingRules[] = {30, 54, 60, 90, 102, 110, 150, 158, 182, 184, 190};
        rule = interestingRules[random(sizeof(interestingRules) / sizeof(interestingRules[0]))];
    }
    
private:
    uint8_t* cells;       // Cell states
    uint8_t* tempCells;   // Temporary buffer for computation
    uint8_t rule;         // The rule to apply (0-255)
    uint16_t currentRow;  // Current row being calculated
};

/**
 * Conway's Game of Life
 */
class GameOfLife : public CellularAutomaton {
public:
    GameOfLife(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height) 
        : CellularAutomaton(matrix, width, height) {
        cells = new uint8_t[width * height];
        nextCells = new uint8_t[width * height];
    }
    
    ~GameOfLife() {
        delete[] cells;
        delete[] nextCells;
    }
    
    void init() override {
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Randomly seed cells (about 25% alive)
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                cells[y * width + x] = random(100) < 25;
            }
        }
        
        // Or add a common pattern (glider, blinker, etc.)
        if (random(100) < 40) {  // 40% chance for a pattern
            addPattern();
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
                
                // Apply Conway's Game of Life rules
                uint8_t current = cells[y * width + x];
                uint8_t next = 0;
                
                if (current) {
                    // Cell is alive
                    next = (neighbors == 2 || neighbors == 3);
                } else {
                    // Cell is dead
                    next = (neighbors == 3);
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
        matrix->fillScreen(0);  // Clear the screen
        
        // Draw all live cells
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[y * width + x]) {
                    matrix->drawPixel(x, y, matrix->color565(255, 255, 255));  // White cells
                }
            }
        }
        
        matrix->show();
    }
    
    const char* getName() const override {
        return "Game of Life";
    }
    
private:
    uint8_t* cells;      // Current generation
    uint8_t* nextCells;  // Next generation
    
    // Add a common Game of Life pattern
    void addPattern() {
        // Choose a random pattern
        uint8_t pattern = random(4);
        
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
        matrix->fillScreen(0);  // Clear the screen
        
        // Draw all cells
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                uint8_t state = cells[y * width + x];
                if (state == 1) {
                    // On cells - white
                    matrix->drawPixel(x, y, matrix->color565(255, 255, 255));
                } else if (state == 2) {
                    // Dying cells - blue
                    matrix->drawPixel(x, y, matrix->color565(0, 0, 255));
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
        matrix->fillScreen(0);  // Clear the screen
        
        // Draw all cells
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                if (cells[y * width + x]) {
                    matrix->drawPixel(x, y, matrix->color565(160, 160, 160));  // Light gray for on cells
                }
            }
        }
        
        // Draw all ants on top
        for (uint8_t i = 0; i < numAnts; i++) {
            matrix->drawPixel(ants[i].x, ants[i].y, ants[i].color);
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
            uint8_t antCount = random(1, 6);  // 1-5 ants
            return new LangtonsAnt(matrix, width, height, antCount);
        }
        case 4:
            return new CyclicAutomaton(matrix, width, height);
        default:
            return new ElementaryAutomaton(matrix, width, height);
    }
}

/**
 * Cyclic Cellular Automaton
 *
 * Each cell has a state from 0 to numStates-1.
 * A cell in state s changes to state (s+1) % numStates if enough
 * neighbors are in state (s+1) % numStates.
 */
class CyclicAutomaton : public CellularAutomaton {
public:
    CyclicAutomaton(Adafruit_Protomatter* matrix, uint16_t width, uint16_t height,
                   uint8_t numStates = 16, uint8_t threshold = 3)
        : CellularAutomaton(matrix, width, height),
          numStates(numStates), threshold(threshold) {
        
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
        // Create a new color palette for variety
        generateColorPalette();
        
        // Clear all cells
        memset(cells, 0, width * height * sizeof(uint8_t));
        
        // Random starting pattern
        if (random(100) < 50) {
            // Random cells throughout
            for (uint16_t y = 0; y < height; y++) {
                for (uint16_t x = 0; x < width; x++) {
                    cells[y * width + x] = random(numStates);
                }
            }
        } else {
            // Start with a small seed in the center
            uint16_t centerX = width / 2;
            uint16_t centerY = height / 2;
            uint16_t radius = min(width, height) / 10;
            
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
    }
    
    void update() override {
        // Calculate the next generation
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                // Get current state
                uint8_t currentState = cells[y * width + x];
                
                // Calculate next state (cyclically)
                uint8_t nextState = (currentState + 1) % numStates;
                
                // Count neighbors in the next state
                uint8_t neighbors = 0;
                for (int8_t dy = -1; dy <= 1; dy++) {
                    for (int8_t dx = -1; dx <= 1; dx++) {
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
                if (neighbors >= threshold) {
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
        // Draw all cells with their respective colors
        for (uint16_t y = 0; y < height; y++) {
            for (uint16_t x = 0; x < width; x++) {
                uint8_t state = cells[y * width + x];
                matrix->drawPixel(x, y, colorPalette[state]);
            }
        }
        
        matrix->show();
    }
    
    const char* getName() const override {
        static char name[32];
        sprintf(name, "Cyclic Automaton (%d states)", numStates);
        return name;
    }
    
private:
    uint8_t* cells;        // Current generation
    uint8_t* nextCells;    // Next generation
    uint8_t numStates;     // Number of states
    uint8_t threshold;     // Threshold for state change
    uint16_t* colorPalette; // Color palette for each state
    
    // Generate a color palette
    void generateColorPalette() {
        // Choose one of several possible color schemes
        uint8_t scheme = random(4);
        
        switch (scheme) {
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

#endif // CELLULAR_AUTOMATA_H