// Feature: key-mapping-support, Property 1: 配置解析往返一致性
// Validates: Requirements 1.1, 1.2, 1.3, 1.4, 1.5

#include "config.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
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
  static const std::vector<std::string> types = {"additional", "replace"};
  std::uniform_int_distribution<> dis(0, types.size() - 1);
  return types[dis(gen)];
}

// Helper: Generate random description
std::string randomDescription() {
  static const std::vector<std::string> descriptions = {
      "Test mapping 1", "Test mapping 2", "CapsLock to Ctrl", "Custom mapping",
      ""};
  std::uniform_int_distribution<> dis(0, descriptions.size() - 1);
  return descriptions[dis(gen)];
}

// Helper: Generate random valid key mapping
KeyMappingConfig generateRandomMapping() {
  return KeyMappingConfig(randomScanCode(), randomBool(), randomModifierKeyId(),
                          randomMappingType(), randomDescription());
}

// Helper: Generate random list of mappings
std::vector<KeyMappingConfig> generateRandomMappings(int count) {
  std::vector<KeyMappingConfig> mappings;
  for (int i = 0; i < count; ++i) {
    mappings.push_back(generateRandomMapping());
  }
  return mappings;
}

// Helper: Create temp file and cleanup
class TempFile {
public:
  TempFile(const std::string &name) : filename_(name) {}
  ~TempFile() { std::remove(filename_.c_str()); }
  const std::string &name() const { return filename_; }

private:
  std::string filename_;
};

// Helper: Compare two mappings for equality
bool mappingsEqual(const KeyMappingConfig &a, const KeyMappingConfig &b) {
  return a.sourceScanCode == b.sourceScanCode &&
         a.sourceNeedsE0 == b.sourceNeedsE0 && a.targetKeyId == b.targetKeyId &&
         a.mappingType == b.mappingType && a.description == b.description;
}

// Property 1: Configuration round-trip consistency
// For any valid mapping configuration list, serializing to TOML and parsing
// back should produce equivalent configuration
bool testRoundTripProperty(int iteration) {
  // Generate random number of mappings (0-5)
  std::uniform_int_distribution<> countDis(0, 5);
  int count = countDis(gen);

  std::vector<KeyMappingConfig> originalMappings =
      generateRandomMappings(count);

  // Create config and set mappings
  Config config;
  config.setKeyMappings(originalMappings);

  // Save to file
  std::string filename =
      "test_roundtrip_" + std::to_string(iteration) + ".toml";
  TempFile tempFile(filename);

  if (!config.save(filename)) {
    std::cerr << "Failed to save config in iteration " << iteration
              << std::endl;
    return false;
  }

  // Load from file
  Config loadedConfig;
  if (!loadedConfig.load(filename)) {
    std::cerr << "Failed to load config in iteration " << iteration
              << std::endl;
    return false;
  }

  // Compare mappings
  const auto &loadedMappings = loadedConfig.getKeyMappings();

  if (originalMappings.size() != loadedMappings.size()) {
    std::cerr << "Mapping count mismatch in iteration " << iteration
              << ": expected " << originalMappings.size() << ", got "
              << loadedMappings.size() << std::endl;
    return false;
  }

  for (size_t i = 0; i < originalMappings.size(); ++i) {
    if (!mappingsEqual(originalMappings[i], loadedMappings[i])) {
      std::cerr << "Mapping mismatch at index " << i << " in iteration "
                << iteration << std::endl;
      std::cerr << "  Original: scanCode=0x" << std::hex
                << originalMappings[i].sourceScanCode
                << ", E0=" << originalMappings[i].sourceNeedsE0
                << ", target=" << originalMappings[i].targetKeyId
                << ", type=" << originalMappings[i].mappingType << std::endl;
      std::cerr << "  Loaded: scanCode=0x" << std::hex
                << loadedMappings[i].sourceScanCode
                << ", E0=" << loadedMappings[i].sourceNeedsE0
                << ", target=" << loadedMappings[i].targetKeyId
                << ", type=" << loadedMappings[i].mappingType << std::endl;
      return false;
    }
  }

  return true;
}

int main() {
  std::cout << "=== Config Key Mapping Property-Based Tests ===" << std::endl;
  std::cout << std::endl;

  const int NUM_ITERATIONS = 100;

  std::cout << "Property 1: Configuration round-trip consistency" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  int passed = 0;
  int failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testRoundTripProperty(i)) {
      passed++;
    } else {
      failed++;
      std::cerr << "FAILED at iteration " << i << std::endl;
      break; // Stop on first failure to show the issue
    }

    // Progress indicator
    if ((i + 1) % 10 == 0) {
      std::cout << "  Completed " << (i + 1) << "/" << NUM_ITERATIONS
                << " iterations..." << std::endl;
    }
  }

  std::cout << std::endl;
  std::cout << "Results: " << passed << " passed, " << failed << " failed"
            << std::endl;

  if (failed == 0) {
    std::cout << "Property test PASSED!" << std::endl;
    return 0;
  } else {
    std::cout << "Property test FAILED!" << std::endl;
    return 1;
  }
}
