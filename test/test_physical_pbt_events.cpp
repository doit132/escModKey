// Feature: key-mapping-support
// Property 3: Source key press synchronizes target
// Validates: Requirements 2.2, 5.2, 5.3
// Property 4: Source key release synchronizes target
// Validates: Requirements 2.3, 5.4
// Property 5: Target key updates independently
// Validates: Requirements 2.4
// Property 6: Multiple sources keep target pressed
// Validates: Requirements 2.5, 3.5
// Property 10: Mapping lookup correctness
// Validates: Requirements 5.1, 5.5

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

// Helper: Get scan code and E0 flag for a modifier key ID
std::pair<unsigned short, bool> getScanCodeForKeyId(const std::string &keyId) {
  if (keyId == "lctrl")
    return std::make_pair(static_cast<unsigned short>(0x1D), false);
  if (keyId == "rctrl")
    return std::make_pair(static_cast<unsigned short>(0x1D), true);
  if (keyId == "lshift")
    return std::make_pair(static_cast<unsigned short>(0x2A), false);
  if (keyId == "rshift")
    return std::make_pair(static_cast<unsigned short>(0x36), false);
  if (keyId == "lalt")
    return std::make_pair(static_cast<unsigned short>(0x38), false);
  if (keyId == "ralt")
    return std::make_pair(static_cast<unsigned short>(0x38), true);
  if (keyId == "lwin")
    return std::make_pair(static_cast<unsigned short>(0x5B), true);
  if (keyId == "rwin")
    return std::make_pair(static_cast<unsigned short>(0x5C), true);
  return std::make_pair(static_cast<unsigned short>(0), false);
}

// Helper: Check if key is pressed by ID
bool isKeyPressed(const PhysicalKeyDetector &detector,
                  const std::string &keyId) {
  const auto &states = detector.getStates();
  const KeyState *key = states.findKeyById(keyId);
  return key ? key->pressed : false;
}

// Helper: Create a key stroke
InterceptionKeyStroke createKeyStroke(unsigned short scanCode, bool needsE0,
                                      bool isPressed) {
  InterceptionKeyStroke stroke;
  stroke.code = scanCode;
  stroke.state = (needsE0 ? INTERCEPTION_KEY_E0 : 0) |
                 (isPressed ? 0 : INTERCEPTION_KEY_UP);
  stroke.information = 0;
  return stroke;
}

// Helper: Enable monitoring for a specific key type
void enableMonitoringForKey(const std::string &keyId, bool &monitorCtrl,
                            bool &monitorShift, bool &monitorAlt,
                            bool &monitorWin) {
  if (keyId == "lctrl" || keyId == "rctrl")
    monitorCtrl = true;
  else if (keyId == "lshift" || keyId == "rshift")
    monitorShift = true;
  else if (keyId == "lalt" || keyId == "ralt")
    monitorAlt = true;
  else if (keyId == "lwin" || keyId == "rwin")
    monitorWin = true;
}

// Property 3: Source key press synchronizes target key
bool testSourceKeyPressSyncsTarget(int iteration) {
  unsigned short sourceScanCode = randomScanCode();
  bool sourceNeedsE0 = randomBool();
  std::string targetKeyId = randomModifierKeyId();

  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(sourceScanCode, sourceNeedsE0, targetKeyId,
                        "additional");

  bool monitorCtrl = false, monitorShift = false, monitorAlt = false,
       monitorWin = false;
  enableMonitoringForKey(targetKeyId, monitorCtrl, monitorShift, monitorAlt,
                         monitorWin);

  PhysicalKeyDetector detector;
  detector.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                                monitorWin, {}, {}, mappings);

  if (isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Initial state error in iteration " << iteration << std::endl;
    return false;
  }

  InterceptionKeyStroke stroke =
      createKeyStroke(sourceScanCode, sourceNeedsE0, true);
  detector.processKeyStroke(stroke);

  if (!isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Target key not pressed after source press in iteration "
              << iteration << std::endl;
    return false;
  }

  return true;
}

// Property 4: Source key release synchronizes target key
bool testSourceKeyReleaseSyncsTarget(int iteration) {
  unsigned short sourceScanCode = randomScanCode();
  bool sourceNeedsE0 = randomBool();
  std::string targetKeyId = randomModifierKeyId();

  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(sourceScanCode, sourceNeedsE0, targetKeyId,
                        "additional");

  bool monitorCtrl = false, monitorShift = false, monitorAlt = false,
       monitorWin = false;
  enableMonitoringForKey(targetKeyId, monitorCtrl, monitorShift, monitorAlt,
                         monitorWin);

  PhysicalKeyDetector detector;
  detector.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                                monitorWin, {}, {}, mappings);

  InterceptionKeyStroke pressStroke =
      createKeyStroke(sourceScanCode, sourceNeedsE0, true);
  detector.processKeyStroke(pressStroke);

  InterceptionKeyStroke releaseStroke =
      createKeyStroke(sourceScanCode, sourceNeedsE0, false);
  detector.processKeyStroke(releaseStroke);

  if (isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Target key still pressed after source release in iteration "
              << iteration << std::endl;
    return false;
  }

  return true;
}

// Property 5: Target key itself updates independently
bool testTargetKeyIndependentUpdate(int iteration) {
  unsigned short sourceScanCode = randomScanCode();
  bool sourceNeedsE0 = randomBool();
  std::string targetKeyId = randomModifierKeyId();

  std::pair<unsigned short, bool> targetInfo = getScanCodeForKeyId(targetKeyId);
  unsigned short targetScanCode = targetInfo.first;
  bool targetNeedsE0 = targetInfo.second;

  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(sourceScanCode, sourceNeedsE0, targetKeyId,
                        "additional");

  bool monitorCtrl = false, monitorShift = false, monitorAlt = false,
       monitorWin = false;
  enableMonitoringForKey(targetKeyId, monitorCtrl, monitorShift, monitorAlt,
                         monitorWin);

  PhysicalKeyDetector detector;
  detector.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                                monitorWin, {}, {}, mappings);

  InterceptionKeyStroke pressStroke =
      createKeyStroke(targetScanCode, targetNeedsE0, true);
  detector.processKeyStroke(pressStroke);

  if (!isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Target key not pressed after direct press in iteration "
              << iteration << std::endl;
    return false;
  }

  InterceptionKeyStroke releaseStroke =
      createKeyStroke(targetScanCode, targetNeedsE0, false);
  detector.processKeyStroke(releaseStroke);

  if (isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Target key still pressed after direct release in iteration "
              << iteration << std::endl;
    return false;
  }

  return true;
}

// Property 6: Multiple source keys to same target
bool testMultipleSourcesKeepTargetPressed(int iteration) {
  std::string targetKeyId = randomModifierKeyId();

  unsigned short source1ScanCode = randomScanCode();
  bool source1NeedsE0 = randomBool();
  unsigned short source2ScanCode = randomScanCode();
  bool source2NeedsE0 = randomBool();

  if (source1ScanCode == source2ScanCode && source1NeedsE0 == source2NeedsE0) {
    source2ScanCode = (source1ScanCode + 1) % 0xFF;
  }

  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(source1ScanCode, source1NeedsE0, targetKeyId,
                        "additional");
  mappings.emplace_back(source2ScanCode, source2NeedsE0, targetKeyId,
                        "additional");

  bool monitorCtrl = false, monitorShift = false, monitorAlt = false,
       monitorWin = false;
  enableMonitoringForKey(targetKeyId, monitorCtrl, monitorShift, monitorAlt,
                         monitorWin);

  PhysicalKeyDetector detector;
  detector.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                                monitorWin, {}, {}, mappings);

  InterceptionKeyStroke press1 =
      createKeyStroke(source1ScanCode, source1NeedsE0, true);
  detector.processKeyStroke(press1);

  if (!isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Target not pressed after first source in iteration "
              << iteration << std::endl;
    return false;
  }

  InterceptionKeyStroke press2 =
      createKeyStroke(source2ScanCode, source2NeedsE0, true);
  detector.processKeyStroke(press2);

  if (!isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Target not pressed after second source in iteration "
              << iteration << std::endl;
    return false;
  }

  return true;
}

// Property 10: Mapping lookup correctness
bool testMappingLookupCorrectness(int iteration) {
  unsigned short mappedScanCode = randomScanCode();
  bool mappedNeedsE0 = randomBool();
  std::string targetKeyId = randomModifierKeyId();

  unsigned short unmappedScanCode = (mappedScanCode + 1) % 0xFF;
  bool unmappedNeedsE0 = !mappedNeedsE0;

  std::vector<KeyMappingConfig> mappings;
  mappings.emplace_back(mappedScanCode, mappedNeedsE0, targetKeyId,
                        "additional");

  bool monitorCtrl = false, monitorShift = false, monitorAlt = false,
       monitorWin = false;
  enableMonitoringForKey(targetKeyId, monitorCtrl, monitorShift, monitorAlt,
                         monitorWin);

  PhysicalKeyDetector detector;
  detector.initializeWithConfig(monitorCtrl, monitorShift, monitorAlt,
                                monitorWin, {}, {}, mappings);

  InterceptionKeyStroke mappedPress =
      createKeyStroke(mappedScanCode, mappedNeedsE0, true);
  detector.processKeyStroke(mappedPress);

  if (!isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Mapped key didn't affect target in iteration " << iteration
              << std::endl;
    return false;
  }

  InterceptionKeyStroke mappedRelease =
      createKeyStroke(mappedScanCode, mappedNeedsE0, false);
  detector.processKeyStroke(mappedRelease);

  InterceptionKeyStroke unmappedPress =
      createKeyStroke(unmappedScanCode, unmappedNeedsE0, true);
  detector.processKeyStroke(unmappedPress);

  if (isKeyPressed(detector, targetKeyId)) {
    std::cerr << "Unmapped key affected target in iteration " << iteration
              << std::endl;
    return false;
  }

  return true;
}

int main() {
  std::cout << "=== Physical Key Event Processing Property-Based Tests ==="
            << std::endl;
  std::cout << std::endl;

  const int NUM_ITERATIONS = 100;
  int totalPassed = 0;
  int totalFailed = 0;

  std::cout << "Property 3: Source key press synchronizes target" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  int passed = 0;
  int failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testSourceKeyPressSyncsTarget(i)) {
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

  std::cout << std::endl;
  std::cout << "Property 4: Source key release synchronizes target"
            << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testSourceKeyReleaseSyncsTarget(i)) {
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

  std::cout << std::endl;
  std::cout << "Property 5: Target key updates independently" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testTargetKeyIndependentUpdate(i)) {
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

  std::cout << std::endl;
  std::cout << "Property 6: Multiple sources keep target pressed" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testMultipleSourcesKeepTargetPressed(i)) {
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

  std::cout << std::endl;
  std::cout << "Property 10: Mapping lookup correctness" << std::endl;
  std::cout << "Running " << NUM_ITERATIONS << " iterations..." << std::endl;

  passed = 0;
  failed = 0;

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    if (testMappingLookupCorrectness(i)) {
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
