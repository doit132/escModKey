#include "physical_key_detector.h"
#include "config.h"
#include "string_utils.h"
#include <algorithm>

// Modifier key scan codes (based on actual testing)
#define SCANCODE_LCTRL 0x1D
#define SCANCODE_RCTRL 0x1D // with E0 flag
#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_LALT 0x38
#define SCANCODE_RALT 0x38 // with E0 flag
#define SCANCODE_LWIN 0x5B // with E0 flag
#define SCANCODE_RWIN 0x5C // with E0 flag

// ModifierKeyStates implementation
ModifierKeyStates::ModifierKeyStates() { initializeDefaultKeys(); }

void ModifierKeyStates::initializeDefaultKeys() {
  keys_.clear();
  // Add default 8 modifier keys
  keys_.emplace_back("Left Ctrl", "lctrl", SCANCODE_LCTRL, false);
  keys_.emplace_back("Right Ctrl", "rctrl", SCANCODE_RCTRL, true);
  keys_.emplace_back("Left Shift", "lshift", SCANCODE_LSHIFT, false);
  keys_.emplace_back("Right Shift", "rshift", SCANCODE_RSHIFT, false);
  keys_.emplace_back("Left Alt", "lalt", SCANCODE_LALT, false);
  keys_.emplace_back("Right Alt", "ralt", SCANCODE_RALT, true);
  keys_.emplace_back("Left Win", "lwin", SCANCODE_LWIN, true);
  keys_.emplace_back("Right Win", "rwin", SCANCODE_RWIN, true);
}

void ModifierKeyStates::initializeWithConfig(bool monitorCtrl,
                                             bool monitorShift, bool monitorAlt,
                                             bool monitorWin) {
  keys_.clear();

  // Add keys based on configuration
  if (monitorCtrl) {
    keys_.emplace_back("Left Ctrl", "lctrl", SCANCODE_LCTRL, false);
    keys_.emplace_back("Right Ctrl", "rctrl", SCANCODE_RCTRL, true);
  }

  if (monitorShift) {
    keys_.emplace_back("Left Shift", "lshift", SCANCODE_LSHIFT, false);
    keys_.emplace_back("Right Shift", "rshift", SCANCODE_RSHIFT, false);
  }

  if (monitorAlt) {
    keys_.emplace_back("Left Alt", "lalt", SCANCODE_LALT, false);
    keys_.emplace_back("Right Alt", "ralt", SCANCODE_RALT, true);
  }

  if (monitorWin) {
    keys_.emplace_back("Left Win", "lwin", SCANCODE_LWIN, true);
    keys_.emplace_back("Right Win", "rwin", SCANCODE_RWIN, true);
  }
}

void ModifierKeyStates::initializeWithConfig(
    bool monitorCtrl, bool monitorShift, bool monitorAlt, bool monitorWin,
    const std::vector<std::string> &disabledKeys,
    const std::vector<CustomKeyConfig> &customKeys) {
  // First, initialize with simple mode (adds standard keys)
  initializeWithConfig(monitorCtrl, monitorShift, monitorAlt, monitorWin);

  // Remove disabled keys
  for (const auto &disabledId : disabledKeys) {
    // Convert to lowercase for case-insensitive comparison
    std::string lowerDisabledId = StringUtils::toLower(disabledId);

    // Remove matching keys
    keys_.erase(std::remove_if(keys_.begin(), keys_.end(),
                               [&lowerDisabledId](const KeyState &key) {
                                 return key.id == lowerDisabledId;
                               }),
                keys_.end());
  }

  // Add custom keys
  for (const auto &customKey : customKeys) {
    // Generate ID from name (lowercase, no spaces)
    std::string id = StringUtils::generateIdFromName(customKey.name);

    keys_.emplace_back(customKey.name, id, customKey.scanCode,
                       customKey.needsE0);
  }
}

KeyState *ModifierKeyStates::findKeyById(const std::string &id) {
  for (auto &key : keys_) {
    if (key.id == id) {
      return &key;
    }
  }
  return nullptr;
}

const KeyState *ModifierKeyStates::findKeyById(const std::string &id) const {
  for (const auto &key : keys_) {
    if (key.id == id) {
      return &key;
    }
  }
  return nullptr;
}

KeyState *ModifierKeyStates::findKeyByScanCode(unsigned short scanCode,
                                               bool needsE0) {
  for (auto &key : keys_) {
    if (key.scanCode == scanCode && key.needsE0 == needsE0) {
      return &key;
    }
  }
  return nullptr;
}

// Backward compatibility methods
bool ModifierKeyStates::lctrl() const {
  const KeyState *key = findKeyById("lctrl");
  return key ? key->pressed : false;
}

bool ModifierKeyStates::rctrl() const {
  const KeyState *key = findKeyById("rctrl");
  return key ? key->pressed : false;
}

bool ModifierKeyStates::lshift() const {
  const KeyState *key = findKeyById("lshift");
  return key ? key->pressed : false;
}

bool ModifierKeyStates::rshift() const {
  const KeyState *key = findKeyById("rshift");
  return key ? key->pressed : false;
}

bool ModifierKeyStates::lalt() const {
  const KeyState *key = findKeyById("lalt");
  return key ? key->pressed : false;
}

bool ModifierKeyStates::ralt() const {
  const KeyState *key = findKeyById("ralt");
  return key ? key->pressed : false;
}

bool ModifierKeyStates::lwin() const {
  const KeyState *key = findKeyById("lwin");
  return key ? key->pressed : false;
}

bool ModifierKeyStates::rwin() const {
  const KeyState *key = findKeyById("rwin");
  return key ? key->pressed : false;
}

bool ModifierKeyStates::anyCtrl() const { return lctrl() || rctrl(); }

bool ModifierKeyStates::anyShift() const { return lshift() || rshift(); }

bool ModifierKeyStates::anyAlt() const { return lalt() || ralt(); }

bool ModifierKeyStates::anyWin() const { return lwin() || rwin(); }

bool ModifierKeyStates::operator!=(const ModifierKeyStates &other) const {
  if (keys_.size() != other.keys_.size()) {
    return true;
  }
  for (size_t i = 0; i < keys_.size(); ++i) {
    if (keys_[i].pressed != other.keys_[i].pressed) {
      return true;
    }
  }
  return false;
}

// PhysicalKeyDetector implementation
PhysicalKeyDetector::PhysicalKeyDetector() { initialize(); }

PhysicalKeyDetector::~PhysicalKeyDetector() {}

void PhysicalKeyDetector::initialize() { states_.initializeDefaultKeys(); }

void PhysicalKeyDetector::initializeWithConfig(bool monitorCtrl,
                                               bool monitorShift,
                                               bool monitorAlt,
                                               bool monitorWin) {
  states_.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                               monitorWin);
}

void PhysicalKeyDetector::initializeWithConfig(
    bool monitorCtrl, bool monitorShift, bool monitorAlt, bool monitorWin,
    const std::vector<std::string> &disabledKeys,
    const std::vector<CustomKeyConfig> &customKeys) {
  states_.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                               monitorWin, disabledKeys, customKeys);
}

void PhysicalKeyDetector::processKeyStroke(
    const InterceptionKeyStroke &stroke) {
  if (isModifierKey(stroke)) {
    updateModifierState(stroke);
  }
}

bool PhysicalKeyDetector::isModifierKey(
    const InterceptionKeyStroke &stroke) const {
  bool isE0 = stroke.state & INTERCEPTION_KEY_E0;
  // Check if any monitored key matches this scan code and E0 flag
  for (const auto &key : states_.getKeys()) {
    if (key.scanCode == stroke.code && key.needsE0 == isE0) {
      return true;
    }
  }
  return false;
}

void PhysicalKeyDetector::updateModifierState(
    const InterceptionKeyStroke &stroke) {
  bool isPressed = !(stroke.state & INTERCEPTION_KEY_UP);
  bool isE0 = stroke.state & INTERCEPTION_KEY_E0;

  // Find the key and update its state
  KeyState *key = states_.findKeyByScanCode(stroke.code, isE0);
  if (key) {
    key->pressed = isPressed;
  }
}
