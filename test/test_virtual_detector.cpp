#include "virtual_key_detector.h"
#include <Windows.h>
#include <conio.h>
#include <iomanip>
#include <iostream>

// Display virtual key states
void displayVirtualKeyStates(const VirtualKeyStates &states) {
  system("cls");

  std::cout << "=== Virtual Key State Detection ===" << std::endl;
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
  std::cout << "Virtual Key Detector Test" << std::endl;
  std::cout << "Initializing..." << std::endl;
  Sleep(1000);

  VirtualKeyDetector detector;
  VirtualKeyStates previousStates;

  std::cout << "Monitoring virtual key states..." << std::endl;
  std::cout << "Press any modifier key to see the state" << std::endl;
  std::cout << "Press ESC to exit" << std::endl;
  Sleep(2000);

  // Display initial state
  displayVirtualKeyStates(detector.getStates());

  // Main loop
  while (true) {
    // Update virtual key states
    detector.update();

    // Only display if states have changed
    if (detector.getStates() != previousStates) {
      displayVirtualKeyStates(detector.getStates());
      previousStates = detector.getStates();
    }

    // Check for ESC key to exit
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
      std::cout << "\nESC detected, exiting..." << std::endl;
      break;
    }

    // Small delay to reduce CPU usage
    Sleep(50);
  }

  std::cout << "Program exited" << std::endl;
  return 0;
}
