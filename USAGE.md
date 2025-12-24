# Physical Key Detector - Usage Guide

## Module Overview

The `PhysicalKeyDetector` module provides hardware-level detection of modifier key states using the Interception driver.

## Quick Start

### 1. Include the header

```cpp
#include "physical_key_detector.h"
```

### 2. Create and initialize detector

```cpp
PhysicalKeyDetector detector;
// Detector is automatically initialized in constructor
```

### 3. Process key strokes

```cpp
InterceptionKeyStroke stroke;
// ... receive stroke from Interception driver ...

detector.processKeyStroke(stroke);
```

### 4. Query key states

```cpp
// Get full state structure
const ModifierKeyStates& states = detector.getStates();

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

### ModifierKeyStates Structure

```cpp
struct ModifierKeyStates {
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
};
```

### PhysicalKeyDetector Class

#### Methods

- `PhysicalKeyDetector()` - Constructor, initializes detector
- `void initialize()` - Reset all key states to released
- `void processKeyStroke(const InterceptionKeyStroke &stroke)` - Process a key event
- `const ModifierKeyStates& getStates() const` - Get current states
- `bool isCtrlPressed() const` - Check if any Ctrl is pressed
- `bool isShiftPressed() const` - Check if any Shift is pressed
- `bool isAltPressed() const` - Check if any Alt is pressed
- `bool isWinPressed() const` - Check if any Win is pressed

## Complete Example

```cpp
#include "interception.h"
#include "physical_key_detector.h"
#include <iostream>

int main() {
  // Create Interception context
  InterceptionContext context = interception_create_context();
  if (!context) {
    std::cerr << "Failed to create context" << std::endl;
    return 1;
  }
  
  // Set filter
  interception_set_filter(
      context, interception_is_keyboard,
      INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP |
      INTERCEPTION_FILTER_KEY_E0 | INTERCEPTION_FILTER_KEY_E1);
  
  // Create detector
  PhysicalKeyDetector detector;
  
  // Main loop
  while (true) {
    InterceptionDevice device = interception_wait(context);
    
    if (interception_is_keyboard(device)) {
      InterceptionKeyStroke stroke;
      
      if (interception_receive(context, device, 
                               (InterceptionStroke *)&stroke, 1) > 0) {
        // Process the key stroke
        detector.processKeyStroke(stroke);
        
        // Check states
        if (detector.isCtrlPressed()) {
          std::cout << "Ctrl is pressed!" << std::endl;
        }
        
        // Forward the key
        interception_send(context, device, 
                         (InterceptionStroke *)&stroke, 1);
      }
    }
  }
  
  interception_destroy_context(context);
  return 0;
}
```

## Notes

- The detector only tracks modifier keys (Ctrl, Shift, Alt, Win)
- All other keys are ignored by `processKeyStroke()`
- Key states are updated in real-time as events are processed
- The detector distinguishes between left and right modifier keys
- Requires Interception driver and administrator privileges
