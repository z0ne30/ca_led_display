#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Protomatter.h>
#include "PanelConfig.h"
#include "CellularAutomata.h"

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

// Panel dimensions
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANEL_COUNT 4
#define TOTAL_WIDTH (PANEL_WIDTH * 2)
#define TOTAL_HEIGHT (PANEL_HEIGHT * 2)

// Animation speed settings
#define FRAME_DELAY 20      // Milliseconds between frames (reduced for faster animation)
#define AUTOMATON_DURATION 180000  // Run each automaton for 3 minutes before switching

// Global variables
uint8_t rgbPins[] = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN};
uint8_t addrPins[] = {A_PIN, B_PIN, C_PIN, D_PIN, E_PIN};

// Create the matrix object with explicit width parameter for multiple panels
Adafruit_Protomatter matrix(
  PANEL_WIDTH * 2,  // CRITICAL: Width must be total width (2 panels wide)
  4,                // Bit depth (4 = 4096 colors)
  1, rgbPins,       // Number of RGB pins, RGB pins
  5, addrPins,      // Number of address pins, address pins
  CLK_PIN, LAT_PIN, OE_PIN,  // Other pins
  true,             // Double-buffering
  2                 // 2 vertical tiles (2x2 grid)
);

// Global pointer to the current cellular automaton
CellularAutomaton* currentAutomaton = nullptr;
unsigned long lastAutomatonChange = 0;

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

// Function to draw a pixel with proper panel mapping
void drawMappedPixel(Adafruit_Protomatter* matrix, int16_t x, int16_t y, uint16_t color) {
  int16_t mapped_x, mapped_y;
  mapCoordinates(x, y, &mapped_x, &mapped_y);
  matrix->drawPixel(mapped_x, mapped_y, color);
}

// Function to draw text using mapped coordinates
void drawMappedText(Adafruit_Protomatter* matrix, const char* text, int16_t x, int16_t y, uint16_t color) {
  // Save current text settings
  int16_t x1, y1;
  uint16_t w, h;
  
  // Get text bounds for the entire string
  matrix->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  
  // Map the starting position
  int16_t mapped_x, mapped_y;
  mapCoordinates(x, y, &mapped_x, &mapped_y);
  
  // Set cursor to mapped position and color
  matrix->setCursor(mapped_x, mapped_y);
  matrix->setTextColor(color);
  
  // Print the entire text at once
  matrix->print(text);
}

// Function to display automaton name on the top-right panel only
void displayAutomatonName(const char* name) {
  // Clear everything
  matrix.fillScreen(0);
  
  // Define colors
  uint16_t WHITE = matrix.color565(255, 255, 255);
  uint16_t RED = matrix.color565(255, 0, 0);
  
  // Log the name to serial
  Serial1.print("Selected automaton: ");
  Serial1.println(name);
  
  // CRITICAL: Explicitly clear the bottom-right panel
  for (int x = PANEL_WIDTH; x < PANEL_WIDTH * 2; x++) {
    for (int y = PANEL_HEIGHT; y < PANEL_HEIGHT * 2; y++) {
      matrix.drawPixel(x, y, 0); // Black
    }
  }
  
  // Draw a red border around the top-right panel to make it clear
  for (int x = PANEL_WIDTH; x < PANEL_WIDTH * 2; x++) {
    matrix.drawPixel(x, 0, RED);
    matrix.drawPixel(x, PANEL_HEIGHT - 1, RED);
  }
  for (int y = 0; y < PANEL_HEIGHT; y++) {
    matrix.drawPixel(PANEL_WIDTH, y, RED);
    matrix.drawPixel(PANEL_WIDTH * 2 - 1, y, RED);
  }
  
  // Set text size and color - use smallest possible text
  matrix.setTextSize(1); // Default size is about 8 pixels tall
  matrix.setTextColor(WHITE);
  
  // Define the text area for the top-right panel only
  const int TEXT_AREA_WIDTH = PANEL_WIDTH - 10; // 5 pixel margin on each side
  const int TEXT_AREA_X = PANEL_WIDTH + 5; // Top-right panel starts at PANEL_WIDTH
  const int TEXT_AREA_Y = 15; // Position from top
  const int LINE_HEIGHT = 9; // Reduced space between lines
  
  // Split the text into words
  char words[32][32]; // Assuming no more than 32 words, each no longer than 31 chars
  int wordCount = 0;
  
  // Copy the name to a temporary buffer we can modify
  char tempName[256];
  strncpy(tempName, name, 255);
  tempName[255] = '\0';
  
  // Split the string into words
  char* word = strtok(tempName, " ");
  while (word != NULL && wordCount < 32) {
    strcpy(words[wordCount], word);
    wordCount++;
    word = strtok(NULL, " ");
  }
  
  // Now arrange words into lines that fit the width
  char lines[8][64]; // Up to 8 lines, each up to 63 chars
  int lineCount = 0;
  int currentLine = 0;
  uint16_t currentLineWidth = 0;
  
  lines[0][0] = '\0'; // Start with empty first line
  
  for (int i = 0; i < wordCount; i++) {
    // Calculate width of this word
    int16_t x1, y1;
    uint16_t wordWidth, h;
    matrix.getTextBounds(words[i], 0, 0, &x1, &y1, &wordWidth, &h);
    
    // Calculate width if we add this word to the current line
    char testLine[64];
    if (strlen(lines[currentLine]) > 0) {
      // Add a space before the word if the line isn't empty
      sprintf(testLine, "%s %s", lines[currentLine], words[i]);
    } else {
      strcpy(testLine, words[i]);
    }
    
    uint16_t testWidth;
    matrix.getTextBounds(testLine, 0, 0, &x1, &y1, &testWidth, &h);
    
    // Check if adding this word would exceed the line width
    if (testWidth <= TEXT_AREA_WIDTH) {
      // Word fits, add it to the current line
      strcpy(lines[currentLine], testLine);
      currentLineWidth = testWidth;
    } else {
      // Word doesn't fit, start a new line
      currentLine++;
      if (currentLine >= 8) break; // Maximum 8 lines
      
      // Start the new line with this word
      strcpy(lines[currentLine], words[i]);
      
      // Calculate the width of the new line
      matrix.getTextBounds(lines[currentLine], 0, 0, &x1, &y1, &currentLineWidth, &h);
    }
  }
  
  // Update line count
  lineCount = currentLine + 1;
  
  // Display each line centered
  for (int i = 0; i < lineCount; i++) {
    // Calculate width for centering
    int16_t x1, y1;
    uint16_t lineWidth, h;
    matrix.getTextBounds(lines[i], 0, 0, &x1, &y1, &lineWidth, &h);
    
    // Center and display this line
    int centerX = TEXT_AREA_X + (TEXT_AREA_WIDTH - lineWidth) / 2;
    matrix.setCursor(centerX, TEXT_AREA_Y + i * LINE_HEIGHT);
    matrix.print(lines[i]);
  }
  
  // Show the display
  matrix.show();
  delay(5000); // Show name for 5 seconds as requested
}


// Keep track of the last automaton type to avoid repeating
static uint8_t lastAutomatonType = 255; // Initialize to an invalid value

// Function to select a random automaton
void selectRandomAutomaton() {
  // Delete any existing automaton
  if (currentAutomaton != nullptr) {
    delete currentAutomaton;
    currentAutomaton = nullptr;
  }
  
  // Seed the random number generator with multiple sources of entropy
  // Use a combination of time, analog noise, and a rotating value
  static uint32_t seedRotator = 0;
  seedRotator = (seedRotator * 1664525) + 1013904223; // Simple LCG for additional entropy
  randomSeed(millis() ^ analogRead(A0) ^ seedRotator);
  
  // Select a random automaton type, ensuring it's different from the last one
  uint8_t newType;
  do {
    newType = random(NUM_AUTOMATA);
  } while (newType == lastAutomatonType && NUM_AUTOMATA > 1);
  
  // Remember this type to avoid repeating next time
  lastAutomatonType = newType;
  
  // Create the new automaton of the selected type
  switch (newType) {
    case 0: {
      ElementaryAutomaton* automaton = new ElementaryAutomaton(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT);
      automaton->randomRule();
      currentAutomaton = automaton;
      break;
    }
    case 1:
      currentAutomaton = new GameOfLife(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT);
      break;
    case 2:
      currentAutomaton = new BriansBrain(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT);
      break;
    case 3: {
      uint8_t antCount = random(1, 6);  // 1-5 ants
      currentAutomaton = new LangtonsAnt(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT, antCount);
      break;
    }
    case 4:
      currentAutomaton = new CyclicAutomaton(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT);
      break;
    case 5:
      currentAutomaton = new BubblingLava(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT);
      break;
    case 6:
      currentAutomaton = new OrderAndChaos(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT);
      break;
    default:
      currentAutomaton = new ElementaryAutomaton(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT);
      break;
  }
  
  // Initialize the automaton
  currentAutomaton->init();
  
  // Display the name of the automaton
  displayAutomatonName(currentAutomaton->getName());
  
  // Reset the timer
  lastAutomatonChange = millis();
  
  Serial1.print("Selected automaton: ");
  Serial1.println(currentAutomaton->getName());
}

// Function to display test pattern with position labels
void displayTestPattern() {
  // Clear everything
  matrix.fillScreen(0);
  matrix.show();
  delay(200);  // Brief pause to ensure the clear is visible
  
  // Define colors
  uint16_t WHITE = matrix.color565(255, 255, 255);
  uint16_t RED = matrix.color565(255, 0, 0);
  uint16_t GREEN = matrix.color565(0, 255, 0);
  uint16_t BLUE = matrix.color565(0, 0, 255);
  
  // Display a simple test pattern
  // Draw a centered message
  matrix.setTextSize(1);
  
  // Set text size and color
  matrix.setTextSize(1);
  matrix.setTextColor(WHITE);
  
  // Calculate text position to center it
  int16_t x1, y1;
  uint16_t w, h;
  const char* testMsg = "Panel Test";
  
  // Get text bounds to center it properly
  matrix.getTextBounds(testMsg, 0, 0, &x1, &y1, &w, &h);
  
  // Calculate the logical center of the entire display
  int16_t logicalCenterX = TOTAL_WIDTH / 2;
  int16_t logicalCenterY = TOTAL_HEIGHT / 2 - 20; // Offset up by 20 pixels
  
  // Calculate the starting position to center the text in logical coordinates
  // Adjust for the text bounds offset and baseline
  int16_t logicalTextX = logicalCenterX - (w / 2) - x1;
  int16_t logicalTextY = logicalCenterY - (h / 2) - y1;
  
  // Map the logical coordinates to physical coordinates
  int16_t physicalTextX, physicalTextY;
  mapCoordinates(logicalTextX, logicalTextY, &physicalTextX, &physicalTextY);
  
  // Set cursor to the mapped position
  matrix.setCursor(physicalTextX, physicalTextY);
  
  // Print the text directly
  matrix.print(testMsg);
  
  // Draw some simple shapes to verify panel operation using mapped coordinates
  // Red circle in top left quadrant
  
  // Draw circle using mapped coordinates
  for (int y = -10; y <= 10; y++) {
    for (int x = -10; x <= 10; x++) {
      if (x*x + y*y <= 100) { // Circle with radius 10
        drawMappedPixel(&matrix, PANEL_WIDTH/2 + x, PANEL_HEIGHT/2 + y, RED);
      }
    }
  }
  
  // Green square in top right quadrant
  for (int y = -10; y <= 10; y++) {
    for (int x = -10; x <= 10; x++) {
      drawMappedPixel(&matrix, PANEL_WIDTH + PANEL_WIDTH/2 + x, PANEL_HEIGHT/2 + y, GREEN);
    }
  }
  
  // Blue triangle in bottom left quadrant
  for (int y = -10; y <= 10; y++) {
    for (int x = -10; x <= 10; x++) {
      // Simple triangle check
      if (y <= 0 && y >= -x && y >= x) {
        drawMappedPixel(&matrix, PANEL_WIDTH/2 + x, PANEL_HEIGHT + PANEL_HEIGHT/2 + y, BLUE);
      }
    }
  }
  
  // White X in bottom right quadrant
  for (int i = -10; i <= 10; i++) {
    drawMappedPixel(&matrix, PANEL_WIDTH + PANEL_WIDTH/2 + i, PANEL_HEIGHT + PANEL_HEIGHT/2 + i, WHITE);
    drawMappedPixel(&matrix, PANEL_WIDTH + PANEL_WIDTH/2 + i, PANEL_HEIGHT + PANEL_HEIGHT/2 - i, WHITE);
  }
  
  // Show the pattern
  matrix.show();
  
  // Log the panel configuration
  Serial1.println("Panel configuration test pattern displayed");
  Serial1.println("Panel mapping (logical to physical):");
  for (int i = 0; i < PANEL_COUNT; i++) {
    Serial1.print("Logical ");
    Serial1.print(i);
    Serial1.print(" -> Physical ");
    Serial1.print(PANEL_CONFIGS[i].physicalPosition);
    Serial1.print(", Rotation: ");
    Serial1.println(PANEL_CONFIGS[i].rotation);
  }
}

void setup() {
  Serial1.begin(115200);
  Serial1.println("LED Matrix Panel Animation");
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED on during setup
  
  // Initialize random seed from an unconnected analog pin
  randomSeed(analogRead(A0));
  
  // Initialize the panels
  Reginit();
  
  if (matrix.begin() != PROTOMATTER_OK) {
    Serial1.println("Matrix initialization failed!");
    while (1) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }
  }
  
  Serial1.println("Matrix initialized successfully");
  digitalWrite(LED_BUILTIN, LOW); // LED off when ready
  
  // Start with a random cellular automaton
  selectRandomAutomaton();
}

void loop() {
  // Update the current automaton
  if (currentAutomaton != nullptr) {
    currentAutomaton->step();
    delay(FRAME_DELAY); // Wait between frames
    
    // Check if it's time to switch to a new automaton (after 3 minutes)
    if (millis() - lastAutomatonChange > AUTOMATON_DURATION) {
      selectRandomAutomaton();
    }
  }
}
