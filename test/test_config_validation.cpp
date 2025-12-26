// Feature: key-mapping-support
// Property 2: Invalid config rejected - Validates: Requirements 1.6, 3.3
// Property 7: Target key ID validation - Validates: Requirements 3.1, 3.3
// Property 8: Mapping type validation - Validates: Requirements 3.2, 3.4

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
  std::uniform_int_distribution<int> dis(0x01, 0xFF);
  return static_cast<unsigned short>(dis(gen));
}

// Helper: Generate random bool
bool randomBool() {
  std::uniform_int_distribution<int> dis(0, 1);
  return dis(gen) == 1;
}

// Helper: Generate random INVALID modifier key ID
std::string randomInvalidKeyId() {
  static std::vector<std::string> invalidKeys;
  if (invalidKeys.empty()) {
    invalidKeys.push_back("invalid");
    invalidKeys.push_back("capslock");
    invalidKeys.push_back("tab");
    invalidKeys.push_back("enter");
    invalidKeys.push_back("space");
    invalidKeys.push_back("LCTRL");
    invalidKeys.push_back("Rctrl");
    invalidKeys.push_back("leftctrl");
    invalidKeys.push_back("ctrl");
    invalidKeys.push_back("shift");
  }
  std::uniform_int_distribution<size_t> dis(0, invalidKeys.size() - 1);
  return invalidKeys[dis(gen)];
}

// Helper: Generate random VALID modifier key ID
std::string randomValidKeyId() {
  static std::vector<std::string> validKeys;
  if (validKeys.empty()) {
    validKeys.push_back("lctrl");
    validKeys.push_back("rctrl");
    validKeys.push_back("lshift");
    validKeys.push_back("rshift");
    validKeys.push_back("lalt");
    validKeys.push_back("ralt");
    validKeys.push_back("lwin");
    validKeys.push_back("rwin");
  }
  std::uniform_int_distribution<size_t> dis(0, validKeys.size() - 1);
  return validKeys[dis(gen)];
}

// Helper: Generate random INVALID mapping type
std::string randomInvalidMappingType() {
  static std::vector<std::string> invalidTypes;
  if (invalidTypes.empty()) {
    invalidTypes.push_back("invalid");
    invalidTypes.push_back("replace_all");
    invalidTypes.push_back("add");
    invalidTypes.push_back("override");
    invalidTypes.push_back("ADDITIONAL");
    invalidTypes.push_back("Replace");
  }
  std::uniform_int_distribution<size_t> dis(0, invalidTypes.size() - 1);
  return invalidTypes[dis(gen)];
}

// Helper: Create temp file
class TempFile {
public:
  TempFile(const std::string &name) : filename_(name) {}
  ~TempFile() { std::remove(filename_.c_str()); }
  const std::string &name() const { return filename_; }

private:
  std::string filename_;
};

// Property 2: Invalid configurations are rejected
bool testInvalidConfigRejection(int iteration) {
  std::string filename = "test_invalid_" + std::to_string(iteration) + ".toml";
  TempFile tempFile(filename);

  // Generate config with missing required field
  std::stringstream ss;
  ss << "[general]\n";
  ss << "thresholdMs = 1000\n\n";
  ss << "[[keyMappings]]\n";
  ss << "sourceScanCode = 0x" << std::hex << randomScanCode() << std::dec
     << "\n";
  // Missing sourceNeedsE0
  ss << "targetKeyId = \"" << randomValidKeyId() << "\"\n";
  ss << "mappingType = \"additional\"\n";

  std::ofstream file(filename);
  file << ss.str();
  file.close();

  Config config;
  bool loaded = config.load(filename);

  if (!loaded) {
    std::cerr << "Config should load even with invalid mappings (iteration "
              << iteration << ")" << std::endl;
    return false;
  }

  if (config.getKeyMappings().size() != 0) {
    std::cerr << "Invalid mapping should be ignored (iteration " << iteration
              << ")" << std::endl;
    return false;
  }

  return true;
}

// Property 7: Target key ID validation
bool testTargetKeyIdValidation(int iteration) {
  std::string filename = "test_target_" + std::to_string(iteration) + ".toml";
  TempFile tempFile(filename);

  // Generate config with invalid target key ID
  std::stringstream ss;
  ss << "[general]\n";
  ss << "thresholdMs = 1000\n\n";
  ss << "[[keyMappings]]\n";
  ss << "sourceScanCode = 0x" << std::hex << randomScanCode() << std::dec
     << "\n";
  ss << "sourceNeedsE0 = " << (randomBool() ? "true" : "false") << "\n";
  ss << "targetKeyId = \"" << randomInvalidKeyId() << "\"\n";
  ss << "mappingType = \"additional\"\n";

  std::ofstream file(filename);
  file << ss.str();
  file.close();

  Config config;
  bool loaded = config.load(filename);

  if (!loaded) {
    std::cerr << "Config should load (iteration " << iteration << ")"
              << std::endl;
    return false;
  }

  if (config.getKeyMappings().size() != 0) {
    std::cerr << "Invalid target key ID should be rejected (iteration "
              << iteration << ")" << std::endl;
    return false;
  }

  return true;
}

// Property 8: Mapping type validation and default value
bool testMappingTypeValidation(int iteration) {
  std::string filename = "test_type_" + std::to_string(iteration) + ".toml";
  TempFile tempFile(filename);

  // Generate config with invalid mapping type
  std::stringstream ss;
  ss << "[general]\n";
  ss << "thresholdMs = 1000\n\n";
  ss << "[[keyMappings]]\n";
  ss << "sourceScanCode = 0x" << std::hex << randomScanCode() << std::dec
     << "\n";
  ss << "sourceNeedsE0 = " << (randomBool() ? "true" : "false") << "\n";
  ss << "targetKeyId = \"" << randomValidKeyId() << "\"\n";
  ss << "mappingType = \"" << randomInvalidMappingType() << "\"\n";

  std::ofstream file(filename);
  file << ss.str();
  file.close();

  Config config;
  bool loaded = config.load(filename);

  if (!loaded) {
    std::cerr << "Config should load (iteration " << iteration << ")"
              << std::endl;
    return false;
  }

  if (config.getKeyMappings().size() != 1) {
    std::cerr << "Should have 1 mapping (iteration " << iteration << ")"
              << std::endl;
    return false;
  }

  if (config.getKeyMappings()[0].mappingType != "additional") {
    std::cerr << "Should use default 'additional' type (iteration " << iteration
              << ")" << std::endl;
    return false;
  }

  return true;
}

// Additional test: Missing mapping type should default to "additional"
bool testMissingMappingType(int iteration) {
  std::string filename =
      "test_missing_type_" + std::to_string(iteration) + ".toml";
  TempFile tempFile(filename);

  // Generate config without mapping type
  std::stringstream ss;
  ss << "[general]\n";
  ss << "thresholdMs = 1000\n\n";
  ss << "[[keyMappings]]\n";
  ss << "sourceScanCode = 0x" << std::hex << randomScanCode() << std::dec
     << "\n";
  ss << "sourceNeedsE0 = " << (randomBool() ? "true" : "false") << "\n";
  ss << "targetKeyId = \"" << randomValidKeyId() << "\"\n";

  std::ofstream file(filename);
  file << ss.str();
  file.close();

  Config config;
  bool loaded = config.load(filename);

  if (!loaded) {
    std::cerr << "Config should load (iteration " << iteration << ")"
              << std::endl;
    return false;
  }

  if (config.getKeyMappings().size() != 1) {
    std::cerr << "Should have 1 mapping (iteration " << iteration << ")"
              << std::endl;
    return false;
  }

  if (config.getKeyMappings()[0].mappingType != "additional") {
    std::cerr << "Should use default 'additional' type (iteration " << iteration
              << ")" << std::endl;
    return false;
  }

  return true;
}

int main() {
  std::cout << "=== Config Validation Property-Based Tests ===" << std::endl;
  std::cout << std::endl;

  const int NUM_ITERATIONS = 100;
  int totalPassed = 0;
  int totalFailed = 0;

  // Property 2
  std::cout << "Property 2: Invalid configurations are rejected" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  int passed = 0;
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testInvalidConfigRejection(i)) {
      passed++;
    } else {
      std::cerr << "FAILED at iteration " << i << std::endl;
      break;
    }
    if ((i + 1) % 25 == 0) {
      std::cout << "  Completed " << (i + 1) << "/" << NUM_ITERATIONS
                << std::endl;
    }
  }
  std::cout << "Result: " << passed << "/" << NUM_ITERATIONS << " passed"
            << std::endl;
  totalPassed += passed;
  if (passed < NUM_ITERATIONS)
    totalFailed++;
  std::cout << std::endl;

  // Property 7
  std::cout << "Property 7: Target key ID validation" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testTargetKeyIdValidation(i)) {
      passed++;
    } else {
      std::cerr << "FAILED at iteration " << i << std::endl;
      break;
    }
    if ((i + 1) % 25 == 0) {
      std::cout << "  Completed " << (i + 1) << "/" << NUM_ITERATIONS
                << std::endl;
    }
  }
  std::cout << "Result: " << passed << "/" << NUM_ITERATIONS << " passed"
            << std::endl;
  totalPassed += passed;
  if (passed < NUM_ITERATIONS)
    totalFailed++;
  std::cout << std::endl;

  // Property 8
  std::cout << "Property 8: Mapping type validation and default value"
            << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testMappingTypeValidation(i)) {
      passed++;
    } else {
      std::cerr << "FAILED at iteration " << i << std::endl;
      break;
    }
    if ((i + 1) % 25 == 0) {
      std::cout << "  Completed " << (i + 1) << "/" << NUM_ITERATIONS
                << std::endl;
    }
  }
  std::cout << "Result: " << passed << "/" << NUM_ITERATIONS << " passed"
            << std::endl;
  totalPassed += passed;
  if (passed < NUM_ITERATIONS)
    totalFailed++;
  std::cout << std::endl;

  // Additional test
  std::cout << "Additional: Missing mapping type defaults to 'additional'"
            << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testMissingMappingType(i)) {
      passed++;
    } else {
      std::cerr << "FAILED at iteration " << i << std::endl;
      break;
    }
    if ((i + 1) % 25 == 0) {
      std::cout << "  Completed " << (i + 1) << "/" << NUM_ITERATIONS
                << std::endl;
    }
  }
  std::cout << "Result: " << passed << "/" << NUM_ITERATIONS << " passed"
            << std::endl;
  totalPassed += passed;
  if (passed < NUM_ITERATIONS)
    totalFailed++;
  std::cout << std::endl;

  // Summary
  std::cout << "=== Summary ===" << std::endl;
  std::cout << "Total iterations: " << totalPassed << "/"
            << (NUM_ITERATIONS * 4) << std::endl;

  if (totalFailed == 0) {
    std::cout << "All property tests PASSED!" << std::endl;
    return 0;
  } else {
    std::cout << totalFailed << " property test(s) FAILED!" << std::endl;
    return 1;
  }
}
