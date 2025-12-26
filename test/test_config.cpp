#include "config.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

// Helper function to create a temporary config file
void createTempConfigFile(const std::string &filename,
                          const std::string &content) {
  std::ofstream file(filename);
  file << content;
  file.close();
}

// Helper function to delete a temporary file
void deleteTempFile(const std::string &filename) {
  std::remove(filename.c_str());
}

// Test 1: Parse configuration with single mapping
void testParseSingleMapping() {
  std::cout << "Test 1: Parse single mapping... ";

  std::string configContent = R"(
[general]
thresholdMs = 1000

[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
description = "CapsLock to Left Ctrl"
)";

  createTempConfigFile("test_single.toml", configContent);

  Config config;
  bool loaded = config.load("test_single.toml");

  assert(loaded && "Failed to load config");
  assert(config.getKeyMappings().size() == 1 && "Should have 1 mapping");

  const auto &mapping = config.getKeyMappings()[0];
  assert(mapping.sourceScanCode == 0x3A && "Source scan code mismatch");
  assert(mapping.sourceNeedsE0 == false && "Source E0 flag mismatch");
  assert(mapping.targetKeyId == "lctrl" && "Target key ID mismatch");
  assert(mapping.mappingType == "additional" && "Mapping type mismatch");
  assert(mapping.description == "CapsLock to Left Ctrl" &&
         "Description mismatch");

  deleteTempFile("test_single.toml");
  std::cout << "PASSED" << std::endl;
}

// Test 2: Parse configuration with multiple mappings
void testParseMultipleMappings() {
  std::cout << "Test 2: Parse multiple mappings... ";

  std::string configContent = R"(
[general]
thresholdMs = 1000

[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"

[[keyMappings]]
sourceScanCode = 0x1D
sourceNeedsE0 = false
targetKeyId = "lwin"
mappingType = "additional"
)";

  createTempConfigFile("test_multiple.toml", configContent);

  Config config;
  bool loaded = config.load("test_multiple.toml");

  assert(loaded && "Failed to load config");
  assert(config.getKeyMappings().size() == 2 && "Should have 2 mappings");

  deleteTempFile("test_multiple.toml");
  std::cout << "PASSED" << std::endl;
}

// Test 3: Parse configuration with empty mapping list
void testParseEmptyMappings() {
  std::cout << "Test 3: Parse empty mappings... ";

  std::string configContent = R"(
[general]
thresholdMs = 1000
)";

  createTempConfigFile("test_empty.toml", configContent);

  Config config;
  bool loaded = config.load("test_empty.toml");

  assert(loaded && "Failed to load config");
  assert(config.getKeyMappings().size() == 0 && "Should have 0 mappings");

  deleteTempFile("test_empty.toml");
  std::cout << "PASSED" << std::endl;
}

// Test 4: Parse configuration with missing required fields
void testParseMissingFields() {
  std::cout << "Test 4: Parse with missing required fields... ";

  std::string configContent = R"(
[general]
thresholdMs = 1000

[[keyMappings]]
sourceScanCode = 0x3A
targetKeyId = "lctrl"
)";

  createTempConfigFile("test_missing.toml", configContent);

  Config config;
  bool loaded = config.load("test_missing.toml");

  assert(loaded && "Should load config even with invalid mappings");
  assert(config.getKeyMappings().size() == 0 &&
         "Invalid mapping should be ignored");

  deleteTempFile("test_missing.toml");
  std::cout << "PASSED" << std::endl;
}

// Test 5: Parse configuration with invalid field types
void testParseInvalidFieldTypes() {
  std::cout << "Test 5: Parse with invalid field types... ";

  std::string configContent = R"(
[general]
thresholdMs = 1000

[[keyMappings]]
sourceScanCode = "invalid"
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
)";

  createTempConfigFile("test_invalid_type.toml", configContent);

  Config config;
  bool loaded = config.load("test_invalid_type.toml");

  assert(loaded && "Should load config even with invalid mappings");
  assert(config.getKeyMappings().size() == 0 &&
         "Invalid mapping should be ignored");

  deleteTempFile("test_invalid_type.toml");
  std::cout << "PASSED" << std::endl;
}

// Test 6: Validate invalid target key ID
void testInvalidTargetKeyId() {
  std::cout << "Test 6: Validate invalid target key ID... ";

  std::string configContent = R"(
[general]
thresholdMs = 1000

[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "invalid_key"
mappingType = "additional"
)";

  createTempConfigFile("test_invalid_target.toml", configContent);

  Config config;
  bool loaded = config.load("test_invalid_target.toml");

  assert(loaded && "Should load config");
  assert(config.getKeyMappings().size() == 0 &&
         "Invalid target key should be rejected");

  deleteTempFile("test_invalid_target.toml");
  std::cout << "PASSED" << std::endl;
}

// Test 7: Validate invalid mapping type with default fallback
void testInvalidMappingType() {
  std::cout << "Test 7: Validate invalid mapping type... ";

  std::string configContent = R"(
[general]
thresholdMs = 1000

[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "invalid_type"
)";

  createTempConfigFile("test_invalid_mapping_type.toml", configContent);

  Config config;
  bool loaded = config.load("test_invalid_mapping_type.toml");

  assert(loaded && "Should load config");
  assert(config.getKeyMappings().size() == 1 && "Should have 1 mapping");
  assert(config.getKeyMappings()[0].mappingType == "additional" &&
         "Should use default 'additional' type");

  deleteTempFile("test_invalid_mapping_type.toml");
  std::cout << "PASSED" << std::endl;
}

// Test 8: Validate missing mapping type uses default
void testMissingMappingType() {
  std::cout << "Test 8: Validate missing mapping type... ";

  std::string configContent = R"(
[general]
thresholdMs = 1000

[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
)";

  createTempConfigFile("test_missing_type.toml", configContent);

  Config config;
  bool loaded = config.load("test_missing_type.toml");

  assert(loaded && "Should load config");
  assert(config.getKeyMappings().size() == 1 && "Should have 1 mapping");
  assert(config.getKeyMappings()[0].mappingType == "additional" &&
         "Should use default 'additional' type");

  deleteTempFile("test_missing_type.toml");
  std::cout << "PASSED" << std::endl;
}

int main() {
  std::cout << "=== Config Key Mapping Unit Tests ===" << std::endl;
  std::cout << std::endl;

  try {
    testParseSingleMapping();
    testParseMultipleMappings();
    testParseEmptyMappings();
    testParseMissingFields();
    testParseInvalidFieldTypes();
    testInvalidTargetKeyId();
    testInvalidMappingType();
    testMissingMappingType();

    std::cout << std::endl;
    std::cout << "All tests PASSED!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << std::endl;
    std::cerr << "Test FAILED with exception: " << e.what() << std::endl;
    return 1;
  }
}
