#ifndef PHYSICAL_KEY_DETECTOR_H
#define PHYSICAL_KEY_DETECTOR_H

#include "interception.h"
#include <map>
#include <string>
#include <vector>

// Forward declaration
struct CustomKeyConfig;
struct KeyMappingConfig;

// Single key state
struct KeyState {
  std::string name;        // Key name (e.g., "Left Ctrl")
  std::string id;          // Unique ID (e.g., "lctrl")
  unsigned short scanCode; // Scan code
  bool needsE0;            // Whether E0 flag is required
  bool pressed;            // Current state

  KeyState(const std::string &name_, const std::string &id_,
           unsigned short scanCode_, bool needsE0_)
      : name(name_), id(id_), scanCode(scanCode_), needsE0(needsE0_),
        pressed(false) {}
};

// Modifier key states (now dynamic)
class ModifierKeyStates {
public:
  ModifierKeyStates();

  // Initialize with default modifier keys
  void initializeDefaultKeys();

  // Initialize with configuration (simple mode: quick toggles only)
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin);

  // Initialize with full configuration (advanced mode: with disabled keys and
  // custom keys)
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin,
                            const std::vector<std::string> &disabledKeys,
                            const std::vector<CustomKeyConfig> &customKeys);

  // Get all keys
  const std::vector<KeyState> &getKeys() const { return keys_; }
  std::vector<KeyState> &getKeys() { return keys_; }

  // Find key by ID
  KeyState *findKeyById(const std::string &id);
  const KeyState *findKeyById(const std::string &id) const;

  // Find key by scan code and E0 flag
  KeyState *findKeyByScanCode(unsigned short scanCode, bool needsE0);

  // Backward compatibility: access by field name
  bool lctrl() const;
  bool rctrl() const;
  bool lshift() const;
  bool rshift() const;
  bool lalt() const;
  bool ralt() const;
  bool lwin() const;
  bool rwin() const;

  // Helper methods to check combined states
  bool anyCtrl() const;
  bool anyShift() const;
  bool anyAlt() const;
  bool anyWin() const;

  // Check if states have changed
  bool operator!=(const ModifierKeyStates &other) const;

private:
  std::vector<KeyState> keys_;
};

// Physical key detector class
class PhysicalKeyDetector {
public:
  PhysicalKeyDetector();
  ~PhysicalKeyDetector();

  // Initialize the detector (resets all states)
  void initialize();

  // Initialize with configuration (simple mode: quick toggles only)
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin);

  // Initialize with full configuration (advanced mode: with disabled keys and
  // custom keys)
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin,
                            const std::vector<std::string> &disabledKeys,
                            const std::vector<CustomKeyConfig> &customKeys);

  // Initialize with full configuration including key mappings
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin,
                            const std::vector<std::string> &disabledKeys,
                            const std::vector<CustomKeyConfig> &customKeys,
                            const std::vector<KeyMappingConfig> &keyMappings);

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

  // Key mapping: source key (scanCode, needsE0) -> target key ID
  std::map<std::pair<unsigned short, bool>, std::string> keyMappings_;

  bool isModifierKey(const InterceptionKeyStroke &stroke) const;
  void updateModifierState(const InterceptionKeyStroke &stroke);
  void applyKeyMapping(unsigned short scanCode, bool needsE0, bool isPressed);
};

#endif // PHYSICAL_KEY_DETECTOR_H
