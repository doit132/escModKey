#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

// Custom key configuration
struct CustomKeyConfig {
  unsigned short scanCode;
  bool needsE0;
  std::string name;
  int vkCode;

  CustomKeyConfig(unsigned short sc, bool e0, const std::string &n, int vk)
      : scanCode(sc), needsE0(e0), name(n), vkCode(vk) {}
};

// Configuration class for Modifier Key Auto-Fix
class Config {
public:
  Config();

  // Load configuration from file
  // Returns true if successful, false otherwise
  bool load(const std::string &filepath);

  // Save current configuration to file
  bool save(const std::string &filepath) const;

  // Load default configuration
  void loadDefaults();

  // General settings
  int getThresholdMs() const { return thresholdMs_; }
  void setThresholdMs(int ms) { thresholdMs_ = ms; }

  bool getShowMessages() const { return showMessages_; }
  void setShowMessages(bool show) { showMessages_ = show; }

  // Notification settings (GUI version)
  bool getNotificationsEnabled() const { return notificationsEnabled_; }
  void setNotificationsEnabled(bool enabled) {
    notificationsEnabled_ = enabled;
  }

  bool getNotifyOnFix() const { return notifyOnFix_; }
  void setNotifyOnFix(bool notify) { notifyOnFix_ = notify; }

  bool getNotifyOnStartup() const { return notifyOnStartup_; }
  void setNotifyOnStartup(bool notify) { notifyOnStartup_ = notify; }

  // Advanced settings
  int getTooltipUpdateInterval() const { return tooltipUpdateInterval_; }
  void setTooltipUpdateInterval(int ms) { tooltipUpdateInterval_ = ms; }

  bool getDebugMode() const { return debugMode_; }
  void setDebugMode(bool debug) { debugMode_ = debug; }

  // Key monitoring settings
  bool getMonitorCtrl() const { return monitorCtrl_; }
  void setMonitorCtrl(bool monitor) { monitorCtrl_ = monitor; }

  bool getMonitorShift() const { return monitorShift_; }
  void setMonitorShift(bool monitor) { monitorShift_ = monitor; }

  bool getMonitorAlt() const { return monitorAlt_; }
  void setMonitorAlt(bool monitor) { monitorAlt_ = monitor; }

  bool getMonitorWin() const { return monitorWin_; }
  void setMonitorWin(bool monitor) { monitorWin_ = monitor; }

  const std::vector<std::string> &getDisabledKeys() const {
    return disabledKeys_;
  }
  void setDisabledKeys(const std::vector<std::string> &keys) {
    disabledKeys_ = keys;
  }

  const std::vector<CustomKeyConfig> &getCustomKeys() const {
    return customKeys_;
  }
  void setCustomKeys(const std::vector<CustomKeyConfig> &keys) {
    customKeys_ = keys;
  }

  // Get configuration file path
  // Tries program directory first, then user directory
  static std::string getDefaultConfigPath();

private:
  // General settings
  int thresholdMs_;
  bool showMessages_;

  // Notification settings
  bool notificationsEnabled_;
  bool notifyOnFix_;
  bool notifyOnStartup_;

  // Advanced settings
  int tooltipUpdateInterval_;
  bool debugMode_;

  // Key monitoring settings
  bool monitorCtrl_;
  bool monitorShift_;
  bool monitorAlt_;
  bool monitorWin_;
  std::vector<std::string> disabledKeys_;
  std::vector<CustomKeyConfig> customKeys_;

  // Helper methods
  static std::string getProgramDirectory();
  static std::string getUserConfigDirectory();
};

#endif // CONFIG_H
