# Testing Plan for 4-Panel RGB Matrix Display (64x64×4)

## Hardware Configuration

```
┌─────┬─────┐
│     │     │
│ P1  │ P2  │
│     │     │
├─────┼─────┤
│     │     │
│ P3  │ P4  │
│     │     │
└─────┴─────┘
```

## 1. Setup and Configuration

### Required Modifications to the Code

```cpp
// Update the panel configuration in main.cpp
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANEL_CHAIN 4  // 4 panels in a chain

// Update matrix initialization
Adafruit_Protomatter matrix(
  PANEL_WIDTH,      // Matrix width
  6,                // Bit depth
  1,                // Number of parallel chains
  rgbPins,
  5,                // Number of address pins
  addrPins,
  CLK_PIN, LAT_PIN, OE_PIN,
  true,             // Double buffering
  2                 // Progressive tiling (adjust based on your wiring)
);
```

## 2. Basic Testing Steps

1. **Power Supply Check**
   - Ensure adequate power (5V/4A per panel = 16A total)
   - Use proper gauge wiring for power distribution

2. **Connection Verification**
   - Connect output of Panel 1 to input of Panel 2
   - Connect output of Panel 2 to input of Panel 3
   - Connect output of Panel 3 to input of Panel 4
   - Verify all data connections are secure

3. **Initial Testing**
   - Display solid colors across all panels
   - Verify continuity between panels
   - Check for brightness uniformity

## 3. Full-Grid Testing

1. **Coordinate System Test**
   - Draw a grid pattern to verify panel ordering
   - Display panel numbers on each panel

2. **Cross-Panel Drawing Test**
   - Draw lines that span across multiple panels
   - Display text that crosses panel boundaries

3. **Full Display Animation**
   - Test scrolling text across the entire display
   - Run animations that utilize the full 128×128 area

## 4. Performance Testing

1. **Refresh Rate Testing**
   - Measure FPS with different content types
   - Test with varying bit depths (trade color depth for speed)

2. **Memory Usage Optimization**
   - Monitor RAM usage
   - Implement double-buffering efficiently

## 5. Troubleshooting Common Issues

1. **Panel Ordering Problems**
   - Adjust tileMode parameter (0=none, 1=serpentine, 2=progressive)
   - Match software configuration to physical wiring

2. **Flickering or Artifacts**
   - Reduce bit depth
   - Check for adequate power
   - Ensure clock speeds are appropriate

3. **Data Corruption**
   - Shorten data cables
   - Add termination resistors if needed
   - Check for interference sources