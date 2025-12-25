#include "config.h"
#include "toml.h"
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <shlobj.h>

Config::Config() { loadDefaults(); }

void Config::loadDefaults() {
  // General settings
  thresholdMs_ = 1000;
  showMessages_ = true;

  // Notification settings
  notificationsEnabled_ = true;
  notifyOnFix_ = true;
  notifyOnStartup_ = true;

  // Advanced settings
  tooltipUpdateInterval_ = 1000;
  debugMode_ = false;

  // Key monitoring settings (default: monitor all)
  monitorCtrl_ = true;
  monitorShift_ = true;
  monitorAlt_ = true;
  monitorWin_ = true;
  disabledKeys_.clear();
  customKeys_.clear();
}

bool Config::load(const std::string &filepath) {
  try {
    // Parse TOML file
    auto config = toml::parse_file(filepath);

    // Load general settings
    if (auto general = config["general"].as_table()) {
      if (auto threshold = (*general)["thresholdMs"].value<int64_t>()) {
        thresholdMs_ = static_cast<int>(*threshold);
      }
      if (auto showMsg = (*general)["showMessages"].value<bool>()) {
        showMessages_ = *showMsg;
      }
    }

    // Load notification settings
    if (auto notifications = config["notifications"].as_table()) {
      if (auto enabled = (*notifications)["enabled"].value<bool>()) {
        notificationsEnabled_ = *enabled;
      }
      if (auto onFix = (*notifications)["notifyOnFix"].value<bool>()) {
        notifyOnFix_ = *onFix;
      }
      if (auto onStartup = (*notifications)["notifyOnStartup"].value<bool>()) {
        notifyOnStartup_ = *onStartup;
      }
    }

    // Load advanced settings
    if (auto advanced = config["advanced"].as_table()) {
      if (auto interval =
              (*advanced)["tooltipUpdateInterval"].value<int64_t>()) {
        tooltipUpdateInterval_ = static_cast<int>(*interval);
      }
      if (auto debug = (*advanced)["debugMode"].value<bool>()) {
        debugMode_ = *debug;
      }
    }

    // Load key monitoring settings
    if (auto keys = config["keys"].as_table()) {
      if (auto monitorCtrl = (*keys)["monitorCtrl"].value<bool>()) {
        monitorCtrl_ = *monitorCtrl;
      }
      if (auto monitorShift = (*keys)["monitorShift"].value<bool>()) {
        monitorShift_ = *monitorShift;
      }
      if (auto monitorAlt = (*keys)["monitorAlt"].value<bool>()) {
        monitorAlt_ = *monitorAlt;
      }
      if (auto monitorWin = (*keys)["monitorWin"].value<bool>()) {
        monitorWin_ = *monitorWin;
      }

      // Load disabled keys
      if (auto disabledKeys = (*keys)["disabledKeys"].as_array()) {
        disabledKeys_.clear();
        for (const auto &key : *disabledKeys) {
          if (auto keyStr = key.value<std::string>()) {
            disabledKeys_.push_back(*keyStr);
          }
        }
      }

      // Load custom keys
      if (auto customKeys = (*keys)["customKeys"].as_array()) {
        customKeys_.clear();
        for (const auto &keyArray : *customKeys) {
          if (auto arr = keyArray.as_array()) {
            if (arr->size() >= 4) {
              auto scanCode = (*arr)[0].value<int64_t>();
              auto needsE0 = (*arr)[1].value<bool>();
              auto name = (*arr)[2].value<std::string>();
              auto vkCode = (*arr)[3].value<int64_t>();

              if (scanCode && needsE0 && name && vkCode) {
                customKeys_.emplace_back(static_cast<unsigned short>(*scanCode),
                                         *needsE0, *name,
                                         static_cast<int>(*vkCode));
              }
            }
          }
        }
      }
    }

    return true;
  } catch (const toml::parse_error &err) {
    std::cerr << "Error parsing config file: " << err.description()
              << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error loading config: " << e.what() << std::endl;
    return false;
  }
}

bool Config::save(const std::string &filepath) const {
  try {
    std::ofstream file(filepath);
    if (!file.is_open()) {
      return false;
    }

    file << "# Modifier Key Auto-Fix Configuration\n";
    file << "# 修饰键自动修复工具配置文件\n\n";

    file << "[general]\n";
    file << "# Time threshold (ms) before considering a key as stuck\n";
    file << "# 判定按键卡住的时间阈值（毫秒）\n";
    file << "thresholdMs = " << thresholdMs_ << "\n\n";

    file << "# Show console messages (console version only)\n";
    file << "# 是否显示控制台消息（仅控制台版本）\n";
    file << "showMessages = " << (showMessages_ ? "true" : "false") << "\n\n";

    file << "[notifications]\n";
    file << "# Show balloon notifications (GUI version)\n";
    file << "# 是否显示气泡通知（GUI 版本）\n";
    file << "enabled = " << (notificationsEnabled_ ? "true" : "false")
         << "\n\n";

    file << "# Notify when a key is fixed\n";
    file << "# 修复按键时是否通知\n";
    file << "notifyOnFix = " << (notifyOnFix_ ? "true" : "false") << "\n\n";

    file << "# Notify on startup\n";
    file << "# 启动时是否通知\n";
    file << "notifyOnStartup = " << (notifyOnStartup_ ? "true" : "false")
         << "\n\n";

    file << "[advanced]\n";
    file << "# Update tray tooltip interval (ms)\n";
    file << "# 托盘提示更新间隔（毫秒）\n";
    file << "tooltipUpdateInterval = " << tooltipUpdateInterval_ << "\n\n";

    file << "# Enable debug logging\n";
    file << "# 启用调试日志\n";
    file << "debugMode = " << (debugMode_ ? "true" : "false") << "\n\n";

    file << "[keys]\n";
    file << "# Quick toggle for standard modifier keys\n";
    file << "# 标准修饰键快速开关\n";
    file << "monitorCtrl = " << (monitorCtrl_ ? "true" : "false") << "\n";
    file << "monitorShift = " << (monitorShift_ ? "true" : "false") << "\n";
    file << "monitorAlt = " << (monitorAlt_ ? "true" : "false") << "\n";
    file << "monitorWin = " << (monitorWin_ ? "true" : "false") << "\n\n";

    file << "# Advanced: Disable specific left/right keys\n";
    file << "# 高级：禁用特定的左右键\n";
    file << "disabledKeys = [";
    for (size_t i = 0; i < disabledKeys_.size(); ++i) {
      if (i > 0)
        file << ", ";
      file << "\"" << disabledKeys_[i] << "\"";
    }
    file << "]\n\n";

    file << "# Advanced: Add custom keys to monitor\n";
    file << "# 高级：添加自定义监控按键\n";
    file << "# Format: [[scanCode, needsE0, name, vkCode]]\n";
    file << "customKeys = [";
    for (size_t i = 0; i < customKeys_.size(); ++i) {
      if (i > 0)
        file << ", ";
      file << "[" << customKeys_[i].scanCode << ", "
           << (customKeys_[i].needsE0 ? "true" : "false") << ", \""
           << customKeys_[i].name << "\", " << customKeys_[i].vkCode << "]";
    }
    file << "]\n";

    file.close();
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error saving config: " << e.what() << std::endl;
    return false;
  }
}

std::string Config::getProgramDirectory() {
  char path[MAX_PATH];
  GetModuleFileNameA(nullptr, path, MAX_PATH);
  std::string fullPath(path);
  size_t pos = fullPath.find_last_of("\\/");
  return fullPath.substr(0, pos);
}

std::string Config::getUserConfigDirectory() {
  char path[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
    std::string appData(path);
    std::string configDir = appData + "\\ModifierKeyAutoFix";

    // Create directory if it doesn't exist
    CreateDirectoryA(configDir.c_str(), nullptr);

    return configDir;
  }
  return "";
}

std::string Config::getDefaultConfigPath() {
  // Try program directory first
  std::string programDir = getProgramDirectory();
  std::string configPath = programDir + "\\config.toml";

  // Check if file exists in program directory
  std::ifstream file(configPath);
  if (file.good()) {
    file.close();
    return configPath;
  }

  // Try user config directory
  std::string userDir = getUserConfigDirectory();
  if (!userDir.empty()) {
    return userDir + "\\config.toml";
  }

  // Fallback to program directory
  return configPath;
}
