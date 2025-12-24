#include "interception.h"
#include "physical_key_detector.h"
#include "virtual_key_detector.h"
#include <Windows.h>
#include <chrono>
#include <conio.h>
#include <iomanip>
#include <iostream>


// Configuration
const int MISMATCH_THRESHOLD_MS = 1000; // Time before considering a key "stuck"
const bool SHOW_FIX_MESSAGES =
    true; // Show fix messages (set to false for silent mode)

// Statistics
struct FixStatistics {
  int totalFixes = 0;
  int lctrlFixes = 0;
  int rctrlFixes = 0;
  int lshiftFixes = 0;
  int rshiftFixes = 0;
  int laltFixes = 0;
  int raltFixes = 0;
  int lwinFixes = 0;
  int rwinFixes = 0;
};

// Structure to track mismatch state for each modifier key
struct MismatchTracker {
  bool isMismatched = false;
  std::chrono::steady_clock::time_point startTime;

  void reset() { isMismatched = false; }

  void start() {
    if (!isMismatched) {
      isMismatched = true;
      startTime = std::chrono::steady_clock::now();
    }
  }

  int getDurationMs() const {
    if (!isMismatched)
      return 0;
    auto now = std::chrono::steady_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return static_cast<int>(duration.count());
  }

  bool isStuck() const {
    return isMismatched && getDurationMs() >= MISMATCH_THRESHOLD_MS;
  }
};

// Tracker for all modifier keys
struct ModifierMismatchTrackers {
  MismatchTracker lctrl;
  MismatchTracker rctrl;
  MismatchTracker lshift;
  MismatchTracker rshift;
  MismatchTracker lalt;
  MismatchTracker ralt;
  MismatchTracker lwin;
  MismatchTracker rwin;
};

// Check if any non-modifier physical key is pressed
bool hasOtherPhysicalKeyPressed(const InterceptionKeyStroke &currentStroke,
                                const ModifierKeyStates &physicalStates) {
  // Check if any modifier key is physically pressed
  if (physicalStates.lctrl || physicalStates.rctrl || physicalStates.lshift ||
      physicalStates.rshift || physicalStates.lalt || physicalStates.ralt ||
      physicalStates.lwin || physicalStates.rwin) {
    return true;
  }

  return false;
}

// Check if the stroke is a modifier key
bool isModifierKeyStroke(const InterceptionKeyStroke &stroke) {
  bool isE0 = stroke.state & INTERCEPTION_KEY_E0;

  switch (stroke.code) {
  case 0x1D: // Ctrl (left or right with E0)
  case 0x2A: // Left Shift
  case 0x36: // Right Shift
  case 0x38: // Alt (left or right with E0)
    return true;
  case 0x5B: // Left Win (with E0)
  case 0x5C: // Right Win (with E0)
    return isE0;
  default:
    return false;
  }
}

// Check if we should trigger fix check
bool shouldCheckForFix(const InterceptionKeyStroke &stroke,
                       const ModifierKeyStates &physicalStates,
                       const ModifierMismatchTrackers &trackers) {
  // Only trigger on key down
  if (stroke.state & INTERCEPTION_KEY_UP) {
    return false;
  }

  // Check if any key is stuck
  bool hasStuckKey = trackers.lctrl.isStuck() || trackers.rctrl.isStuck() ||
                     trackers.lshift.isStuck() || trackers.rshift.isStuck() ||
                     trackers.lalt.isStuck() || trackers.ralt.isStuck() ||
                     trackers.lwin.isStuck() || trackers.rwin.isStuck();

  if (!hasStuckKey) {
    return false;
  }

  // Check if there are other physical keys pressed
  // This helps avoid interfering with key mapping scenarios
  if (hasOtherPhysicalKeyPressed(stroke, physicalStates)) {
    return false;
  }

  return true;
}

// Send a key release event for a specific modifier key
void sendKeyRelease(InterceptionContext context, InterceptionDevice device,
                    unsigned short scanCode, bool needsE0) {
  InterceptionKeyStroke releaseStroke;
  releaseStroke.code = scanCode;
  releaseStroke.state = INTERCEPTION_KEY_UP;

  if (needsE0) {
    releaseStroke.state |= INTERCEPTION_KEY_E0;
  }

  releaseStroke.information = 0;

  // Send the release event
  interception_send(context, device, (InterceptionStroke *)&releaseStroke, 1);
}

// Fix stuck modifier keys
int fixStuckKeys(InterceptionContext context, InterceptionDevice device,
                 const ModifierMismatchTrackers &trackers,
                 VirtualKeyDetector &virtualDetector, FixStatistics &stats) {
  int fixedCount = 0;

  // Fix each stuck key
  if (trackers.lctrl.isStuck()) {
    sendKeyRelease(context, device, 0x1D, false);
    fixedCount++;
    stats.lctrlFixes++;
    if (SHOW_FIX_MESSAGES)
      std::cout << "  [Fixed] Left Ctrl" << std::endl;
  }

  if (trackers.rctrl.isStuck()) {
    sendKeyRelease(context, device, 0x1D, true);
    fixedCount++;
    stats.rctrlFixes++;
    if (SHOW_FIX_MESSAGES)
      std::cout << "  [Fixed] Right Ctrl" << std::endl;
  }

  if (trackers.lshift.isStuck()) {
    sendKeyRelease(context, device, 0x2A, false);
    fixedCount++;
    stats.lshiftFixes++;
    if (SHOW_FIX_MESSAGES)
      std::cout << "  [Fixed] Left Shift" << std::endl;
  }

  if (trackers.rshift.isStuck()) {
    sendKeyRelease(context, device, 0x36, false);
    fixedCount++;
    stats.rshiftFixes++;
    if (SHOW_FIX_MESSAGES)
      std::cout << "  [Fixed] Right Shift" << std::endl;
  }

  if (trackers.lalt.isStuck()) {
    sendKeyRelease(context, device, 0x38, false);
    fixedCount++;
    stats.laltFixes++;
    if (SHOW_FIX_MESSAGES)
      std::cout << "  [Fixed] Left Alt" << std::endl;
  }

  if (trackers.ralt.isStuck()) {
    sendKeyRelease(context, device, 0x38, true);
    fixedCount++;
    stats.raltFixes++;
    if (SHOW_FIX_MESSAGES)
      std::cout << "  [Fixed] Right Alt" << std::endl;
  }

  if (trackers.lwin.isStuck()) {
    sendKeyRelease(context, device, 0x5B, true);
    fixedCount++;
    stats.lwinFixes++;
    if (SHOW_FIX_MESSAGES)
      std::cout << "  [Fixed] Left Win" << std::endl;
  }

  if (trackers.rwin.isStuck()) {
    sendKeyRelease(context, device, 0x5C, true);
    fixedCount++;
    stats.rwinFixes++;
    if (SHOW_FIX_MESSAGES)
      std::cout << "  [Fixed] Right Win" << std::endl;
  }

  // Update statistics
  if (fixedCount > 0) {
    stats.totalFixes += fixedCount;

    // Small delay to let the system process the release events
    Sleep(20);
    virtualDetector.update();
  }

  return fixedCount;
}

// Update mismatch trackers based on current states
void updateMismatchTrackers(ModifierMismatchTrackers &trackers,
                            const ModifierKeyStates &physical,
                            const VirtualKeyStates &virtual_states) {
  // Check each key for mismatch (virtual pressed but physical released)
  auto checkKey = [](MismatchTracker &tracker, bool physical,
                     bool virtual_state) {
    if (!physical && virtual_state) {
      // Mismatch: virtual pressed, physical released
      tracker.start();
    } else {
      // States match or physical is pressed
      tracker.reset();
    }
  };

  checkKey(trackers.lctrl, physical.lctrl, virtual_states.lctrl);
  checkKey(trackers.rctrl, physical.rctrl, virtual_states.rctrl);
  checkKey(trackers.lshift, physical.lshift, virtual_states.lshift);
  checkKey(trackers.rshift, physical.rshift, virtual_states.rshift);
  checkKey(trackers.lalt, physical.lalt, virtual_states.lalt);
  checkKey(trackers.ralt, physical.ralt, virtual_states.ralt);
  checkKey(trackers.lwin, physical.lwin, virtual_states.lwin);
  checkKey(trackers.rwin, physical.rwin, virtual_states.rwin);
}

// Display both physical and virtual states with mismatch tracking
void displayStates(const PhysicalKeyDetector &physical,
                   const VirtualKeyDetector &virtual_detector,
                   const ModifierMismatchTrackers &trackers,
                   const FixStatistics &stats) {
  system("cls");

  std::cout << "=== Modifier Key Auto-Fix Monitor ===" << std::endl;
  std::cout << "Threshold: " << MISMATCH_THRESHOLD_MS << "ms | ";
  std::cout << "Total Fixes: " << stats.totalFixes << std::endl;
  std::cout << "Press ESC to exit" << std::endl;
  std::cout << std::endl;

  const auto &pStates = physical.getStates();
  const auto &vStates = virtual_detector.getStates();

  auto printKeyComparison = [](const char *name, bool physical,
                               bool virtual_state,
                               const MismatchTracker &tracker) {
    std::cout << std::setw(12) << std::left << name << ": ";
    std::cout << "Physical[" << (physical ? "PRESSED " : "RELEASED") << "] ";
    std::cout << "Virtual[" << (virtual_state ? "PRESSED " : "RELEASED") << "]";

    // Show mismatch info
    if (tracker.isMismatched) {
      int duration = tracker.getDurationMs();
      std::cout << " <-- MISMATCH (" << duration << "ms)";

      if (tracker.isStuck()) {
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

  bool hasStuck = trackers.lctrl.isStuck() || trackers.rctrl.isStuck() ||
                  trackers.lshift.isStuck() || trackers.rshift.isStuck() ||
                  trackers.lalt.isStuck() || trackers.ralt.isStuck() ||
                  trackers.lwin.isStuck() || trackers.rwin.isStuck();

  if (hasStuck) {
    std::cout << "Stuck keys detected! Press any key to auto-fix." << std::endl;
  } else {
    std::cout << "All keys normal. Monitoring..." << std::endl;
  }
}

int main() {
  std::cout << "=== Modifier Key Auto-Fix Tool ===" << std::endl;
  std::cout << "Initializing..." << std::endl;

  // Initialize physical key detector
  PhysicalKeyDetector physicalDetector;

  // Initialize virtual key detector
  VirtualKeyDetector virtualDetector;

  // Initialize mismatch trackers
  ModifierMismatchTrackers mismatchTrackers;

  // Initialize statistics
  FixStatistics stats;

  // Create Interception context
  InterceptionContext context = interception_create_context();

  if (!context) {
    std::cerr << "\nERROR: Failed to create Interception context" << std::endl;
    std::cerr << "Please ensure:" << std::endl;
    std::cerr << "1. Interception driver is installed" << std::endl;
    std::cerr << "2. Running with administrator privileges" << std::endl;
    std::cout << "\nPress any key to exit..." << std::endl;
    _getch();
    return 1;
  }

  // Set filter for all keyboard events
  interception_set_filter(
      context, interception_is_keyboard,
      INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP |
          INTERCEPTION_FILTER_KEY_E0 | INTERCEPTION_FILTER_KEY_E1);

  std::cout << "Initialized successfully!" << std::endl;
  std::cout << "Auto-fix will trigger when you press any key after a modifier "
               "key is stuck."
            << std::endl;
  Sleep(2000);

  // Track previous states to detect changes
  ModifierKeyStates prevPhysicalStates;
  VirtualKeyStates prevVirtualStates;

  displayStates(physicalDetector, virtualDetector, mismatchTrackers, stats);

  // Main loop
  while (true) {
    InterceptionDevice device = interception_wait_with_timeout(context, 50);

    bool stateChanged = false;

    if (device > 0) {
      if (interception_is_keyboard(device)) {
        InterceptionKeyStroke stroke;

        if (interception_receive(context, device, (InterceptionStroke *)&stroke,
                                 1) > 0) {
          // Check for ESC key to exit
          if (stroke.code == 0x01 && !(stroke.state & INTERCEPTION_KEY_UP)) {
            std::cout << "\nExiting..." << std::endl;
            std::cout << "Total fixes performed: " << stats.totalFixes
                      << std::endl;
            interception_send(context, device, (InterceptionStroke *)&stroke,
                              1);
            break;
          }

          // Check if we should trigger fix (before updating physical state)
          if (shouldCheckForFix(stroke, physicalDetector.getStates(),
                                mismatchTrackers)) {
            if (SHOW_FIX_MESSAGES) {
              std::cout << "\n[Auto-Fix Triggered]" << std::endl;
            }

            int fixedCount = fixStuckKeys(context, device, mismatchTrackers,
                                          virtualDetector, stats);

            if (SHOW_FIX_MESSAGES && fixedCount > 0) {
              std::cout << "[Auto-Fix] Fixed " << fixedCount << " key(s)"
                        << std::endl;
              Sleep(300); // Brief pause to show message
            }

            stateChanged = true;
          }

          // Update physical key state
          physicalDetector.processKeyStroke(stroke);
          stateChanged = true;

          // Forward key event
          interception_send(context, device, (InterceptionStroke *)&stroke, 1);
        }
      }
    }

    // Update virtual key state periodically
    virtualDetector.update();

    // Update mismatch trackers
    updateMismatchTrackers(mismatchTrackers, physicalDetector.getStates(),
                           virtualDetector.getStates());

    // Check if virtual state changed
    if (virtualDetector.getStates() != prevVirtualStates) {
      stateChanged = true;
    }

    // Check if physical state changed
    if (physicalDetector.getStates() != prevPhysicalStates) {
      stateChanged = true;
    }

    // Also update display if any key is stuck (to show duration update)
    if (mismatchTrackers.lctrl.isMismatched ||
        mismatchTrackers.rctrl.isMismatched ||
        mismatchTrackers.lshift.isMismatched ||
        mismatchTrackers.rshift.isMismatched ||
        mismatchTrackers.lalt.isMismatched ||
        mismatchTrackers.ralt.isMismatched ||
        mismatchTrackers.lwin.isMismatched ||
        mismatchTrackers.rwin.isMismatched) {
      stateChanged = true;
    }

    // Only display if state changed
    if (stateChanged) {
      displayStates(physicalDetector, virtualDetector, mismatchTrackers, stats);
      prevPhysicalStates = physicalDetector.getStates();
      prevVirtualStates = virtualDetector.getStates();
    }
  }

  // Cleanup
  interception_destroy_context(context);
  std::cout << "\nProgram exited successfully." << std::endl;
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

  return 0;
}
