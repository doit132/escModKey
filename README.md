# Modifier Key Auto-Fix Tool

Automatically fixes "stuck" modifier keys (Ctrl, Shift, Alt, Win) caused by automation software.

## The Problem

When using keyboard automation tools (AutoHotkey, Quicker, etc.), modifier keys can get stuck in the virtual pressed state even after being physically released, causing unwanted key combinations.

**Example:**
- You run an AutoHotkey script that uses `Alt+X`
- Script finishes, but Alt remains "pressed" in the system
- When you press `X`, it triggers `Alt+X` instead of just `X`
- You must manually press Alt to "unstick" it

## The Solution

This tool monitors both physical and virtual key states in real-time and automatically releases stuck keys when you press any key.

## Features

- ✅ Real-time monitoring of physical and virtual key states
- ✅ Smart auto-fix (only when truly stuck, not during normal use)
- ✅ Protects against false positives (key mapping, scripts, etc.)
- ✅ Distinguishes between left and right modifier keys
- ✅ Statistics tracking
- ✅ Configurable threshold (default: 1000ms)
- ✅ Non-intrusive operation

## Quick Start

```powershell
# Build
xmake

# Run (requires administrator privileges)
.\run.ps1
```

**See [USER_GUIDE.md](USER_GUIDE.md) for detailed usage instructions.**

## How It Works

1. **Monitors** both physical (hardware) and virtual (system) key states
2. **Detects** when a modifier key is stuck (virtual pressed, physical released)
3. **Waits** for threshold period (1000ms) to avoid false positives
4. **Auto-fixes** when you press any key, by sending a release event

## Requirements

- Windows OS
- Interception driver installed
- Administrator privileges
- Visual Studio 2019 or later (for building)

## Project Structure

```
escModKey/
├── include/                          # Header files
│   ├── interception.h               # Interception driver API
│   ├── physical_key_detector.h      # Physical key detector module
│   └── virtual_key_detector.h       # Virtual key detector module
├── src/                             # Source files
│   ├── main.cpp                     # Main auto-fix program
│   ├── physical_key_detector.cpp    # Physical detector implementation
│   └── virtual_key_detector.cpp     # Virtual detector implementation
├── test/                            # Test programs
│   ├── test_physical_detector.cpp   # Physical detector test
│   └── test_virtual_detector.cpp    # Virtual detector test
├── lib/                             # Libraries
│   ├── interception.dll
│   └── interception.lib
├── USER_GUIDE.md                    # Detailed user guide
├── xmake.lua                        # Build configuration
└── run.ps1                          # Run script
```

## Modules

### PhysicalKeyDetector

Hardware-level detection of modifier key states via Interception driver.
- Cannot be fooled by software key simulation
- Event-driven, real-time detection
- Requires administrator privileges

### VirtualKeyDetector

Software-level detection of modifier key states via Windows API.
- Detects system's view of key states
- Polling-based (50ms interval)
- No special privileges required

## Building

```bash
xmake
```

This builds:
- `escModKey.exe` - Main auto-fix program
- `test_physical_detector.exe` - Physical detector test
- `test_virtual_detector.exe` - Virtual detector test

## Running

### Main Program (Auto-Fix)
```powershell
.\run.ps1
```

### Test Programs
```powershell
# Test physical detector (requires admin)
.\run_test.ps1

# Test virtual detector (no admin required)
.\run_test_virtual.ps1
```

## Configuration

Edit `src/main.cpp`:

```cpp
const int MISMATCH_THRESHOLD_MS = 1000;  // Adjust threshold
const bool SHOW_FIX_MESSAGES = true;     // Show/hide messages
```

## Use Cases

- Fix stuck keys after AutoHotkey scripts
- Recover from automation software glitches
- Prevent unwanted key combinations
- Improve workflow with automation tools

## Troubleshooting

**"Failed to create Interception context"**
- Install Interception driver
- Run with administrator privileges

**Keys still stuck after fix**
- Increase threshold value
- Check if automation software is still running

**False positives**
- Increase `MISMATCH_THRESHOLD_MS` to 2000+

See [USER_GUIDE.md](USER_GUIDE.md) for more troubleshooting tips.

## Documentation

- [USER_GUIDE.md](USER_GUIDE.md) - Detailed usage guide
- [USAGE.md](USAGE.md) - Physical detector API reference
- [USAGE_VIRTUAL.md](USAGE_VIRTUAL.md) - Virtual detector API reference

## License

See LICENSE file.
