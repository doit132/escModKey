#ifndef CONFIG_H
#define CONFIG_H

#include <string>

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

  // Helper methods
  static std::string getProgramDirectory();
  static std::string getUserConfigDirectory();
};

#endif // CONFIG_H
