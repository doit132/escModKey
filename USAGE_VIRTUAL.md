# Virtual Key Detector - Usage Guide

## Module Overview

The `VirtualKeyDetector` module provides software-level detection of modifier key states using Windows API.

## Quick Start

### 1. Include the header

```cpp
#include "virtual_key_detector.h"
```

### 2. Create and initialize detector

```cpp
VirtualKeyDetector detector;
// Detector is automatically initialized in constructor
```

### 3. Update key states

```cpp
// Call this periodically (e.g., in your main loop)
detector.update();
```

### 4. Query key states

```cpp
// Get full state structure
const VirtualKeyStates& states = detector.getStates();

// Check individual keys
if (states.lctrl) {
    // Left Ctrl is pressed
}

// Check combined states
if (detector.isCtrlPressed()) {
    // Either Left or Right Ctrl is pressed
}
```

## API Reference

### VirtualKeyStates Structure

```cpp
struct VirtualKeyStates {
  bool lctrl;   // Left Ctrl
  bool rctrl;   // Right Ctrl
  bool lshift;  // Left Shift
  bool rshift;  // Right Shift
  bool lalt;    // Left Alt
  bool ralt;    // Right Alt
  bool lwin;    // Left Win
  bool rwin;    // Right Win
  
  // Helper methods
  bool anyCtrl() const;   // Any Ctrl pressed
  bool anyShift() const;  // Any Shift pressed
  bool anyAlt() const;    // Any Alt pressed
  bool anyWin() const;    // Any Win pressed
  
  // Comparison operator
  bool operator!=(const VirtualKeyStates &other) const;
};
```

### VirtualKeyDetector Class

#### Methods

- `VirtualKeyDetector()` - Constructor, initializes detector
- `void initialize()` - Reset all key states to released
- `void update()` - Update all key states (call periodically)
- `const VirtualKeyStates& getStates() const` - Get current states
- `bool isCtrlPressed() const` - Check if any Ctrl is pressed
- `bool isShiftPressed() const` - Check if any Shift is pressed
- `bool isAltPressed() const` - Check if any Alt is pressed
- `bool isWinPressed() const` - Check if any Win is pressed
- `bool isLeftCtrlPressed() const` - Check if Left Ctrl is pressed
- `bool isRightCtrlPressed() const` - Check if Right Ctrl is pressed
- `bool isLeftShiftPressed() const` - Check if Left Shift is pressed
- `bool isRightShiftPressed() const` - Check if Right Shift is pressed
- `bool isLeftAltPressed() const` - Check if Left Alt is pressed
- `bool isRightAltPressed() const` - Check if Right Alt is pressed
- `bool isLeftWinPressed() const` - Check if Left Win is pressed
- `bool isRightWinPressed() const` - Check if Right Win is pressed

## Complete Example

```cpp
#include "virtual_key_detector.h"
#include <Windows.h>
#include <iostream>

int main() {
  VirtualKeyDetector detector;
  VirtualKeyStates previousStates;
  
  while (true) {
    // Update states
    detector.update();
    
    // Check if states changed
    if (detector.getStates() != previousStates) {
      // States changed, do something
      if (detector.isCtrlPressed()) {
        std::cout << "Ctrl is pressed!" << std::endl;
      }
      
      previousStates = detector.getStates();
    }
    
    // Check for exit
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
      break;
    }
    
    Sleep(50);
  }
  
  return 0;
}
```

## Comparison with Physical Key Detector

| Feature             | VirtualKeyDetector         | PhysicalKeyDetector            |
| ------------------- | -------------------------- | ------------------------------ |
| Detection Level     | Software (Windows API)     | Hardware (Interception driver) |
| Admin Required      | No                         | Yes                            |
| Software Simulation | Detectable                 | Not detectable                 |
| Driver Required     | No                         | Yes (Interception)             |
| Performance         | Polling-based              | Event-based                    |
| Use Case            | General key state checking | Anti-cheat, security           |

## Notes

- Uses `GetAsyncKeyState()` Windows API
- Polling-based, call `update()` regularly (e.g., every 50ms)
- Can detect software-simulated key presses
- No special privileges required
- Works at application level
- The detector distinguishes between left and right modifier keys

