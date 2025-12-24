#ifndef VIRTUAL_KEY_DETECTOR_H
#define VIRTUAL_KEY_DETECTOR_H

#include <Windows.h>

// Virtual key states structure
struct VirtualKeyStates {
  bool lctrl = false;
  bool rctrl = false;
  bool lshift = false;
  bool rshift = false;
  bool lalt = false;
  bool ralt = false;
  bool lwin = false;
  bool rwin = false;

  // Helper methods to check combined states
  bool anyCtrl() const { return lctrl || rctrl; }
  bool anyShift() const { return lshift || rshift; }
  bool anyAlt() const { return lalt || ralt; }
  bool anyWin() const { return lwin || rwin; }

  // Check if states have changed
  bool operator!=(const VirtualKeyStates &other) const {
    return lctrl != other.lctrl || rctrl != other.rctrl ||
           lshift != other.lshift || rshift != other.rshift ||
           lalt != other.lalt || ralt != other.ralt || lwin != other.lwin ||
           rwin != other.rwin;
  }
};

// Virtual key detector class
class VirtualKeyDetector {
public:
  VirtualKeyDetector();
  ~VirtualKeyDetector();

  // Initialize the detector (resets all states)
  void initialize();

  // Update virtual key states (call this periodically)
  void update();

  // Get current virtual key states
  const VirtualKeyStates &getStates() const { return states_; }

  // Check combined modifier key states
  bool isCtrlPressed() const { return states_.anyCtrl(); }
  bool isShiftPressed() const { return states_.anyShift(); }
  bool isAltPressed() const { return states_.anyAlt(); }
  bool isWinPressed() const { return states_.anyWin(); }

  // Check individual key states
  bool isLeftCtrlPressed() const { return states_.lctrl; }
  bool isRightCtrlPressed() const { return states_.rctrl; }
  bool isLeftShiftPressed() const { return states_.lshift; }
  bool isRightShiftPressed() const { return states_.rshift; }
  bool isLeftAltPressed() const { return states_.lalt; }
  bool isRightAltPressed() const { return states_.ralt; }
  bool isLeftWinPressed() const { return states_.lwin; }
  bool isRightWinPressed() const { return states_.rwin; }

private:
  VirtualKeyStates states_;

  bool isVirtualKeyPressed(int vkCode) const;
};

#endif // VIRTUAL_KEY_DETECTOR_H
