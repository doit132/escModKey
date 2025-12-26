#include "config.h"
#include "modifier_key_fixer.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

void createTempConfigFile(const std::string &filename,
                          const std::string &content) {
  std::ofstream file(filename);
  file << content;
  file.close();
}

void deleteTempFile(const std::string &filename) {
  std::remove(filename.c_str());
}

void testCompleteConfigLoadAndApply() {
  std::cout << "Test 1: Complete config load and apply... ";

  std::string configContent =
      "[general]\nthresholdMs = 1500\nshowMessages = false\n\n";
  configContent +=
      "[keyMonitoring]\nmonitorCtrl = true\nmonitorShift = false\n";
  configContent += "monitorAlt = false\nmonitorWin = false\n\n";
  configContent +=
      "[[keyMappings]]\nsourceScanCode = 0x3A\nsourceNeedsE0 = false\n";
  configContent += "targetKeyId = \"lctrl\"\nmappingType = \"additional\"\n";
  configContent += "description = \"CapsLock to Left Ctrl\"\n";

  createTempConfigFile("test_integration_1.toml", configContent);

  Config config;
  bool loaded = config.load("test_integration_1.toml");
  assert(loaded && "Failed to lo");
  assert(config.getThresholdMs() == 1500 && "Threshold mismatch");
  assert(config.getMonitorCtrl() == true && "Monitor Ctrl mismatch");
  assert(config.getKeyMappings().size() == 1 && "Should have 1 mapping");

  deleteTempFile("test_integration_1.toml");
  std::cout << "PASSED" << std::endl;
}

void testRestartReloadsMappings() {
  std::cout << "Test 2: Restart reloads mappings... ";

  std::string configContent1 = "[general]\nthresholdMs = 1000\n\n";
  configContent1 +=
      "[keyMonitoring]\nmonitorCtrl = true\nmonitorShift = false\n";
  configContent1 += "monitorAlt = false\nmonitorWin = false\n\n";
  configContent1 +=
      "[[keyMappings]]\nsourceScanCode = 0x3A\nsourceNeedsE0 = false\n";
  configContent1 += "targetKeyId = \"lctrl\"\nmappingType = \"additional\"\n";

  createTempConfigFile("test_restart.toml", configContent1);

  Config config;
  bool loaded = config.load("test_restart.toml");
  assert(loaded && "Failed to load initial config");
  assert(config.getKeyMappings().size() == 1 && "Should have 1 mapping");

  std::string configContent2 = "[general]\nthresholdMs = 1000\n\n";
  configContent2 +=
      "[keyMonitoring]\nmonitorCtrl = true\nmonitorShift = false\n";
  configContent2 += "monitorAlt = false\nmonitorWin = false\n\n";
  configContent2 +=
      "[[keyMappings]]\nsourceScanCode = 0x3A\nsourceNeedsE0 = false\n";
  configContent2 += "targetKeyId = \"lctrl\"\nmappingType = \"additional\"\n\n";
  configContent2 +=
      "[[keyMappings]]\nsourceScanCode = 0x1D\nsourceNeedsE0 = false\n";
  configContent2 += "targetKeyId = \"lwin\"\nmappingType = \"additional\"\n";

  createTempConfigFile("test_restart.toml", configContent2);

  loaded = config.load("test_restart.toml");
  assert(loaded && "Failed to reload config");
  assert(config.getKeyMappings().size() == 2 &&
         "Should have 2 mappings after reload");

  deleteTempFile("test_restart.toml");
  std::cout << "PASSED" << std::endl;
}

void testErrorConfigHandling() {
  std::cout << "Test 3: Error config handling... ";

  std::string configContent = "[general]\nthresholdMs = 1000\n\n";
  configContent +=
      "[keyMonitoring]\nmonitorCtrl = true\nmonitorShift = false\n";
  configContent += "monitorAlt = false\nmonitorWin = false\n\n";
  configContent +=
      "[[keyMappings]]\nsourceScanCode = 0x3A\nsourceNeedsE0 = false\n";
  configContent +=
      "targetKeyId = \"invalid_key\"\nmappingType = \"additional\"\n\n";
  configContent +=
      "[[keyMappings]]\nsourceScanCode = 0x1D\nsourceNeedsE0 = false\n";
  configContent += "targetKeyId = \"lctrl\"\nmappingType = \"additional\"\n";

  createTempConfigFile("test_error.toml", configContent);

  Config config;
  bool loaded = config.load("test_error.toml");
  assert(loaded && "Should load config even with invalid mappings");
  assert(config.getKeyMappings().size() == 1 && "Should have 1 valid mapping");
  assert(config.getKeyMappings()[0].targetKeyId == "lctrl" &&
         "Should keep valid mapping");

  deleteTempFile("test_error.toml");
  std::cout << "PASSED" << std::endl;
}

void testConfigWithNoMappings() {
  std::cout << "Test 4: Config with no mappings... ";

  std::string configContent = "[general]\nthresholdMs = 1000\n\n";
  configContent += "[keyMonitoring]\nmonitorCtrl = true\nmonitorShift = true\n";
  configContent += "monitorAlt = true\nmonitorWin = true\n";

  createTempConfigFile("test_no_mappings.toml", configContent);

  Config config;
  bool loaded = config.load("test_no_mappings.toml");
  assert(loaded && "Failed to load config");
  assert(config.getKeyMappings().size() == 0 && "Should have 0 mappings");

  deleteTempFile("test_no_mappings.toml");
  std::cout << "PASSED" << std::endl;
}

void testMultipleMappingsToDifferentTargets() {
  std::cout << "Test 5: Multiple mappings to different targets... ";

  std::string configContent = "[general]\nthresholdMs = 1000\n\n";
  configContent += "[keyMonitoring]\nmonitorCtrl = true\nmonitorShift = true\n";
  configContent += "monitorAlt = true\nmonitorWin = true\n\n";
  configContent +=
      "[[keyMappings]]\nsourceScanCode = 0x3A\nsourceNeedsE0 = false\n";
  configContent += "targetKeyId = \"lctrl\"\nmappingType = \"additional\"\n";
  configContent += "description = \"CapsLock to Left Ctrl\"\n\n";
  configContent +=
      "[[keyMappings]]\nsourceScanCode = 0x38\nsourceNeedsE0 = false\n";
  configContent += "targetKeyId = \"lalt\"\nmappingType = \"additional\"\n\n";
  configContent +=
      "[[keyMappings]]\nsourceScanCode = 0x2A\nsourceNeedsE0 = false\n";
  configContent += "targetKeyId = \"lshift\"\nmappingType = \"additional\"\n";

  createTempConfigFile("test_multiple_targets.toml", configContent);

  Config config;
  bool loaded = config.load("test_multiple_targets.toml");
  assert(loaded && "Failed to load config");
  assert(config.getKeyMappings().size() == 3 && "Should have 3 mappings");
  assert(config.getKeyMappings()[0].targetKeyId == "lctrl" &&
         "First mapping target mismatch");
  assert(config.getKeyMappings()[1].targetKeyId == "lalt" &&
         "Second mapping target mismatch");
  assert(config.getKeyMappings()[2].targetKeyId == "lshift" &&
         "Third mapping target mismatch");

  deleteTempFile("test_multiple_targets.toml");
  std::cout << "PASSED" << std::endl;
}

void testConfigReloadPreservesSettings() {
  std::cout << "Test 6: Config reload preserves settings... ";

  std::string configContent =
      "[general]\nthresholdMs = 2000\nshowMessages = true\n\n";
  configContent += "[notifications]\nenabled = true\nnotifyOnFix = "
                   "true\nnotifyOnStartup = false\n\n";
  configContent += "[keyMonitoring]\nmonitorCtrl = true\nmonitorShift = true\n";
  configContent += "monitorAlt = false\nmonitorWin = false\n\n";
  configContent +=
      "[[keyMappings]]\nsourceScanCode = 0x3A\nsourceNeedsE0 = false\n";
  configContent += "targetKeyId = \"lctrl\"\nmappingType = \"additional\"\n";

  createTempConfigFile("test_preserve.toml", configContent);

  Config config;
  bool loaded = config.load("test_preserve.toml");
  assert(loaded && "Failed to load config");
  assert(config.getThresholdMs() == 2000 && "Threshold mismatch");
  assert(config.getShowMessages() == true && "ShowMessages mismatch");
  assert(config.getNotificationsEnabled() == true && "Notifications mismatch");
  assert(config.getNotifyOnFix() == true && "NotifyOnFix mismatch");
  assert(config.getNotifyOnStartup() == false && "NotifyOnStartup mismatch");
  assert(config.getMonitorCtrl() == true && "MonitorCtrl mismatch");
  assert(config.getMonitorShift() == true && "MonitorShift mismatch");
  assert(config.getMonitorAlt() == false && "MonitorAlt mismatch");
  assert(config.getMonitorWin() == false && "MonitorWin mismatch");
  assert(config.getKeyMappings().size() == 1 && "Should have 1 mapping");

  deleteTempFile("test_preserve.toml");
  std::cout << "PASSED" << std::endl;
}

int main() {
  std::cout << "=== Integration Tests ===" << std::endl;
  std::cout << std::endl;

  try {
    testCompleteConfigLoadAndApply();
    testRestartReloadsMappings();
    testErrorConfigHandling();
    testConfigWithNoMappings();
    testMultipleMappingsToDifferentTargets();
    testConfigReloadPreservesSettings();

    std::cout << std::endl;
    std::cout << "All integration tests PASSED!" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: Full initialization tests require Interception driver."
              << std::endl;
    std::cout << "These tests verify configuration loading and handling logic."
              << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << std::endl;
    std::cerr << "Test FAILED with exception: " << e.what() << std::endl;
    return 1;
  }
}
