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
bool ModifierMismatchTrackers::hasAnyStuck(int thresholdMs) const {
  return lctrl.isStuck(thresholdMs) || rctrl.isStuck(thresholdMs) ||
         lshift.isStuck(thresholdMs) || rshift.isStuck(thresholdMs) ||
         lalt.isStuck(thresholdMs) || ralt.isStuck(thresholdMs) ||
         lwin.isStuck(thresholdMs) || rwin.isStuck(thresholdMs);
}

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

  auto checkKey = [](MismatchTracker &tracker, bool physical,
                     bool virtual_state) {
    if (!physical && virtual_state) {
      tracker.start();
    } else {
      tracker.reset();
    }
  };

  checkKey(mismatchTrackers_.lctrl, physical.lctrl, virtual_states.lctrl);
  checkKey(mismatchTrackers_.rctrl, physical.rctrl, virtual_states.rctrl);
  checkKey(mismatchTrackers_.lshift, physical.lshift, virtual_states.lshift);
  checkKey(mismatchTrackers_.rshift, physical.rshift, virtual_states.rshift);
  checkKey(mismatchTrackers_.lalt, physical.lalt, virtual_states.lalt);
  checkKey(mismatchTrackers_.ralt, physical.ralt, virtual_states.ralt);
  checkKey(mismatchTrackers_.lwin, physical.lwin, virtual_states.lwin);
  checkKey(mismatchTrackers_.rwin, physical.rwin, virtual_states.rwin);
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

  if (mismatchTrackers_.lctrl.isStuck(thresholdMs_)) {
    sendKeyRelease(device, 0x1D, false);
    fixedCount++;
    stats_.lctrlFixes++;
    if (showMessages_)
      std::cout << "  [Fixed] Left Ctrl" << std::endl;
  }

  if (mismatchTrackers_.rctrl.isStuck(thresholdMs_)) {
    sendKeyRelease(device, 0x1D, true);
    fixedCount++;
    stats_.rctrlFixes++;
    if (showMessages_)
      std::cout << "  [Fixed] Right Ctrl" << std::endl;
  }

  if (mismatchTrackers_.lshift.isStuck(thresholdMs_)) {
    sendKeyRelease(device, 0x2A, false);
    fixedCount++;
    stats_.lshiftFixes++;
    if (showMessages_)
      std::cout << "  [Fixed] Left Shift" << std::endl;
  }

  if (mismatchTrackers_.rshift.isStuck(thresholdMs_)) {
    sendKeyRelease(device, 0x36, false);
    fixedCount++;
    stats_.rshiftFixes++;
    if (showMessages_)
      std::cout << "  [Fixed] Right Shift" << std::endl;
  }

  if (mismatchTrackers_.lalt.isStuck(thresholdMs_)) {
    sendKeyRelease(device, 0x38, false);
    fixedCount++;
    stats_.laltFixes++;
    if (showMessages_)
      std::cout << "  [Fixed] Left Alt" << std::endl;
  }

  if (mismatchTrackers_.ralt.isStuck(thresholdMs_)) {
    sendKeyRelease(device, 0x38, true);
    fixedCount++;
    stats_.raltFixes++;
    if (showMessages_)
      std::cout << "  [Fixed] Right Alt" << std::endl;
  }

  if (mismatchTrackers_.lwin.isStuck(thresholdMs_)) {
    sendKeyRelease(device, 0x5B, true);
    fixedCount++;
    stats_.lwinFixes++;
    if (showMessages_)
      std::cout << "  [Fixed] Left Win" << std::endl;
  }

  if (mismatchTrackers_.rwin.isStuck(thresholdMs_)) {
    sendKeyRelease(device, 0x5C, true);
    fixedCount++;
    stats_.rwinFixes++;
    if (showMessages_)
      std::cout << "  [Fixed] Right Win" << std::endl;
  }

  if (fixedCount > 0) {
    stats_.totalFixes += fixedCount;
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

  if (physical.lctrl || physical.rctrl || physical.lshift || physical.rshift ||
      physical.lalt || physical.ralt || physical.lwin || physical.rwin) {
    return true;
  }

  return false;
}
