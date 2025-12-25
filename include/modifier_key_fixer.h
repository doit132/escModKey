#ifndef MODIFIER_KEY_FIXER_H
#define MODIFIER_KEY_FIXER_H

#include "config.h"
#include "interception.h"
#include "physical_key_detector.h"
#include "virtual_key_detector.h"
#include <chrono>

// Mismatch tracker for a single key
struct MismatchTracker {
  bool isMismatched = false;
  std::chrono::steady_clock::time_point startTime;

  void reset();
  void start();
  int getDurationMs() const;
  bool isStuck(int thresholdMs) const;
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

  bool hasAnyStuck(int thresholdMs) const;
};

// Fix statistics
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

// Main fixer class
class ModifierKeyFixer {
public:
  ModifierKeyFixer();
  ~ModifierKeyFixer();

  // Initialization
  bool initialize();
  bool initialize(const Config &config);
  void cleanup();

  // Main processing
  bool processEvents(int timeoutMs = 50);

  // State access
  const ModifierKeyStates &getPhysicalStates() const;
  const VirtualKeyStates &getVirtualStates() const;
  const ModifierMismatchTrackers &getMismatchTrackers() const;
  const FixStatistics &getStatistics() const;

  // Control
  void pause();
  void resume();
  bool isPaused() const { return paused_; }

  // Configuration
  void setThreshold(int ms) { thresholdMs_ = ms; }
  int getThreshold() const { return thresholdMs_; }
  void setShowMessages(bool show) { showMessages_ = show; }
  bool getShowMessages() const { return showMessages_; }
  void applyConfig(const Config &config);

  // Check if initialized
  bool isInitialized() const { return context_ != nullptr; }

private:
  // Detectors
  PhysicalKeyDetector physicalDetector_;
  VirtualKeyDetector virtualDetector_;

  // Trackers
  ModifierMismatchTrackers mismatchTrackers_;
  FixStatistics stats_;

  // Interception
  InterceptionContext context_;

  // Configuration
  int thresholdMs_;
  bool showMessages_;
  bool paused_;

  // Internal methods
  void updateMismatchTrackers();
  bool shouldCheckForFix(const InterceptionKeyStroke &stroke);
  int fixStuckKeys(InterceptionDevice device);
  void sendKeyRelease(InterceptionDevice device, unsigned short scanCode,
                      bool needsE0);
  bool hasOtherPhysicalKeyPressed(const InterceptionKeyStroke &stroke);
};

#endif // MODIFIER_KEY_FIXER_H
