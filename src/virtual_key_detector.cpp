#include "virtual_key_detector.h"

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
