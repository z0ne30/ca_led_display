# 4-Panel RGB LED Matrix Cellular Automata Display

This project implements various cellular automata algorithms on a 4-panel RGB LED matrix (64x64 per panel) setup using the Raspberry Pi Pico and Adafruit Protomatter library.

## Overview

This project showcases various cellular automata algorithms visualized on a multi-panel RGB LED matrix display. Cellular automata are mathematical models that simulate how cells evolve over time based on simple rules and neighboring cell states. The project includes implementations of:

1. Elementary Cellular Automata (Rule 30, 90, 110, etc.)
2. Conway's Game of Life
3. Brian's Brain
4. Langton's Ant
5. Cyclic Cellular Automaton
6. Bubbling Lava
7. Order and Chaos

The display framework handles coordinate mapping for multi-panel setups, allowing for custom panel arrangements while maintaining proper visual continuity across panels.

## Hardware Requirements

- Raspberry Pi Pico
- 4 RGB LED Matrix panels (64x64 P3.0)
- 5V power supply (4A per panel recommended = 16A total)
- Appropriate wiring according to the pinout in the code

## Pin Configuration

The default pinout is:

```
R1: GP2     G1: GP3     B1: GP4
R2: GP5     G2: GP8     B2: GP9

A: GP10     B: GP16     C: GP18     D: GP20     E: GP22

CLK: GP11   LAT: GP12   OE: GP13
```

## Panel Configuration

The test code allows for custom panel arrangements. By default, it assumes a 2x2 grid:

```
┌─────┬─────┐
│     │     │
│  0  │  1  │
│     │     │
├─────┼─────┤
│     │     │
│  2  │  3  │
│     │     │
└─────┴─────┘
```

Where the numbers represent the logical positions of the panels. The physical positions (order in the chain) and rotation can be configured in `src/PanelConfig.h`.

## Customizing Panel Arrangement

To customize your panel arrangement, edit the `PANEL_CONFIGS` array in `src/PanelConfig.h`:

```cpp
const PanelConfig PANEL_CONFIGS[PANEL_COUNT] = {
    {0, 0, 0},  // Logical position 0 (top-left) -> Physical position 0, no rotation
    {1, 1, 0},  // Logical position 1 (top-right) -> Physical position 1, no rotation
    {2, 2, 0},  // Logical position 2 (bottom-left) -> Physical position 2, no rotation
    {3, 3, 0}   // Logical position 3 (bottom-right) -> Physical position 3, no rotation
};
```

Each entry contains:
1. Logical position (0-3): The position in the virtual grid (0=top-left, 1=top-right, etc.)
2. Physical position (0-3): The position in the physical chain (0=first panel, 1=second panel, etc.)
3. Rotation (0-3): 0=normal, 1=90° clockwise, 2=180°, 3=270° clockwise

### Example Custom Configurations

**1. Serpentine Arrangement**:
```cpp
const PanelConfig PANEL_CONFIGS[PANEL_COUNT] = {
    {0, 0, 0},  // Logical position 0 (top-left) -> Physical position 0
    {1, 1, 0},  // Logical position 1 (top-right) -> Physical position 1
    {2, 3, 0},  // Logical position 2 (bottom-left) -> Physical position 3
    {3, 2, 0}   // Logical position 3 (bottom-right) -> Physical position 2
};
```

**2. Rotated Panel**:
```cpp
const PanelConfig PANEL_CONFIGS[PANEL_COUNT] = {
    {0, 0, 0},  // Logical position 0 (top-left) -> Physical position 0, normal
    {1, 1, 0},  // Logical position 1 (top-right) -> Physical position 1, normal
    {2, 2, 0},  // Logical position 2 (bottom-left) -> Physical position 2, normal
    {3, 3, 2}   // Logical position 3 (bottom-right) -> Physical position 3, 180° rotation
};
```

**3. Linear Chain (1x4)**:
```cpp
// First, modify TOTAL_WIDTH and TOTAL_HEIGHT in PanelConfig.h:
#define TOTAL_WIDTH (PANEL_WIDTH * 4)
#define TOTAL_HEIGHT PANEL_HEIGHT

// Then update the panel configs:
const PanelConfig PANEL_CONFIGS[PANEL_COUNT] = {
    {0, 0, 0},  // Logical position 0 -> Physical position 0
    {1, 1, 0},  // Logical position 1 -> Physical position 1
    {2, 2, 0},  // Logical position 2 -> Physical position 2
    {3, 3, 0}   // Logical position 3 -> Physical position 3
};
```

## Cellular Automata Implementations

The project includes several cellular automata implementations:

1. **Elementary Cellular Automaton**: 1D automaton with rules like Rule 30 (chaos), Rule 90 (Sierpinski triangle), and Rule 110 (Turing complete)
2. **Conway's Game of Life**: Classic 2D cellular automaton with rules for birth, survival, and death
3. **Brian's Brain**: Three-state cellular automaton with "ready", "firing", and "refractory" states
4. **Langton's Ant**: Cellular automaton where an "ant" moves based on cell colors, creating emergent patterns
5. **Cyclic Cellular Automaton**: Cells cycle through colors based on their neighbors
6. **Bubbling Lava**: A custom automaton simulating bubbling lava-like effects
7. **Order and Chaos**: A custom automaton showcasing the transition between ordered and chaotic states

The display automatically rotates between these different automata at regular intervals.

## Building and Uploading

1. Connect your Raspberry Pi Pico to your computer while holding the BOOTSEL button to enter bootloader mode
2. Use PlatformIO to build and upload the code:

```bash
# From the project directory
pio run -t upload
```

3. The test patterns will start running automatically

## Troubleshooting

If the display doesn't work correctly:

1. **Power issues**: Ensure your power supply can provide enough current (16A for 4 panels)
2. **Initialization**: The special initialization sequence (`Reginit()`) is critical for these panels
3. **Mapping problems**: If the panels display in the wrong order or orientation, adjust the `PANEL_CONFIGS` array
4. **Connection issues**: Double-check all wiring connections against the pinout

## Performance Considerations

- For better performance, reduce the bit depth from 6 to 4 or 3
- Use the built-in LED to monitor the Pico's status (on during setup, off when running)
- The serial output (115200 baud) provides debugging information and FPS measurements

## Customizing and Extending

You can customize this project in several ways:

1. **Modify Automata Parameters**: Adjust parameters in `CellularAutomata.h` to create different visual effects
2. **Add New Automata**: Create your own cellular automata by inheriting from the `CellularAutomaton` base class
3. **Change Timing**: Modify the transition time between automata in `main.cpp` (AUTOMATON_DURATION)
4. **Adjust Animation Speed**: Change the FRAME_DELAY constant in `main.cpp` to speed up or slow down animations

The modular design makes it easy to experiment with different cellular automata rules and visualization techniques.