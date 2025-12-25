#ifndef MODIFIER_KEY_FIXER_H
#define MODIFIER_KEY_FIXER_H

#include "config.h"
#include "interception.h"
#include "physical_key_detector.h"
#include "virtual_key_detector.h"
#include <chrono>
#include <map>
#include <string>

// Mismatch tracker for a single key
struct MismatchTracker {
  bool isMismatched = false;
  std::chrono::steady_clock::time_point startTime;

  void reset();
  void start();
  int getDurationMs() const;
  bool isStuck(int thresholdMs) const;
};

// Tracker for all modifier keys (dynamic, supports any number of keys)
// Automatically tracks mismatch state for all monitored keys
// Can be initialized with custom key lists
class ModifierMismatchTrackers {
public:
  ModifierMismatchTrackers();

  // Initialize trackers for given key IDs
  void initializeForKeys(const std::vector<std::string> &keyIds);

  // Get tracker by key ID
  MismatchTracker *getTracker(const std::string &keyId);
  const MismatchTracker *getTracker(const std::string &keyId) const;

  // Check if any key is stuck
  bool hasAnyStuck(int thresholdMs) const;

  // Backward compatibility: access by field name
  const MismatchTracker &lctrl() const;
  const MismatchTracker &rctrl() const;
  const MismatchTracker &lshift() const;
  const MismatchTracker &rshift() const;
  const MismatchTracker &lalt() const;
  const MismatchTracker &ralt() const;
  const MismatchTracker &lwin() const;
  const MismatchTracker &rwin() const;

private:
  std::map<std::string, MismatchTracker> trackers_;
  MismatchTracker emptyTracker_; // For backward compatibility
};

// Fix statistics (dynamic, supports any number of keys)
// Automatically tracks fix counts for all monitored keys
// Can be initialized with custom key lists
class FixStatistics {
public:
  FixStatistics();

  // Initialize statistics for given key IDs
  void initializeForKeys(const std::vector<std::string> &keyIds);

  // Increment fix count for a key
  void incrementFix(const std::string &keyId);

  // Get fix count for a key
  int getFixCount(const std::string &keyId) const;

  // Get total fixes
  int getTotalFixes() const { return totalFixes_; }

  // Reset all statistics
  void reset();

  // Backward compatibility: access by field name
  int lctrlFixes() const;
  int rctrlFixes() const;
  int lshiftFixes() const;
  int rshiftFixes() const;
  int laltFixes() const;
  int raltFixes() const;
  int lwinFixes() const;
  int rwinFixes() const;

  // Get all fix counts
  const std::map<std::string, int> &getAllFixes() const { return fixes_; }

private:
  int totalFixes_;
  std::map<std::string, int> fixes_;
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
  bool initializeCommon();
  void updateMismatchTrackers();
  bool shouldCheckForFix(const InterceptionKeyStroke &stroke);
  int fixStuckKeys(InterceptionDevice device);
  void sendKeyRelease(InterceptionDevice device, unsigned short scanCode,
                      bool needsE0);
  bool hasOtherPhysicalKeyPressed(const InterceptionKeyStroke &stroke);
};

#endif // MODIFIER_KEY_FIXER_H
