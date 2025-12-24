#include "interception.h"
#include <Windows.h>
#include <conio.h>
#include <iomanip>
#include <iostream>

// Modifier key scan codes (based on actual testing)
#define SCANCODE_LCTRL 0x1D
#define SCANCODE_RCTRL 0x1D // with E0 flag
#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_LALT 0x38
#define SCANCODE_RALT 0x38 // with E0 flag
#define SCANCODE_LWIN 0x5B // with E0 flag
#define SCANCODE_RWIN 0x5C // with E0 flag

// Modifier key states structure
struct ModifierKeyStates {
  bool lctrl = false;
  bool rctrl = false;
  bool lshift = false;
  bool rshift = false;
  bool lalt = false;
  bool ralt = false;
  bool lwin = false;
  bool rwin = false;
};

// Update modifier key state
void updateModifierState(ModifierKeyStates &states,
                         const InterceptionKeyStroke &stroke) {
  bool isPressed = !(stroke.state & INTERCEPTION_KEY_UP);
  bool isE0 = stroke.state & INTERCEPTION_KEY_E0;

  switch (stroke.code) {
  case SCANCODE_LCTRL:
    if (isE0) {
      states.rctrl = isPressed;
    } else {
      states.lctrl = isPressed;
    }
    break;
  case SCANCODE_LSHIFT:
    states.lshift = isPressed;
    break;
  case SCANCODE_RSHIFT:
    states.rshift = isPressed;
    break;
  case SCANCODE_LALT:
    if (isE0) {
      states.ralt = isPressed;
    } else {
      states.lalt = isPressed;
    }
    break;
  case SCANCODE_LWIN:
    if (isE0) {
      states.lwin = isPressed;
    }
    break;
  case SCANCODE_RWIN:
    if (isE0) {
      states.rwin = isPressed;
    }
    break;
  }
}

// Display modifier key states
void displayModifierStates(const ModifierKeyStates &states) {
  system("cls");

  std::cout << "=== Modifier Key Physical State Detection ===" << std::endl;
  std::cout << "Press ESC to exit" << std::endl;
  std::cout << std::endl;

  auto printKey = [](const char *name, bool pressed) {
    std::cout << std::setw(12) << std::left << name << ": ";
    if (pressed) {
      std::cout << "[PRESSED]" << std::endl;
    } else {
      std::cout << "[RELEASED]" << std::endl;
    }
  };

  printKey("Left Ctrl", states.lctrl);
  printKey("Right Ctrl", states.rctrl);
  printKey("Left Shift", states.lshift);
  printKey("Right Shift", states.rshift);
  printKey("Left Alt", states.lalt);
  printKey("Right Alt", states.ralt);
  printKey("Left Win", states.lwin);
  printKey("Right Win", states.rwin);

  std::cout << std::endl;
  std::cout << "Combined: ";
  if (states.lctrl || states.rctrl)
    std::cout << "Ctrl ";
  if (states.lshift || states.rshift)
    std::cout << "Shift ";
  if (states.lalt || states.ralt)
    std::cout << "Alt ";
  if (states.lwin || states.rwin)
    std::cout << "Win ";
  std::cout << std::endl;
}

// Check if key is a modifier key
bool isModifierKey(const InterceptionKeyStroke &stroke) {
  switch (stroke.code) {
  case SCANCODE_LCTRL:
  case SCANCODE_LSHIFT:
  case SCANCODE_RSHIFT:
  case SCANCODE_LALT:
  case SCANCODE_LWIN:
  case SCANCODE_RWIN:
    return true;
  default:
    return false;
  }
}

int main() {
  std::cout << "Initializing Interception driver..." << std::endl;

  // Create Interception context
  InterceptionContext context = interception_create_context();

  if (!context) {
    std::cerr << "ERROR: Failed to create Interception context" << std::endl;
    std::cerr << "Please ensure:" << std::endl;
    std::cerr << "1. Interception driver is installed" << std::endl;
    std::cerr << "2. Running with administrator privileges" << std::endl;
    std::cout << "Press any key to exit..." << std::endl;
    _getch();
    return 1;
  }

  // Set filter to listen for all keyboard events including extended keys
  interception_set_filter(
      context, interception_is_keyboard,
      INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP |
          INTERCEPTION_FILTER_KEY_E0 | INTERCEPTION_FILTER_KEY_E1);

  ModifierKeyStates states;
  displayModifierStates(states);

  std::cout << "Driver initialized successfully, listening..." << std::endl;
  Sleep(1000);
  displayModifierStates(states);

  // Main loop
  while (true) {
    InterceptionDevice device = interception_wait(context);

    if (interception_is_keyboard(device)) {
      InterceptionKeyStroke stroke;

      if (interception_receive(context, device, (InterceptionStroke *)&stroke,
                               1) > 0) {
        // Check for ESC key to exit
        if (stroke.code == 0x01 && !(stroke.state & INTERCEPTION_KEY_UP)) {
          std::cout << "\nESC detected, exiting..." << std::endl;
          interception_send(context, device, (InterceptionStroke *)&stroke, 1);
          break;
        }

        // Update modifier key state
        if (isModifierKey(stroke)) {
          updateModifierState(states, stroke);
          displayModifierStates(states);
        }

        // Forward key event to not block normal input
        interception_send(context, device, (InterceptionStroke *)&stroke, 1);
      }
    }
  }

  // Cleanup
  interception_destroy_context(context);
  std::cout << "Program exited" << std::endl;

  return 0;
}
