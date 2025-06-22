# Quick Start Guide for RGB LED Matrix Testing

This guide will help you quickly get your 4-panel RGB LED matrix test up and running, addressing common issues along the way.

## Building the Project

### First-Time Setup

1. **Install PlatformIO** if you haven't already (as a VS Code extension)

2. **Open the project** in PlatformIO
   - Open VS Code
   - File → Open Folder → select the `fresh_pico_project` folder
   
3. **Fixing C/C++ IntelliSense Errors**
   
   If you see errors like:
   ```
   cannot open source file "Arduino.h"
   #include errors detected. Please update your includePath.
   ```
   
   These are just VS Code IntelliSense errors, not actual compile errors. To fix them:
   
   - Run a PlatformIO build first (this downloads the libraries)
   - Click the PlatformIO icon in the sidebar
   - Select "Project Tasks" → "Build"
   
   If errors persist:
   - Click "Terminal" → "New Terminal" in VS Code
   - Run: `pio run -t clean` and then `pio run`
   - Restart VS Code if needed

## Hardware Connection

1. **Connect your Pico** to the RGB LED matrix panels following this pinout:

   ```
   Pico Pin    Function
   -----------------------------
   GP2         R1 (Red high bit)
   GP3         G1 (Green high bit)
   GP4         B1 (Blue high bit)
   GP5         R2 (Red low bit)
   GP8         G2 (Green low bit)
   GP9         B2 (Blue low bit)
   GP10        A (Address line A)
   GP16        B (Address line B)
   GP18        C (Address line C)
   GP20        D (Address line D)
   GP22        E (Address line E)
   GP11        CLK (Clock)
   GP12        LAT (Latch)
   GP13        OE (Output Enable)
   ```

2. **Chain the panels** in the order specified in your `PanelConfig.h` file:
   - Connect output of Panel 1 to input of Panel 2
   - Connect output of Panel 2 to input of Panel 3
   - Continue to Panel 4

3. **Power supply**:
   - Use a 5V power supply with at least 4A per panel (16A total)
   - Connect power supply to all panels (power injection may be needed)
   - Make sure the ground from the Pico is connected to the ground of the panels

## Uploading and Testing

1. **Put Pico in bootloader mode**:
   - Press and hold the BOOTSEL button on the Pico
   - While holding BOOTSEL, connect the Pico to your computer via USB
   - Release BOOTSEL after connecting

2. **Upload the code**:
   - In PlatformIO, click the upload button (right arrow icon)
   - Or run `pio run -t upload` in the terminal

3. **Monitor the output**:
   - Open the Serial Monitor in PlatformIO
   - Or run `pio device monitor` in the terminal
   - Set the baud rate to 115200

## Troubleshooting Panel Issues

If the panels aren't displaying correctly:

1. **No display at all**:
   - Check power connections
   - Verify data pins are connected correctly
   - Make sure the Pico's onboard LED turns off after initialization

2. **Wrong panel order or orientation**:
   - Look at the panel identification test (panels should be Red, Green, Blue, Yellow)
   - Note which color appears on which physical panel
   - Edit `PanelConfig.h` to match your physical arrangement

3. **Incorrect mapping across panels**:
   - Use the grid test to verify coordinate mapping
   - Check the cross-panel line test - lines should be continuous
   - Update `PANEL_CONFIGS` array in `PanelConfig.h`

4. **Flickering or dim display**:
   - Check power supply capacity (should be 5V/16A for 4 panels)
   - Reduce bit depth in `main.cpp` from 6 to 4 or 3
   - Check for loose connections

## Next Steps

1. After successfully testing the panel configuration, you can modify the code for your specific application
2. Add new animations or display patterns in the `loop()` function
3. Try increasing the FPS by optimizing the display code

## Command Reference

```bash
# Build the project
pio run

# Upload to Pico
pio run -t upload

# Clean the project
pio run -t clean

# Monitor serial output
pio device monitor
```

For more detailed information, see the full [README.md](./README.md).