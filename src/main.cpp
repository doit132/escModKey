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
  std::cout << "Total Fixes: " << fixer.getStatistics().getTotalFixes()
            << " | ";
  std::cout << "Status: " << (fixer.isPaused() ? "PAUSED" : "RUNNING")
            << std::endl;
  std::cout << "Press ESC to exit | Press P to pause/resume" << std::endl;
  std::cout << std::endl;

  const auto &pStates = fixer.getPhysicalStates();
  const auto &vStates = fixer.getVirtualStates();
  const auto &trackers = fixer.getMismatchTrackers();

  // Iterate through all monitored keys
  for (const auto &physKey : pStates.getKeys()) {
    // Find corresponding virtual key and tracker
    const VirtualKeyState *virtKey = vStates.findKeyById(physKey.id);
    const MismatchTracker *tracker = trackers.getTracker(physKey.id);

    if (!virtKey || !tracker) {
      continue; // Skip if not found
    }

    // Display key name
    std::cout << std::setw(12) << std::left << physKey.name << ": ";

    // Display physical state
    std::cout << "Physical[" << (physKey.pressed ? "PRESSED " : "RELEASED")
              << "] ";

    // Display virtual state
    std::cout << "Virtual[" << (virtKey->pressed ? "PRESSED " : "RELEASED")
              << "]";

    // Display mismatch info
    if (tracker->isMismatched) {
      int duration = tracker->getDurationMs();
      std::cout << " <-- MISMATCH (" << duration << "ms)";

      if (tracker->isStuck(fixer.getThreshold())) {
        std::cout << " [STUCK!]";
      }
    }

    std::cout << std::endl;
  }

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

  // Print key monitoring configuration (Step 5: now applied)
  std::cout << "\nKey Monitoring Configuration:" << std::endl;
  std::cout << "  Monitor Ctrl: " << (config.getMonitorCtrl() ? "Yes" : "No")
            << std::endl;
  std::cout << "  Monitor Shift: " << (config.getMonitorShift() ? "Yes" : "No")
            << std::endl;
  std::cout << "  Monitor Alt: " << (config.getMonitorAlt() ? "Yes" : "No")
            << std::endl;
  std::cout << "  Monitor Win: " << (config.getMonitorWin() ? "Yes" : "No")
            << std::endl;

  if (!config.getDisabledKeys().empty()) {
    std::cout << "  Disabled Keys: ";
    for (size_t i = 0; i < config.getDisabledKeys().size(); ++i) {
      if (i > 0)
        std::cout << ", ";
      std::cout << config.getDisabledKeys()[i];
    }
    std::cout << " (not implemented yet)" << std::endl;
  }

  if (!config.getCustomKeys().empty()) {
    std::cout << "  Custom Keys: " << config.getCustomKeys().size()
              << " defined (not implemented yet)" << std::endl;
  }

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
    if (trackers.lctrl().isMismatched || trackers.rctrl().isMismatched ||
        trackers.lshift().isMismatched || trackers.rshift().isMismatched ||
        trackers.lalt().isMismatched || trackers.ralt().isMismatched ||
        trackers.lwin().isMismatched || trackers.rwin().isMismatched) {
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
  std::cout << "  Total fixes: " << stats.getTotalFixes() << std::endl;

  // Display individual key statistics
  const auto &pStates = fixer.getPhysicalStates();
  for (const auto &key : pStates.getKeys()) {
    int fixCount = stats.getFixCount(key.id);
    if (fixCount > 0) {
      std::cout << "  " << key.name << ": " << fixCount << std::endl;
    }
  }

  std::cout << "\nProgram exited successfully." << std::endl;

  return 0;
}
