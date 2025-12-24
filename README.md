# Modifier Key Detector

A modular Windows application for detecting modifier key states using Interception driver.

## Project Structure

```
escModKey/
├── include/                    # Header files
│   ├── interception.h         # Interception driver API
│   ├── utils.h                # Utility functions
│   └── physical_key_detector.h # Physical key detector module
├── src/                       # Source files
│   ├── main.cpp              # Main program (ready for development)
│   └── physical_key_detector.cpp # Physical key detector implementation
├── test/                      # Test programs
│   └── test_physical_detector.cpp # Physical key detector test
├── lib/                       # Libraries
│   ├── interception.dll
│   └── interception.lib
├── build/                     # Build output (generated)
├── xmake.lua                 # Build configuration
├── run.ps1                   # Run main program
└── run_test.ps1              # Run test program
```

## Modules

### PhysicalKeyDetector

Hardware-level detection of modifier key states (Ctrl, Shift, Alt, Win).

**Features:**
- Detects actual physical key presses via Interception driver
- Cannot be fooled by software key simulation
- Distinguishes between left and right modifier keys
- Simple API for integration

**Documentation:** See `USAGE.md`

### VirtualKeyDetector

Software-level detection of modifier key states (Ctrl, Shift, Alt, Win).

**Features:**
- Detects virtual key states via Windows API
- Can be affected by software key simulation
- Distinguishes between left and right modifier keys
- No special privileges required
- Simple polling-based API

## Building

```bash
xmake
```

This will build:
- `escModKey.exe` - Main program
- `test_physical_detector.exe` - Physical key detector test
- `test_virtual_detector.exe` - Virtual key detector test

## Running

### Main Program
```powershell
.\run.ps1
```

### Test Programs

**Physical Key Detector:**
```powershell
.\run_test.ps1
```

**Virtual Key Detector:**
```powershell
.\run_test_virtual.ps1
```

Or manually:
```bash
.\build\windows\x64\release\test_physical_detector.exe
.\build\windows\x64\release\test_virtual_detector.exe
```

## Requirements

- Windows OS
- Interception driver installed
- Administrator privileges
- Visual Studio 2019 or later (for building)

## Development

The `src/main.cpp` is kept clean and ready for new module development. All test code is in the `test/` directory.

To add a new test:
1. Create `test/test_your_module.cpp`
2. Add a new target in `xmake.lua`
3. Build with `xmake`

## Next Steps

- Combine physical and virtual detection for comparison
- Additional input detection features

