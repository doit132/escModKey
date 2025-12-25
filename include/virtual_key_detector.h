#ifndef VIRTUAL_KEY_DETECTOR_H
#define VIRTUAL_KEY_DETECTOR_H

#include <Windows.h>
#include <string>
#include <vector>

// Forward declaration
struct CustomKeyConfig;

// Virtual key state (similar to physical but with VK code)
struct VirtualKeyState {
  std::string name; // Key name (e.g., "Left Ctrl")
  std::string id;   // Unique ID (e.g., "lctrl")
  int vkCode;       // Virtual key code
  bool pressed;     // Current state

  VirtualKeyState(const std::string &name_, const std::string &id_, int vkCode_)
      : name(name_), id(id_), vkCode(vkCode_), pressed(false) {}
};

// Virtual key states (now dynamic)
class VirtualKeyStates {
public:
  VirtualKeyStates();

  // Initialize with default modifier keys
  void initializeDefaultKeys();

  // Initialize with configuration (Step 5: simple mode)
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin);

  // Initialize with full configuration (Step 6: advanced mode)
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin,
                            const std::vector<std::string> &disabledKeys,
                            const std::vector<CustomKeyConfig> &customKeys);

  // Get all keys
  const std::vector<VirtualKeyState> &getKeys() const { return keys_; }
  std::vector<VirtualKeyState> &getKeys() { return keys_; }

  // Find key by ID
  VirtualKeyState *findKeyById(const std::string &id);
  const VirtualKeyState *findKeyById(const std::string &id) const;

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
  bool operator!=(const VirtualKeyStates &other) const;

private:
  std::vector<VirtualKeyState> keys_;
};

// Virtual key detector class
class VirtualKeyDetector {
public:
  VirtualKeyDetector();
  ~VirtualKeyDetector();

  // Initialize the detector (resets all states)
  void initialize();

  // Initialize with configuration (Step 5: simple mode)
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin);

  // Initialize with full configuration (Step 6: advanced mode)
  void initializeWithConfig(bool monitorCtrl, bool monitorShift,
                            bool monitorAlt, bool monitorWin,
                            const std::vector<std::string> &disabledKeys,
                            const std::vector<CustomKeyConfig> &customKeys);

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
  bool isLeftCtrlPressed() const { return states_.lctrl(); }
  bool isRightCtrlPressed() const { return states_.rctrl(); }
  bool isLeftShiftPressed() const { return states_.lshift(); }
  bool isRightShiftPressed() const { return states_.rshift(); }
  bool isLeftAltPressed() const { return states_.lalt(); }
  bool isRightAltPressed() const { return states_.ralt(); }
  bool isLeftWinPressed() const { return states_.lwin(); }
  bool isRightWinPressed() const { return states_.rwin(); }

private:
  VirtualKeyStates states_;

  bool isVirtualKeyPressed(int vkCode) const;
};

#endif // VIRTUAL_KEY_DETECTOR_H
