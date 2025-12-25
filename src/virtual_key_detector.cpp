#include "virtual_key_detector.h"
#include "config.h"
#include "string_utils.h"
#include <algorithm>

// VirtualKeyStates implementation
VirtualKeyStates::VirtualKeyStates() { initializeDefaultKeys(); }

void VirtualKeyStates::initializeDefaultKeys() {
  keys_.clear();
  // Add default 8 modifier keys with their VK codes
  keys_.emplace_back("Left Ctrl", "lctrl", VK_LCONTROL);
  keys_.emplace_back("Right Ctrl", "rctrl", VK_RCONTROL);
  keys_.emplace_back("Left Shift", "lshift", VK_LSHIFT);
  keys_.emplace_back("Right Shift", "rshift", VK_RSHIFT);
  keys_.emplace_back("Left Alt", "lalt", VK_LMENU);
  keys_.emplace_back("Right Alt", "ralt", VK_RMENU);
  keys_.emplace_back("Left Win", "lwin", VK_LWIN);
  keys_.emplace_back("Right Win", "rwin", VK_RWIN);
}

void VirtualKeyStates::initializeWithConfig(bool monitorCtrl, bool monitorShift,
                                            bool monitorAlt, bool monitorWin) {
  keys_.clear();

  // Add keys based on configuration
  if (monitorCtrl) {
    keys_.emplace_back("Left Ctrl", "lctrl", VK_LCONTROL);
    keys_.emplace_back("Right Ctrl", "rctrl", VK_RCONTROL);
  }

  if (monitorShift) {
    keys_.emplace_back("Left Shift", "lshift", VK_LSHIFT);
    keys_.emplace_back("Right Shift", "rshift", VK_RSHIFT);
  }

  if (monitorAlt) {
    keys_.emplace_back("Left Alt", "lalt", VK_LMENU);
    keys_.emplace_back("Right Alt", "ralt", VK_RMENU);
  }

  if (monitorWin) {
    keys_.emplace_back("Left Win", "lwin", VK_LWIN);
    keys_.emplace_back("Right Win", "rwin", VK_RWIN);
  }
}

void VirtualKeyStates::initializeWithConfig(
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
                               [&lowerDisabledId](const VirtualKeyState &key) {
                                 return key.id == lowerDisabledId;
                               }),
                keys_.end());
  }

  // Add custom keys
  for (const auto &customKey : customKeys) {
    // Generate ID from name (lowercase, no spaces)
    std::string id = StringUtils::generateIdFromName(customKey.name);

    keys_.emplace_back(customKey.name, id, customKey.vkCode);
  }
}

VirtualKeyState *VirtualKeyStates::findKeyById(const std::string &id) {
  for (auto &key : keys_) {
    if (key.id == id) {
      return &key;
    }
  }
  return nullptr;
}

const VirtualKeyState *
VirtualKeyStates::findKeyById(const std::string &id) const {
  for (const auto &key : keys_) {
    if (key.id == id) {
      return &key;
    }
  }
  return nullptr;
}

// Backward compatibility methods
bool VirtualKeyStates::lctrl() const {
  const VirtualKeyState *key = findKeyById("lctrl");
  return key ? key->pressed : false;
}

bool VirtualKeyStates::rctrl() const {
  const VirtualKeyState *key = findKeyById("rctrl");
  return key ? key->pressed : false;
}

bool VirtualKeyStates::lshift() const {
  const VirtualKeyState *key = findKeyById("lshift");
  return key ? key->pressed : false;
}

bool VirtualKeyStates::rshift() const {
  const VirtualKeyState *key = findKeyById("rshift");
  return key ? key->pressed : false;
}

bool VirtualKeyStates::lalt() const {
  const VirtualKeyState *key = findKeyById("lalt");
  return key ? key->pressed : false;
}

bool VirtualKeyStates::ralt() const {
  const VirtualKeyState *key = findKeyById("ralt");
  return key ? key->pressed : false;
}

bool VirtualKeyStates::lwin() const {
  const VirtualKeyState *key = findKeyById("lwin");
  return key ? key->pressed : false;
}

bool VirtualKeyStates::rwin() const {
  const VirtualKeyState *key = findKeyById("rwin");
  return key ? key->pressed : false;
}

bool VirtualKeyStates::anyCtrl() const { return lctrl() || rctrl(); }

bool VirtualKeyStates::anyShift() const { return lshift() || rshift(); }

bool VirtualKeyStates::anyAlt() const { return lalt() || ralt(); }

bool VirtualKeyStates::anyWin() const { return lwin() || rwin(); }

bool VirtualKeyStates::operator!=(const VirtualKeyStates &other) const {
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

// VirtualKeyDetector implementation
VirtualKeyDetector::VirtualKeyDetector() { initialize(); }

VirtualKeyDetector::~VirtualKeyDetector() {}

void VirtualKeyDetector::initialize() { states_.initializeDefaultKeys(); }

void VirtualKeyDetector::initializeWithConfig(bool monitorCtrl,
                                              bool monitorShift,
                                              bool monitorAlt,
                                              bool monitorWin) {
  states_.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                               monitorWin);
}

void VirtualKeyDetector::initializeWithConfig(
    bool monitorCtrl, bool monitorShift, bool monitorAlt, bool monitorWin,
    const std::vector<std::string> &disabledKeys,
    const std::vector<CustomKeyConfig> &customKeys) {
  states_.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                               monitorWin, disabledKeys, customKeys);
}

void VirtualKeyDetector::update() {
  // Update all monitored keys
  for (auto &key : states_.getKeys()) {
    key.pressed = isVirtualKeyPressed(key.vkCode);
  }
}

bool VirtualKeyDetector::isVirtualKeyPressed(int vkCode) const {
  // GetAsyncKeyState returns the key state
  // High-order bit (0x8000) is set if key is currently down
  return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}
