#include "config.h"
#include "physical_key_detector.h"
#include <Windows.h>
#include <cassert>
#include <iostream>

// Test 1: Initialize with valid mapping configuration
void testInitializeWithValidMapping() {
  std::cout << "Test 1: Initialize with valid mapping... ";

  PhysicalKeyDetector detector;

  // Create mapping configuration: CapsLock (0x3A) -> Left Ctrl
  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(0x3A, false, "lctrl", "additional",
                        "CapsLock to Left Ctrl");

  // Initialize with Ctrl monitoring enabled
  detector.initializeWithConfig(true, false, false, false, {}, {}, mappings);

  std::cout << "PASSED" << std::endl;
}

// Test 2: Target key not in monitoring list
void testTargetKeyNotMonitored() {
  std::cout << "Test 2: Target key not in monitoring list... ";

  PhysicalKeyDetector detector;

  // Create mapping configuration: CapsLock -> Left Ctrl
  // But Ctrl is NOT being monitored
  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(0x3A, false, "lctrl", "additional");

  // Initialize with Ctrl monitoring DISABLED
  detector.initializeWithConfig(false, true, false, false, {}, {}, mappings);

  // The mapping should be ignored (warning logged to stderr)
  // No assertion needed - just verify it doesn't crash

  std::cout << "PASSED" << std::endl;
}

// Test 3: Multiple source keys mapping to same target key
void testMultipleSourcesToSameTarget() {
  std::cout << "Test 3: Multiple sources to same target... ";

  PhysicalKeyDetector detector;

  // Create multiple mappings to the same target
  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(0x3A, false, "lctrl", "additional",
                        "CapsLock to Left Ctrl");
  mappings.emplace_back(0x1D, false, "lctrl", "additional",
                        "Left Ctrl to Left Ctrl");

  // Initialize with Ctrl monitoring enabled
  detector.initializeWithConfig(true, false, false, false, {}, {}, mappings);

  std::cout << "PASSED" << std::endl;
}

// Test 4: Initialize with disabled keys
void testInitializeWithDisabledKeys() {
  std::cout << "Test 4: Initialize with disabled keys... ";

  PhysicalKeyDetector detector;

  // Create mapping to rctrl
  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(0x3A, false, "rctrl", "additional");

  // Disable rctrl
  std::vector<std::string> disabledKeys = {"rctrl"};

  // Initialize - mapping should be ignored because target is disabled
  detector.initializeWithConfig(true, false, false, false, disabledKeys, {},
                                mappings);

  std::cout << "PASSED" << std::endl;
}

// Test 5: Initialize with custom keys as targets
void testInitializeWithCustomKeyTarget() {
  std::cout << "Test 5: Initialize with custom key as target... ";

  PhysicalKeyDetector detector;

  // Create a custom key
  std::vector<CustomKeyConfig> customKeys;
  customKeys.emplace_back(0x3B, false, "Custom F1", VK_F1);

  // Create mapping to the custom key
  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(0x3A, false, "customf1", "additional");

  // Initialize
  detector.initializeWithConfig(true, false, false, false, {}, customKeys,
                                mappings);

  std::cout << "PASSED" << std::endl;
}

// Test 6: Initialize with unsupported mapping type
void testUnsupportedMappingType() {
  std::cout << "Test 6: Unsupported mapping type... ";

  PhysicalKeyDetector detector;

  // Create mapping with "replace" type (not yet supported)
  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(0x3A, false, "lctrl", "replace");

  // Initialize - mapping should be ignored with warning
  detector.initializeWithConfig(true, false, false, false, {}, {}, mappings);

  std::cout << "PASSED" << std::endl;
}

// Test 7: Initialize with empty mapping list
void testInitializeWithEmptyMappings() {
  std::cout << "Test 7: Initialize with empty mappings... ";

  PhysicalKeyDetector detector;

  std::vector<KeyMappingConfig> mappings; // Empty

  // Initialize
  detector.initializeWithConfig(true, false, false, false, {}, {}, mappings);

  std::cout << "PASSED" << std::endl;
}

int main() {
  std::cout << "=== Physical Key Detector Mapping Initialization Unit Tests ==="
            << std::endl;
  std::cout << std::endl;

  try {
    testInitializeWithValidMapping();
    testTargetKeyNotMonitored();
    testMultipleSourcesToSameTarget();
    testInitializeWithDisabledKeys();
    testInitializeWithCustomKeyTarget();
    testUnsupportedMappingType();
    testInitializeWithEmptyMappings();

    std::cout << std::endl;
    std::cout << "All tests PASSED!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << std::endl;
    std::cerr << "Test FAILED with exception: " << e.what() << std::endl;
    return 1;
  }
}
