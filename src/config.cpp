#include "config.h"
#include "toml.h"
#include <Windows.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <shlobj.h>

// Helper function to validate target key ID
static bool isValidModifierKeyId(const std::string &keyId) {
  static const std::set<std::string> validKeys = {
      "lctrl", "rctrl", "lshift", "rshift", "lalt", "ralt", "lwin", "rwin"};
  return validKeys.find(keyId) != validKeys.end();
}

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
  keyMappings_.clear();
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

    // Load key mappings
    if (auto mappingsArray = config["keyMappings"].as_array()) {
      keyMappings_.clear();
      for (const auto &mappingTable : *mappingsArray) {
        if (auto table = mappingTable.as_table()) {
          auto sourceScanCode = (*table)["sourceScanCode"].value<int64_t>();
          auto sourceNeedsE0 = (*table)["sourceNeedsE0"].value<bool>();
          auto targetKeyId = (*table)["targetKeyId"].value<std::string>();
          auto mappingType = (*table)["mappingType"].value<std::string>();
          auto description = (*table)["description"].value<std::string>();

          // Validate required fields
          if (!sourceScanCode || !sourceNeedsE0.has_value() || !targetKeyId) {
            std::cerr << "Warning: Key mapping missing required field. Mapping "
                         "ignored."
                      << std::endl;
            continue;
          }

          // Validate target key ID
          if (!isValidModifierKeyId(*targetKeyId)) {
            std::cerr << "Error: Invalid target key ID '" << *targetKeyId
                      << "'. Mapping rejected." << std::endl;
            continue;
          }

          // Validate and set mapping type (default to "additional")
          std::string type = "additional";
          if (mappingType) {
            if (*mappingType == "additional" || *mappingType == "replace") {
              type = *mappingType;
            } else {
              std::cerr << "Warning: Invalid mapping type '" << *mappingType
                        << "'. Using default 'additional'." << std::endl;
            }
          }

          std::string desc = description ? *description : "";

          keyMappings_.emplace_back(
              static_cast<unsigned short>(*sourceScanCode), *sourceNeedsE0,
              *targetKeyId, type, desc);
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
    file << "]\n\n";

    // Save key mappings
    file << "# Key Mappings: Map non-modifier keys to modifier keys\n";
    file << "# 按键映射：将非修饰键映射为修饰键\n";
    file << "# Example: Map CapsLock to Left Ctrl\n";
    file << "# 示例：将 CapsLock 映射为左 Ctrl\n";
    for (const auto &mapping : keyMappings_) {
      file << "[[keyMappings]]\n";
      file << "sourceScanCode = 0x" << std::hex << std::uppercase
           << static_cast<int>(mapping.sourceScanCode) << std::dec << "\n";
      file << "sourceNeedsE0 = " << (mapping.sourceNeedsE0 ? "true" : "false")
           << "\n";
      file << "targetKeyId = \"" << mapping.targetKeyId << "\"\n";
      file << "mappingType = \"" << mapping.mappingType << "\"\n";
      if (!mapping.description.empty()) {
        file << "description = \"" << mapping.description << "\"\n";
      }
      file << "\n";
    }

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
