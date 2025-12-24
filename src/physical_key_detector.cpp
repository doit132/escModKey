#include "physical_key_detector.h"

// Modifier key scan codes (based on actual testing)
#define SCANCODE_LCTRL 0x1D
#define SCANCODE_RCTRL 0x1D // with E0 flag
#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_LALT 0x38
#define SCANCODE_RALT 0x38 // with E0 flag
#define SCANCODE_LWIN 0x5B // with E0 flag
#define SCANCODE_RWIN 0x5C // with E0 flag

PhysicalKeyDetector::PhysicalKeyDetector() { initialize(); }

PhysicalKeyDetector::~PhysicalKeyDetector() {}

void PhysicalKeyDetector::initialize() { states_ = ModifierKeyStates(); }

void PhysicalKeyDetector::processKeyStroke(
    const InterceptionKeyStroke &stroke) {
  if (isModifierKey(stroke)) {
    updateModifierState(stroke);
  }
}

bool PhysicalKeyDetector::isModifierKey(
    const InterceptionKeyStroke &stroke) const {
  switch (stroke.code) {
  case SCANCODE_LCTRL:
  case SCANCODE_LSHIFT:
  case SCANCODE_RSHIFT:
  case SCANCODE_LALT:
  case SCANCODE_LWIN:
  case SCANCODE_RWIN:
    return true;
  default:
    return false;
  }
}

void PhysicalKeyDetector::updateModifierState(
    const InterceptionKeyStroke &stroke) {
  bool isPressed = !(stroke.state & INTERCEPTION_KEY_UP);
  bool isE0 = stroke.state & INTERCEPTION_KEY_E0;

  switch (stroke.code) {
  case SCANCODE_LCTRL:
    if (isE0) {
      states_.rctrl = isPressed;
    } else {
      states_.lctrl = isPressed;
    }
    break;
  case SCANCODE_LSHIFT:
    states_.lshift = isPressed;
    break;
  case SCANCODE_RSHIFT:
    states_.rshift = isPressed;
    break;
  case SCANCODE_LALT:
    if (isE0) {
      states_.ralt = isPressed;
    } else {
      states_.lalt = isPressed;
    }
    break;
  case SCANCODE_LWIN:
    if (isE0) {
      states_.lwin = isPressed;
    }
    break;
  case SCANCODE_RWIN:
    if (isE0) {
      states_.rwin = isPressed;
    }
    break;
  }
}
