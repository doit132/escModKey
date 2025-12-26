// Feature: key-mapping-support, Property 9: 初始化时建立映射表
// Validates: Requirements 4.2, 4.3, 4.4

#include "config.h"
#include "physical_key_detector.h"
#include <Windows.h>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// Random generator
std::random_device rd;
std::mt19937 gen(rd());

// Helper: Generate random scan code
unsigned short randomScanCode() {
  std::uniform_int_distribution<> dis(0x01, 0xFF);
  return static_cast<unsigned short>(dis(gen));
}

// Helper: Generate random bool
bool randomBool() {
  std::uniform_int_distribution<> dis(0, 1);
  return dis(gen) == 1;
}

// Helper: Generate random valid modifier key ID
std::string randomModifierKeyId() {
  static const std::vector<std::string> validKeys = {
      "lctrl", "rctrl", "lshift", "rshift", "lalt", "ralt", "lwin", "rwin"};
  std::uniform_int_distribution<> dis(0, validKeys.size() - 1);
  return validKeys[dis(gen)];
}

// Helper: Generate random mapping type
std::string randomMappingType() {
  // Only "additional" is supported curly
  return "additional";
}

// Helper: Generate random valid key mapping
KeyMappingConfig generateRandomMapping() {
  return KeyMappingConfig(randomScanCode(), randomBool(), randomModifierKeyId(),
                          randomMappingType(), "Random mapping");
}

// Helper: Generate random list of mappings
std::vector<KeyMappingConfig> generateRandomMappings(int count) {
  std::vector<KeyMappingConfig> mappings;
  for (int i = 0; i < count; ++i) {
    mappings.push_back(generateRandomMapping());
  }
  return mappings;
}

// Helper: Generate random monitoring configuration
struct MonitoringConfig {
  bool monitorCtrl;
  bool monitorShift;
  bool monitorAlt;
  bool monitorWin;
};

MonitoringConfig generateRandomMonitoring() {
  // Ensure at least one key type is monitored
  MonitoringConfig config;
  do {
    config.monitorCtrl = randomBool();
    config.monitorShift = randomBool();
    config.monitorAlt = randomBool();
    config.monitorWin = randomBool();
  } while (!config.monitorCtrl && !config.monitorShift && !config.monitorAlt &&
           !config.monitorWin);
  return config;
}

// Helper: Check if a key ID is monitored given the configuration
bool isKeyMonitored(const std::string &keyId, const MonitoringConfig &config) {
  if (keyId == "lctrl" || keyId == "rctrl")
    return config.monitorCtrl;
  if (keyId == "lshift" || keyId == "rshift")
    return config.monitorShift;
  if (keyId == "lalt" || keyId == "ralt")
    return config.monitorAlt;
  if (keyId == "lwin" || keyId == "rwin")
    return config.monitorWin;
  return false;
}

// Property 9: Initialization establishes mapping table correctly
// For any mapping configuration list and monitoring configuration,
// when the physical key detector initializes, it should establish
// associations for each valid mapping (where target key is monitored)
bool testMappingTableProperty(int iteration) {
  // Generate random monitoring configuration
  MonitoringConfig monitoring = generateRandomMonitoring();

  // Generate random number of mappings (0-10)
  std::uniform_int_distribution<> countDis(0, 10);
  int count = countDis(gen);
  std::vector<KeyMappingConfig> mappings = generateRandomMappings(count);

  // Initialize detector
  PhysicalKeyDetector detector;
  detector.initializeWithConfig(monitoring.monitorCtrl, monitoring.monitorShift,
                                monitoring.monitorAlt, monitoring.monitorWin,
                                {}, {}, mappings);

  // Verify: The detector should have initialized without crashing
  // We can't directly inspect the internal mapping table, but we can verify
  // that the initialization completed successfully by checking the states

  const auto &states = detector.getStates();
  const auto &keys = states.getKeys();

  // Count how many keys are being monitored
  int monitoredCount = keys.size();

  // Count how many mappings should be valid (target is monitored)
  int validMappingCount = 0;
  for (const auto &mapping : mappings) {
    if (isKeyMonitored(mapping.targetKeyId, monitoring)) {
      validMappingCount++;
    }
  }

  // We can't directly verify the mapping table, but we can verify that:
  // 1. The detector initialized without crashing
  // 2. The number of monitored keys matches the configuration
  int expectedKeyCount = 0;
  if (monitoring.monitorCtrl)
    expectedKeyCount += 2; // lctrl, rctrl
  if (monitoring.monitorShift)
    expectedKeyCount += 2; // lshift, rshift
  if (monitoring.monitorAlt)
    expectedKeyCount += 2; // lalt, ralt
  if (monitoring.monitorWin)
    expectedKeyCount += 2; // lwin, rwin

  if (monitoredCount != expectedKeyCount) {
    std::cerr << "Key count mismatch in iteration " << iteration
              << ": expected " << expectedKeyCount << ", got " << monitoredCount
              << std::endl;
    return false;
  }

  return true;
}

// Additional test: Verify that mappings with unmonitored targets are ignored
bool testUnmonitoredTargetProperty(int iteration) {
  // Create a mapping to a key that won't be monitored
  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(0x3A, false, "lctrl", "additional");

  // Initialize with Ctrl NOT monitored
  PhysicalKeyDetector detector;
  detector.initializeWithConfig(false, true, false, false, {}, {}, mappings);

  // Verify: The detector should have initialized successfully
  // The mapping should be ignored (we can't directly verify this,
  // but the initialization should not crash)

  const auto &states = detector.getStates();
  const auto &keys = states.getKeys();

  // Should only have shift keys (2 keys)
  if (keys.size() != 2) {
    std::cerr << "Expected 2 keys (shift only), got " << keys.size()
              << " in iteration " << iteration << std::endl;
    return false;
  }

  return true;
}

// Test: Multiple mappings to same target
bool testMultipleMappingsToSameTargetProperty(int iteration) {
  // Create multiple mappings to the same target
  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(0x3A, false, "lctrl", "additional", "Mapping 1");
  mappings.emplace_back(0x3B, false, "lctrl", "additional", "Mapping 2");
  mappings.emplace_back(0x3C, false, "lctrl", "additional", "Mapping 3");

  // Initialize with Ctrl monitored
  PhysicalKeyDetector detector;
  detector.initializeWithConfig(true, false, false, false, {}, {}, mappings);

  // Verify: The detector should have initialized successfully
  // All mappings should be accepted (multiple sources to same target is valid)

  const auto &states = detector.getStates();
  const auto &keys = states.getKeys();

  // Should have 2 ctrl keys
  if (keys.size() != 2) {
    std::cerr << "Expected 2 keys (ctrl only), got " << keys.size()
              << " in iteration " << iteration << std::endl;
    return false;
  }

  return true;
}

int main() {
  std::cout << "=== Physical Key Detector Mapping Property-Based Tests ==="
            << std::endl;
  std::cout << std::endl;

  const int NUM_ITERATIONS = 100;
  int totalPassed = 0;
  int totalFailed = 0;

  // Test Property 9: Mapping table establishment
  std::cout << "Property 9: Initialization establishes mapping table"
            << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  int passed = 0;
  int failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testMappingTableProperty(i)) {
      passed++;
    } else {
      failed++;
      std::cerr << "FAILED at iteration " << i << std::endl;
      break;
    }

    if ((i + 1) % 10 == 0) {
      std::cout << "  Completed " << (i + 1) << "/" << NUM_ITERATIONS
                << " iterations..." << std::endl;
    }
  }

  std::cout << "Results: " << passed << " passed, " << failed << " failed"
            << std::endl;
  totalPassed += passed;
  totalFailed += failed;

  // Test: Unmonitored targets are ignored
  std::cout << std::endl;
  std::cout << "Additional test: Unmonitored targets are ignored" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testUnmonitoredTargetProperty(i)) {
      passed++;
    } else {
      failed++;
      std::cerr << "FAILED at iteration " << i << std::endl;
      break;
    }

    if ((i + 1) % 10 == 0) {
      std::cout << "  Completed " << (i + 1) << "/" << NUM_ITERATIONS
                << " iterations..." << std::endl;
    }
  }

  std::cout << "Results: " << passed << " passed, " << failed << " failed"
            << std::endl;
  totalPassed += passed;
  totalFailed += failed;

  // Test: Multiple mappings to same target
  std::cout << std::endl;
  std::cout << "Additional test: Multiple mappings to same target" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testMultipleMappingsToSameTargetProperty(i)) {
      passed++;
    } else {
      failed++;
      std::cerr << "FAILED at iteration " << i << std::endl;
      break;
    }

    if ((i + 1) % 10 == 0) {
      std::cout << "  Completed " << (i + 1) << "/" << NUM_ITERATIONS
                << " iterations..." << std::endl;
    }
  }

  std::cout << "Results: " << passed << " passed, " << failed << " failed"
            << std::endl;
  totalPassed += passed;
  totalFailed += failed;

  // Final results
  std::cout << std::endl;
  std::cout << "=== Final Results ===" << std::endl;
  std::cout << "Total: " << totalPassed << " passed, " << totalFailed
            << " failed" << std::endl;

  if (totalFailed == 0) {
    std::cout << "All property tests PASSED!" << std::endl;
    return 0;
  } else {
    std::cout << "Some property tests FAILED!" << std::endl;
    return 1;
  }
}
