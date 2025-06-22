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
#define FRAME_DELAY 50      // Milliseconds between frames
#define AUTOMATON_DURATION 60000  // Run each automaton for 60 seconds before switching

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

// Function to display the automaton name
void displayAutomatonName(const char* name) {
  // Clear just the top portion of the screen
  matrix.fillRect(0, 0, TOTAL_WIDTH, 10, 0);
  
  // Set text properties
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.color565(200, 200, 200));
  matrix.setRotation(0);  // Normal rotation for text
  
  // Calculate center position
  int16_t x1, y1;
  uint16_t w, h;
  matrix.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
  int16_t centerX = (TOTAL_WIDTH - w) / 2;
  
  // Draw the text
  matrix.setCursor(centerX, 2);
  matrix.print(name);
  matrix.show();
}

// Function to select a random automaton
void selectRandomAutomaton() {
  // Delete any existing automaton
  if (currentAutomaton != nullptr) {
    delete currentAutomaton;
    currentAutomaton = nullptr;
  }
  
  // Seed the random number generator with a combination of time and analog noise
  randomSeed(millis() ^ analogRead(A0));
  
  // Create a new random automaton
  currentAutomaton = createRandomAutomaton(&matrix, TOTAL_WIDTH, TOTAL_HEIGHT);
  
  // Initialize the automaton
  currentAutomaton->init();
  
  // Display the name of the automaton
  displayAutomatonName(currentAutomaton->getName());
  
  // Reset the timer
  lastAutomatonChange = millis();
  
  Serial1.print("Selected automaton: ");
  Serial1.println(currentAutomaton->getName());
}

void setup() {
  Serial1.begin(115200);
  Serial1.println("Cellular Automata Demo");
  
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
  
  // Select a random automaton to start
  selectRandomAutomaton();
}

void loop() {
  // Check if it's time to switch to a new automaton
  if (millis() - lastAutomatonChange > AUTOMATON_DURATION) {
    selectRandomAutomaton();
  }
  
  // Update and render the current automaton
  if (currentAutomaton != nullptr) {
    currentAutomaton->step();
  }
  
  // Small delay to control frame rate
  delay(FRAME_DELAY);
}
