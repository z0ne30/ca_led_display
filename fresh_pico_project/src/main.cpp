#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Protomatter.h>
#include "PanelConfig.h"

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

void setup() {
  Serial1.begin(115200);
  Serial1.println("Panel Color Test");
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED on during setup
  
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
  
  // Display different colors on each panel
  Serial1.println("Panel identification test");
  
  // Clear display
  matrix.fillScreen(0);
  
  // Since we now see:
  // - Top-left: P0
  // - Top-right: P1
  // - Bottom-left: P2
  // - Bottom-right: P3
  //
  // Let's use this as our base mapping and just make the colors match the panels
  
  // Top-left: Should be P1 (GREEN) but showing P0, so put P1 in P0's position
  matrix.fillRect(0, 0, PANEL_WIDTH, PANEL_HEIGHT, matrix.color565(0, 255, 0));
  
  // Top-right: Should be P3 (YELLOW) but showing P1, so put P3 in P1's position
  matrix.fillRect(PANEL_WIDTH, 0, PANEL_WIDTH, PANEL_HEIGHT, matrix.color565(255, 255, 0));
  
  // Bottom-left: Should be P2 (BLUE) and showing P2, so this one is correct
  matrix.fillRect(0, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT, matrix.color565(0, 0, 255));
  
  // Bottom-right: Should be P0 (RED) but showing P3, so put P0 in P3's position
  matrix.fillRect(PANEL_WIDTH, PANEL_HEIGHT, PANEL_WIDTH, PANEL_HEIGHT, matrix.color565(255, 0, 0));
  
  // Add labels for each panel
  matrix.setTextSize(3);  // Larger text
  matrix.setTextColor(matrix.color565(255, 255, 255));
  
  // Set rotation to 180 degrees for all text
  matrix.setRotation(2);
  
  // Calculate center positions for each panel
  int charWidth = 18;
  int charHeight = 24;
  
  // Top-left: Should display P1 text
  int centerX = PANEL_WIDTH/2 - charWidth;
  int centerY = PANEL_HEIGHT/2 - charHeight/2;
  matrix.setCursor(centerX, centerY);
  matrix.print("P1");
  
  // Top-right: Should display P3 text
  centerX = PANEL_WIDTH + PANEL_WIDTH/2 - charWidth;
  centerY = PANEL_HEIGHT/2 - charHeight/2;
  matrix.setCursor(centerX, centerY);
  matrix.print("P3");
  
  // Bottom-left: Should display P2 text
  centerX = PANEL_WIDTH/2 - charWidth;
  centerY = PANEL_HEIGHT + PANEL_HEIGHT/2 - charHeight/2;
  matrix.setCursor(centerX, centerY);
  matrix.print("P2");
  
  // Bottom-right: Should display P0 text
  centerX = PANEL_WIDTH + PANEL_WIDTH/2 - charWidth;
  centerY = PANEL_HEIGHT + PANEL_HEIGHT/2 - charHeight/2;
  matrix.setCursor(centerX, centerY);
  matrix.print("P0");
  
  // Show the display
  matrix.show();
  
  Serial1.println("Panel identification complete");
}

void loop() {
  // Nothing to do in the loop
  delay(1000);
}
