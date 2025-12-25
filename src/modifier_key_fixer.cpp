#include "modifier_key_fixer.h"
#include <Windows.h>
#include <iostream>

// MismatchTracker implementation
void MismatchTracker::reset() { isMismatched = false; }

void MismatchTracker::start() {
  if (!isMismatched) {
    isMismatched = true;
    startTime = std::chrono::steady_clock::now();
  }
}

int MismatchTracker::getDurationMs() const {
  if (!isMismatched)
    return 0;
  auto now = std::chrono::steady_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
  return static_cast<int>(duration.count());
}

bool MismatchTracker::isStuck(int thresholdMs) const {
  return isMismatched && getDurationMs() >= thresholdMs;
}

// ModifierMismatchTrackers implementation
ModifierMismatchTrackers::ModifierMismatchTrackers() {
  // Empty constructor, will be initialized by initializeForKeys
}

void ModifierMismatchTrackers::initializeForKeys(
    const std::vector<std::string> &keyIds) {
  trackers_.clear();
  for (const auto &keyId : keyIds) {
    trackers_[keyId] = MismatchTracker();
  }
}

MismatchTracker *
ModifierMismatchTrackers::getTracker(const std::string &keyId) {
  auto it = trackers_.find(keyId);
  return (it != trackers_.end()) ? &it->second : nullptr;
}

const MismatchTracker *
ModifierMismatchTrackers::getTracker(const std::string &keyId) const {
  auto it = trackers_.find(keyId);
  return (it != trackers_.end()) ? &it->second : nullptr;
}

bool ModifierMismatchTrackers::hasAnyStuck(int thresholdMs) const {
  for (const auto &pair : trackers_) {
    if (pair.second.isStuck(thresholdMs)) {
      return true;
    }
  }
  return false;
}

// Backward compatibility methods
const MismatchTracker &ModifierMismatchTrackers::lctrl() const {
  const MismatchTracker *tracker = getTracker("lctrl");
  return tracker ? *tracker : emptyTracker_;
}

const MismatchTracker &ModifierMismatchTrackers::rctrl() const {
  const MismatchTracker *tracker = getTracker("rctrl");
  return tracker ? *tracker : emptyTracker_;
}

const MismatchTracker &ModifierMismatchTrackers::lshift() const {
  const MismatchTracker *tracker = getTracker("lshift");
  return tracker ? *tracker : emptyTracker_;
}

const MismatchTracker &ModifierMismatchTrackers::rshift() const {
  const MismatchTracker *tracker = getTracker("rshift");
  return tracker ? *tracker : emptyTracker_;
}

const MismatchTracker &ModifierMismatchTrackers::lalt() const {
  const MismatchTracker *tracker = getTracker("lalt");
  return tracker ? *tracker : emptyTracker_;
}

const MismatchTracker &ModifierMismatchTrackers::ralt() const {
  const MismatchTracker *tracker = getTracker("ralt");
  return tracker ? *tracker : emptyTracker_;
}

const MismatchTracker &ModifierMismatchTrackers::lwin() const {
  const MismatchTracker *tracker = getTracker("lwin");
  return tracker ? *tracker : emptyTracker_;
}

const MismatchTracker &ModifierMismatchTrackers::rwin() const {
  const MismatchTracker *tracker = getTracker("rwin");
  return tracker ? *tracker : emptyTracker_;
}

// FixStatistics implementation
FixStatistics::FixStatistics() : totalFixes_(0) {
  // Empty constructor, will be initialized by initializeForKeys
}

void FixStatistics::initializeForKeys(const std::vector<std::string> &keyIds) {
  fixes_.clear();
  totalFixes_ = 0;
  for (const auto &keyId : keyIds) {
    fixes_[keyId] = 0;
  }
}

void FixStatistics::incrementFix(const std::string &keyId) {
  totalFixes_++;
  auto it = fixes_.find(keyId);
  if (it != fixes_.end()) {
    it->second++;
  } else {
    fixes_[keyId] = 1;
  }
}

int FixStatistics::getFixCount(const std::string &keyId) const {
  auto it = fixes_.find(keyId);
  return (it != fixes_.end()) ? it->second : 0;
}

void FixStatistics::reset() {
  totalFixes_ = 0;
  for (auto &pair : fixes_) {
    pair.second = 0;
  }
}

// Backward compatibility methods
int FixStatistics::lctrlFixes() const { return getFixCount("lctrl"); }
int FixStatistics::rctrlFixes() const { return getFixCount("rctrl"); }
int FixStatistics::lshiftFixes() const { return getFixCount("lshift"); }
int FixStatistics::rshiftFixes() const { return getFixCount("rshift"); }
int FixStatistics::laltFixes() const { return getFixCount("lalt"); }
int FixStatistics::raltFixes() const { return getFixCount("ralt"); }
int FixStatistics::lwinFixes() const { return getFixCount("lwin"); }
int FixStatistics::rwinFixes() const { return getFixCount("rwin"); }

// ModifierKeyFixer implementation
ModifierKeyFixer::ModifierKeyFixer()
    : context_(nullptr), thresholdMs_(1000), showMessages_(true),
      paused_(false) {}

ModifierKeyFixer::~ModifierKeyFixer() { cleanup(); }

bool ModifierKeyFixer::initialize() {
  // Create Interception context
  context_ = interception_create_context();

  if (!context_) {
    return false;
  }

  // Set filter to listen for all keyboard events
  interception_set_filter(
      context_, interception_is_keyboard,
      INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP |
          INTERCEPTION_FILTER_KEY_E0 | INTERCEPTION_FILTER_KEY_E1);

  // Initialize detectors
  physicalDetector_.initialize();
  virtualDetector_.initialize();

  // Initialize trackers and statistics based on monitored keys
  std::vector<std::string> keyIds;
  for (const auto &key : physicalDetector_.getStates().getKeys()) {
    keyIds.push_back(key.id);
  }
  mismatchTrackers_.initializeForKeys(keyIds);
  stats_.initializeForKeys(keyIds);

  return true;
}

bool ModifierKeyFixer::initialize(const Config &config) {
  if (!initialize()) {
    return false;
  }
  applyConfig(config);
  return true;
}

void ModifierKeyFixer::applyConfig(const Config &config) {
  thresholdMs_ = config.getThresholdMs();
  showMessages_ = config.getShowMessages();
}

void ModifierKeyFixer::cleanup() {
  if (context_) {
    interception_destroy_context(context_);
    context_ = nullptr;
  }
}

bool ModifierKeyFixer::processEvents(int timeoutMs) {
  if (!context_) {
    Sleep(timeoutMs);
    return true;
  }

  InterceptionDevice device =
      interception_wait_with_timeout(context_, timeoutMs);

  if (device > 0) {
    if (interception_is_keyboard(device)) {
      InterceptionKeyStroke stroke;

      if (interception_receive(context_, device, (InterceptionStroke *)&stroke,
                               1) > 0) {

        // If paused, just forward the key and don't process
        if (paused_) {
          interception_send(context_, device, (InterceptionStroke *)&stroke, 1);
        } else {
          // Check for fix trigger (before updating physical state)
          if (shouldCheckForFix(stroke)) {
            if (showMessages_) {
              std::cout << "\n[Auto-Fix Triggered]" << std::endl;
            }

            int fixedCount = fixStuckKeys(device);

            if (showMessages_ && fixedCount > 0) {
              std::cout << "[Auto-Fix] Fixed " << fixedCount << " key(s)"
                        << std::endl;
            }
          }

          // Update physical key state
          physicalDetector_.processKeyStroke(stroke);

          // Forward key event
          interception_send(context_, device, (InterceptionStroke *)&stroke, 1);
        }
      }
    }
  }

  // Always update virtual state
  virtualDetector_.update();

  // Update mismatch trackers
  updateMismatchTrackers();

  return true;
}

const ModifierKeyStates &ModifierKeyFixer::getPhysicalStates() const {
  return physicalDetector_.getStates();
}

const VirtualKeyStates &ModifierKeyFixer::getVirtualStates() const {
  return virtualDetector_.getStates();
}

const ModifierMismatchTrackers &ModifierKeyFixer::getMismatchTrackers() const {
  return mismatchTrackers_;
}

const FixStatistics &ModifierKeyFixer::getStatistics() const { return stats_; }

void ModifierKeyFixer::pause() { paused_ = true; }

void ModifierKeyFixer::resume() { paused_ = false; }

void ModifierKeyFixer::updateMismatchTrackers() {
  const auto &physical = physicalDetector_.getStates();
  const auto &virtual_states = virtualDetector_.getStates();

  // Iterate through all physical keys
  for (const auto &physKey : physical.getKeys()) {
    // Find corresponding virtual key
    const VirtualKeyState *virtKey = virtual_states.findKeyById(physKey.id);
    if (!virtKey) {
      continue;
    }

    // Get tracker for this key
    MismatchTracker *tracker = mismatchTrackers_.getTracker(physKey.id);
    if (!tracker) {
      continue;
    }

    // Check mismatch: physical released but virtual pressed
    if (!physKey.pressed && virtKey->pressed) {
      tracker->start();
    } else {
      tracker->reset();
    }
  }
}

bool ModifierKeyFixer::shouldCheckForFix(const InterceptionKeyStroke &stroke) {
  // Only trigger on key down
  if (stroke.state & INTERCEPTION_KEY_UP) {
    return false;
  }

  // Check if any key is stuck
  if (!mismatchTrackers_.hasAnyStuck(thresholdMs_)) {
    return false;
  }

  // Check if there are other physical keys pressed
  if (hasOtherPhysicalKeyPressed(stroke)) {
    return false;
  }

  return true;
}

int ModifierKeyFixer::fixStuckKeys(InterceptionDevice device) {
  int fixedCount = 0;
  const auto &physical = physicalDetector_.getStates();

  // Iterate through all physical keys
  for (const auto &key : physical.getKeys()) {
    const MismatchTracker *tracker = mismatchTrackers_.getTracker(key.id);
    if (!tracker || !tracker->isStuck(thresholdMs_)) {
      continue;
    }

    // Send release event for this key
    sendKeyRelease(device, key.scanCode, key.needsE0);
    fixedCount++;

    // Update statistics
    stats_.incrementFix(key.id);

    if (showMessages_) {
      std::cout << "  [Fixed] " << key.name << std::endl;
    }
  }

  if (fixedCount > 0) {
    Sleep(20);
    virtualDetector_.update();
  }

  return fixedCount;
}

void ModifierKeyFixer::sendKeyRelease(InterceptionDevice device,
                                      unsigned short scanCode, bool needsE0) {
  InterceptionKeyStroke releaseStroke;
  releaseStroke.code = scanCode;
  releaseStroke.state = INTERCEPTION_KEY_UP;

  if (needsE0) {
    releaseStroke.state |= INTERCEPTION_KEY_E0;
  }

  releaseStroke.information = 0;

  interception_send(context_, device, (InterceptionStroke *)&releaseStroke, 1);
}

bool ModifierKeyFixer::hasOtherPhysicalKeyPressed(
    const InterceptionKeyStroke &stroke) {
  const auto &physical = physicalDetector_.getStates();

  // Check if any monitored key is pressed
  for (const auto &key : physical.getKeys()) {
    if (key.pressed) {
      return true;
    }
  }

  return false;
}
