#ifndef PHYSICAL_KEY_DETECTOR_H
#define PHYSICAL_KEY_DETECTOR_H

#include "interception.h"

// Modifier key states
struct ModifierKeyStates {
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
};

// Physical key detector class
class PhysicalKeyDetector {
public:
  PhysicalKeyDetector();
  ~PhysicalKeyDetector();

  // Initialize the detector (resets all states)
  void initialize();

  // Process a key stroke and update states
  void processKeyStroke(const InterceptionKeyStroke &stroke);

  // Get current modifier key states
  const ModifierKeyStates &getStates() const { return states_; }

  // Check combined modifier key states
  bool isCtrlPressed() const { return states_.anyCtrl(); }
  bool isShiftPressed() const { return states_.anyShift(); }
  bool isAltPressed() const { return states_.anyAlt(); }
  bool isWinPressed() const { return states_.anyWin(); }

private:
  ModifierKeyStates states_;

  bool isModifierKey(const InterceptionKeyStroke &stroke) const;
  void updateModifierState(const InterceptionKeyStroke &stroke);
};

#endif // PHYSICAL_KEY_DETECTOR_H
