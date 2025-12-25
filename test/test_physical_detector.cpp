#include "interception.h"
#include "physical_key_detector.h"
#include <Windows.h>
#include <conio.h>
#include <iomanip>
#include <iostream>

// Display modifier key states
void displayModifierStates(const ModifierKeyStates &states) {
  system("cls");

  std::cout << "=== Modifier Key Physical State Detection ===" << std::endl;
  std::cout << "Press ESC to exit" << std::endl;
  std::cout << std::endl;

  // Display all monitored keys dynamically
  for (const auto &key : states.getKeys()) {
    std::cout << std::setw(12) << std::left << key.name << ": ";
    if (key.pressed) {
      std::cout << "[PRESSED]" << std::endl;
    } else {
      std::cout << "[RELEASED]" << std::endl;
    }
  }

  std::cout << std::endl;
  std::cout << "Combined: ";
  if (states.anyCtrl())
    std::cout << "Ctrl ";
  if (states.anyShift())
    std::cout << "Shift ";
  if (states.anyAlt())
    std::cout << "Alt ";
  if (states.anyWin())
    std::cout << "Win ";
  std::cout << std::endl;
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

  // Initialize physical key detector
  PhysicalKeyDetector detector;
  displayModifierStates(detector.getStates());

  std::cout << "Driver initialized successfully, listening..." << std::endl;
  Sleep(1000);
  displayModifierStates(detector.getStates());

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

        // Process key stroke with detector
        detector.processKeyStroke(stroke);
        displayModifierStates(detector.getStates());

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
