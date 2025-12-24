#include "virtual_key_detector.h"

VirtualKeyDetector::VirtualKeyDetector() { initialize(); }

VirtualKeyDetector::~VirtualKeyDetector() {}

void VirtualKeyDetector::initialize() { states_ = VirtualKeyStates(); }

void VirtualKeyDetector::update() {
  states_.lctrl = isVirtualKeyPressed(VK_LCONTROL);
  states_.rctrl = isVirtualKeyPressed(VK_RCONTROL);
  states_.lshift = isVirtualKeyPressed(VK_LSHIFT);
  states_.rshift = isVirtualKeyPressed(VK_RSHIFT);
  states_.lalt = isVirtualKeyPressed(VK_LMENU);
  states_.ralt = isVirtualKeyPressed(VK_RMENU);
  states_.lwin = isVirtualKeyPressed(VK_LWIN);
  states_.rwin = isVirtualKeyPressed(VK_RWIN);
}

bool VirtualKeyDetector::isVirtualKeyPressed(int vkCode) const {
  // GetAsyncKeyState returns the key state
  // High-order bit (0x8000) is set if key is currently down
  return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}
