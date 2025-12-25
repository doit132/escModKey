#include "config.h"
#include "modifier_key_fixer.h"
#include <Windows.h>
#include <conio.h>
#include <iomanip>
#include <iostream>

// Display current states
void displayStates(const ModifierKeyFixer &fixer) {
  system("cls");

  std::cout << "=== Modifier Key Auto-Fix Monitor ===" << std::endl;
  std::cout << "Threshold: " << fixer.getThreshold() << "ms | ";
  std::cout << "Total Fixes: " << fixer.getStatistics().totalFixes << " | ";
  std::cout << "Status: " << (fixer.isPaused() ? "PAUSED" : "RUNNING")
            << std::endl;
  std::cout << "Press ESC to exit | Press P to pause/resume" << std::endl;
  std::cout << std::endl;

  const auto &pStates = fixer.getPhysicalStates();
  const auto &vStates = fixer.getVirtualStates();
  const auto &trackers = fixer.getMismatchTrackers();

  auto printKeyComparison = [&fixer](const char *name, bool physical,
                                     bool virtual_state,
                                     const MismatchTracker &tracker) {
    std::cout << std::setw(12) << std::left << name << ": ";
    std::cout << "Physical[" << (physical ? "PRESSED " : "RELEASED") << "] ";
    std::cout << "Virtual[" << (virtual_state ? "PRESSED " : "RELEASED") << "]";

    if (tracker.isMismatched) {
      int duration = tracker.getDurationMs();
      std::cout << " <-- MISMATCH (" << duration << "ms)";

      if (tracker.isStuck(fixer.getThreshold())) {
        std::cout << " [STUCK!]";
      }
    }
    std::cout << std::endl;
  };

  printKeyComparison("Left Ctrl", pStates.lctrl, vStates.lctrl, trackers.lctrl);
  printKeyComparison("Right Ctrl", pStates.rctrl, vStates.rctrl,
                     trackers.rctrl);
  printKeyComparison("Left Shift", pStates.lshift, vStates.lshift,
                     trackers.lshift);
  printKeyComparison("Right Shift", pStates.rshift, vStates.rshift,
                     trackers.rshift);
  printKeyComparison("Left Alt", pStates.lalt, vStates.lalt, trackers.lalt);
  printKeyComparison("Right Alt", pStates.ralt, vStates.ralt, trackers.ralt);
  printKeyComparison("Left Win", pStates.lwin, vStates.lwin, trackers.lwin);
  printKeyComparison("Right Win", pStates.rwin, vStates.rwin, trackers.rwin);

  std::cout << std::endl;
  std::cout << "Status: ";

  if (fixer.isPaused()) {
    std::cout << "Monitoring PAUSED. Press P to resume." << std::endl;
  } else if (trackers.hasAnyStuck(fixer.getThreshold())) {
    std::cout << "Stuck keys detected! Press any key to auto-fix." << std::endl;
  } else {
    std::cout << "All keys normal. Monitoring..." << std::endl;
  }
}

int main() {
  std::cout << "=== Modifier Key Auto-Fix Tool ===" << std::endl;
  std::cout << "Initializing..." << std::endl;

  // Load configuration
  Config config;
  std::string configPath = Config::getDefaultConfigPath();
  if (!config.load(configPath)) {
    // Config file doesn't exist or has errors, use defaults
    config.loadDefaults();
    // Try to save default config
    config.save(configPath);
  }

  std::cout << "Configuration loaded from: " << configPath << std::endl;
  std::cout << "Threshold: " << config.getThresholdMs() << "ms" << std::endl;
  std::cout << std::endl;

  // Create and initialize fixer with config
  ModifierKeyFixer fixer;

  if (!fixer.initialize(config)) {
    std::cerr << "\nERROR: Failed to initialize" << std::endl;
    std::cerr << "Please ensure:" << std::endl;
    std::cerr << "1. Interception driver is installed" << std::endl;
    std::cerr << "2. Running with administrator privileges" << std::endl;
    std::cout << "\nPress any key to exit..." << std::endl;
    _getch();
    return 1;
  }

  std::cout << "Initialized successfully!" << std::endl;
  std::cout << "Auto-fix will trigger when you press any key after a modifier "
               "key is stuck."
            << std::endl;
  Sleep(2000);

  // Track previous states for display updates
  ModifierKeyStates prevPhysicalStates;
  VirtualKeyStates prevVirtualStates;
  bool prevPaused = false;

  displayStates(fixer);

  // Main loop
  bool running = true;
  while (running) {
    // Check for ESC or P key FIRST (before processing events)
    if (_kbhit()) {
      int ch = _getch();
      if (ch == 27) { // ESC
        running = false;
        continue;
      } else if (ch == 'p' || ch == 'P') {
        // Toggle pause state
        if (fixer.isPaused()) {
          fixer.resume();
        } else {
          fixer.pause();
        }
        // Force display update immediately
        displayStates(fixer);
        prevPhysicalStates = fixer.getPhysicalStates();
        prevVirtualStates = fixer.getVirtualStates();
        prevPaused = fixer.isPaused();
        continue;
      }
    }

    // Process events
    fixer.processEvents(50);

    // Check if state changed
    bool stateChanged = false;

    if (fixer.getVirtualStates() != prevVirtualStates) {
      stateChanged = true;
    }

    if (fixer.getPhysicalStates() != prevPhysicalStates) {
      stateChanged = true;
    }

    if (fixer.isPaused() != prevPaused) {
      stateChanged = true;
    }

    // Update display if any key is mismatched
    const auto &trackers = fixer.getMismatchTrackers();
    if (trackers.lctrl.isMismatched || trackers.rctrl.isMismatched ||
        trackers.lshift.isMismatched || trackers.rshift.isMismatched ||
        trackers.lalt.isMismatched || trackers.ralt.isMismatched ||
        trackers.lwin.isMismatched || trackers.rwin.isMismatched) {
      stateChanged = true;
    }

    if (stateChanged) {
      displayStates(fixer);
      prevPhysicalStates = fixer.getPhysicalStates();
      prevVirtualStates = fixer.getVirtualStates();
      prevPaused = fixer.isPaused();
    }
  }

  // Cleanup and show statistics
  std::cout << "\nExiting..." << std::endl;

  const auto &stats = fixer.getStatistics();
  std::cout << "\nFix Statistics:" << std::endl;
  std::cout << "  Total fixes: " << stats.totalFixes << std::endl;
  if (stats.lctrlFixes > 0)
    std::cout << "  Left Ctrl: " << stats.lctrlFixes << std::endl;
  if (stats.rctrlFixes > 0)
    std::cout << "  Right Ctrl: " << stats.rctrlFixes << std::endl;
  if (stats.lshiftFixes > 0)
    std::cout << "  Left Shift: " << stats.lshiftFixes << std::endl;
  if (stats.rshiftFixes > 0)
    std::cout << "  Right Shift: " << stats.rshiftFixes << std::endl;
  if (stats.laltFixes > 0)
    std::cout << "  Left Alt: " << stats.laltFixes << std::endl;
  if (stats.raltFixes > 0)
    std::cout << "  Right Alt: " << stats.raltFixes << std::endl;
  if (stats.lwinFixes > 0)
    std::cout << "  Left Win: " << stats.lwinFixes << std::endl;
  if (stats.rwinFixes > 0)
    std::cout << "  Right Win: " << stats.rwinFixes << std::endl;

  std::cout << "\nProgram exited successfully." << std::endl;

  return 0;
}
