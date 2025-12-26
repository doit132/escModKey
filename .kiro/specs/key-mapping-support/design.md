# Design Document: Key Mapping Support

## Overview

本设计文档描述了按键映射支持功能的实现方案。该功能允许用户在配置文件中定义非修饰键到修饰键的映射关系，使得物理按键检测器能够正确识别通过按键映射软件（如 AutoHotkey）映射的按键状态，避免误判为"卡住"并触发不必要的修复。

核心设计思路是：当用户配置了按键映射（如 CapsLock → Ctrl）后，物理按键检测器在检测到源按键（CapsLock）的物理状态变化时，同步更新目标按键（Ctrl）的物理状态，使得虚拟按键检测器看到的虚拟 Ctrl 按下与物理按键检测器看到的物理 Ctrl 按下保持一致，从而避免触发修复逻辑。

## Architecture

### 系统组件

1. **Config System (配置系统)**
   - 负责读取和解析 TOML 配置文件
   - 新增 `KeyMappingConfig` 结构体存储映射配置
   - 验证映射配置的有效性

2. **Physical Key Detector (物理按键检测器)**
   - 接收并存储映射配置
   - 在处理按键事件时应用映射逻辑
   - 维护源按键到目标按键的映射关系

3. **Modifier Key Fixer (修饰键修复器)**
   - 无需修改，继续使用现有逻辑
   - 通过物理和虚拟状态比较判断是否需要修复

### 数据流

```
配置文件 (config.toml)
    ↓
Config System 读取 [[keyMappings]]
    ↓
KeyMappingConfig 列表
    ↓
Physical Key Detector 初始化时接收映射配置
    ↓
建立源按键 → 目标按键的映射表
    ↓
处理按键事件时：
  - 检测到源按键事件
  - 查找映射表
新目标按键的物理状态
    ↓
Modifier Key Fixer 比较物理和虚拟状态
    ↓
状态一致，不触发修复
```

## Components and Interfaces

### 1. KeyMappingConfig 结构体

```cpp
// Key mapping configuration
struct KeyMappingConfig {
  unsigned short sourceScanCode;  // 源按键扫描码
  bool sourceNeedsE0;             // 源按键是否需要 E0 标志
  std::string targetKeyId;        // 目标按键 ID (如 "lctrl", "rctrl")
  std::string mappingType;        // 映射类型: "additional" 或 "replace"
  std::string description;        // 可选描述

  KeyMappingConfig(unsigned short sourceSc, bool sourceE0,
                   const std::string& targetId,
                   const std::string& type = "additional",
                   const std::string& desc = "")
      : sourceScanCode(sourceSc), sourceNeedsE0(sourceE0),
        targetKeyId(targetId), mappingType(type), description(desc) {}
};
```

### 2. Config 类扩展

在 `Config` 类中添加：

```cpp
class Config {
public:
  // ... 现有方法 ...

  // Key mapping settings
  const std::vector<KeyMappingConfig>& getKeyMappings() const {
    return keyMappings_;
  }
  void setKeyMappings(const std::vector<KeyMappingConfig>& mappings) {
    keyMappings_ = mappings;
  }

private:
  // ... 现有成员 ...
  std::vector<KeyMappingConfig> keyMappings_;
};
```

### 3. Config::load() 方法扩展

在 `Config::load()` 方法中添加读取 `[[keyMappings]]` 的逻辑：

```cpp
// Load key mappings
if (auto mappingsArray = config["keyMappings"].as_array()) {
  keyMappings_.clear();
  for (const auto& mappingTable : *mappingsArray) {
    if (auto table = mappingTable.as_table()) {
      auto sourceScanCode = (*table)["sourceScanCode"].value<int64_t>();
      auto sourceNeedsE0 = (*table)["sourceNeedsE0"].value<bool>();
      auto targetKeyId = (*table)["targetKeyId"].value<std::string>();
      auto mappingType = (*table)["mappingType"].value<std::string>();
      auto description = (*table)["description"].value<std::string>();

      if (sourceScanCode && sourceNeedsE0 && targetKeyId) {
        std::string type = mappingType ? *mappingType : "additional";
        std::string desc = description ? *description : "";
        
        keyMappings_.emplace_back(
          static_cast<unsigned short>(*sourceScanCode),
          *sourceNeedsE0,
          *targetKeyId,
          type,
          desc
        );
      }
    }
  }
}
```

### 4. PhysicalKeyDetector 类扩展

在 `PhysicalKeyDetector` 类中添加：

```cpp
class PhysicalKeyDetector {
public:
  // ... 现有方法 ...

  // Initialize with full configuration including key mappings
  void initializeWithConfig(
      bool monitorCtrl, bool monitorShift, bool monitorAlt, bool monitorWin,
      const std::vector<std::string>& disabledKeys,
      const std::vector<CustomKeyConfig>& customKeys,
      const std::vector<KeyMappingConfig>& keyMappings);

private:
  // ... 现有成员 ...
  
  // Key mapping: source key (scanCode, needsE0) -> target key ID
  std::map<std::pair<unsigned short, bool>, std::string> keyMappings_;
  
  // Helper method to apply key mapping
  void applyKeyMapping(unsigned short scanCode, bool needsE0, bool isPressed);
};
```

### 5. PhysicalKeyDetector::initializeWithConfig() 扩展

```cpp
void PhysicalKeyDetector::initializeWithConfig(
    bool monitorCtrl, bool monitorShift, bool monitorAlt, bool monitorWin,
    const std::vector<std::string>& disabledKeys,
    const std::vector<CustomKeyConfig>& customKeys,
    const std::vector<KeyMappingConfig>& keyMappings) {
  
  // 调用现有的初始化方法
  initializeWithConfig(monitorCtrl, monitorShift, monitorAlt, monitorWin,
                       disabledKeys, customKeys);
  
  // 清空并重建映射表
  keyMappings_.clear();
  
  // 处理每个映射配置
  for (const auto& mapping : keyMappings) {
    // 验证目标按键是否在监控列表中
    KeyState* targetKey = states_.findKeyById(mapping.targetKeyId);
    if (!targetKey) {
      // 目标按键不在监控列表中，记录警告并跳过
      std::cerr << "Warning: Key mapping target '" << mapping.targetKeyId
                << "' is not being monitored. Mapping ignored." << std::endl;
      continue;
    }
    
    // 仅支持 additional 类型（replace 类型暂不实现）
    if (mapping.mappingType != "additional") {
      std::cerr << "Warning: Only 'additional' mapping type is supported. "
                << "Mapping type '" << mapping.mappingType << "' ignored." << std::endl;
      continue;
    }
    
    // 添加到映射表
    auto key = std::make_pair(mapping.sourceScanCode, mapping.sourceNeedsE0);
    keyMappings_[key] = mapping.targetKeyId;
  }
}
```

### 6. PhysicalKeyDetector::processKeyStroke() 修改

```cpp
void PhysicalKeyDetector::processKeyStroke(
    const InterceptionKeyStroke& stroke) {
  
  // 首先检查是否是被监控的修饰键
  if (isModifierKey(stroke)) {
    updateModifierState(stroke);
  }
  
  // 然后检查是否是映射的源按键
  bool isE0 = stroke.state & INTERCEPTION_KEY_E0;
  auto key = std::make_pair(stroke.code, isE0);
  auto it = keyMappings_.find(key);
  
  if (it != keyMappings_.end()) {
    // 找到映射，同步更新目标按键状态
    bool isPressed = !(stroke.state & INTERCEPTION_KEY_UP);
    applyKeyMapping(stroke.code, isE0, isPressed);
  }
}
```

### 7. PhysicalKeyDetector::applyKeyMapping() 实现

```cpp
void PhysicalKeyDetector::applyKeyMapping(unsigned short scanCode,
                                          bool needsE0, bool isPressed) {
  auto key = std::make_pair(scanCode, needsE0);
  auto it = keyMappings_.find(key);
  
  if (it == keyMappings_.end()) {
    return;  // 没有映射
  }
  
  // 获取目标按键
  KeyState* targetKey = states_.findKeyById(it->second);
  if (!targetKey) {
    return;  // 目标按键不存在（不应该发生）
  }
  
  // 对于 additional 类型：
  // - 源按键按下 → 目标按键标记为按下
  // - 源按键释放 → 仅当目标按键本身未被物理按下时才标记为释放
  if (isPressed) {
    targetKey->pressed = true;
  } else {
    // 释放时需要检查目标按键本身是否被按下
    // 这里简化处理：直接标记为释放
    // 如果目标按键本身被按下，updateModifierState 会重新设置为按下
    targetKey->pressed = false;
  }
}
```

## Data Models

### KeyMappingConfig

```cpp
struct KeyMappingConfig {
  unsigned short sourceScanCode;  // 源按键扫描码 (0x00-0xFF)
  bool sourceNeedsE0;             // 源按键是否需要 E0 标志
  std::string targetKeyId;        // 目标按键 ID (小写，如 "lctrl")
  std::string mappingType;        // 映射类型: "additional" 或 "replace"
  std::string description;        // 可选描述
};
```

**字段说明：**

- `sourceScanCode`: 源按键的扫描码，范围 0x00-0xFF
- `sourceNeedsE0`: 源按键是否需要 E0 扩展标志（如 Right Ctrl 需要 E0）
- `targetKeyId`: 目标修饰键的 ID，必须是有效的修饰键 ID（lctrl, rctrl, lshift, rshift, lalt, ralt, lwin, rwin）
- `mappingType`: 映射类型，当前仅支持 "additional"
- `description`: 可选的映射描述，用于配置文件注释

### 配置文件格式

```toml
[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
description = "CapsLock -> Left Ctrl"

[[keyMappings]]
sourceScanCode = 0x1D
sourceNeedsE0 = false
targetKeyId = "lwin"
mappingType = "additional"
description = "Left Ctrl -> Left Win (for custom layouts)"
```

### 映射表数据结构

```cpp
// 内部映射表: (scanCode, needsE0) -> targetKeyId
std::map<std::pair<unsigned short, bool>, std::string> keyMappings_;
```

使用 `std::map` 存储映射关系，键为 `(scanCode, needsE0)` 对，值为目标按键 ID。这样可以快速查找源按键对应的目标按键。


## Correctness Properties

*属性（Property）是系统在所有有效执行中应该保持为真的特征或行为——本质上是关于系统应该做什么的形式化陈述。属性是人类可读规范和机器可验证正确性保证之间的桥梁。*

### Property Reflection

在分析接受标准后，我们识别出以下冗余：

- **5.2, 5.3, 5.4 与 2.2, 2.3 重复**：这些都是测试映射的核心功能（源按键状态同步到目标按键）。我们将合并为一个综合属性。
- **7.1, 7.2, 7.3 是同一测试场景**：重启功能的不同方面可以合并为一个集成测试示例。
- **配置解析的多个字段测试（1.2-1.5）**：可以合并为一个综合的配置解析往返属性。

### Property 1: 配置解析往返一致性

*对于任意* 有效的映射配置列表，将其序列化为 TOML 格式，然后解析回来，应该得到等价的配置列表（所有字段值相同）。

**Validates: Requirements 1.1, 1.2, 1.3, 1.4, 1.5**

### Property 2: 无效配置被拒绝

*对于任意* 缺少必需字段或包含无效字段类型的映射配置，配置系统应该拒绝该配置并返回错误或警告。

**Validates: Requirements 1.6, 3.3**

### Property 3: 源按键按下同步目标按键

*对于任意* 配置了映射关系的源按键和目标按键，当源按键物理按下时，目标按键的物理状态应该被标记为按下。

**Validates: Requirements 2.2, 5.2, 5.3**

### Property 4: 源按键释放同步目标按键

*对于任意* 配置了映射关系的源按键和目标按键，当源按键物理释放且目标按键本身未被物理按下时，目标按键的物理状态应该被标记为释放。

**Validates: Requirements 2.3, 5.4**

### Property 5: 目标按键本身的状态独立更新

*对于任意* 配置了映射关系的目标按键，当目标按键本身被物理按下或释放时，其物理状态应该正常更新，不受映射关系影响。

**Validates: Requirements 2.4**

### Property 6: 多源按键同时按下保持目标按键按下

*对于任意* 多个源按键映射到同一个目标按键的配置，当任意一个源按键被物理按下时，目标按键应该保持按下状态，直到所有源按键和目标按键本身都被释放。

**Validates: Requirements 2.5, 3.5**

### Property 7: 目标按键 ID 验证

*对于任意* 映射配置，如果 targetKeyId 不是有效的修饰键 ID（lctrl, rctrl, lshift, rshift, lalt, ralt, lwin, rwin），配置系统应该拒绝该映射。

**Validates: Requirements 3.1, 3.3**

### Property 8: 映射类型验证和默认值

*对于任意* 映射配置，如果 mappingType 不是 "additional" 或 "replace"，配置系统应该使用默认值 "additional"。

**Validates: Requirements 3.2, 3.4**

### Property 9: 初始化时建立映射表

*对于任意* 映射配置列表，当物理按键检测器初始化时，应该为每个有效映射建立源按键到目标按键的关联，并且只有目标按键在监控列表中的映射才会被建立。

**Validates: Requirements 4.2, 4.3, 4.4**

### Property 10: 映射查找正确性

*对于任意* 按键事件，物理按键检测器应该正确查找该按键是否存在映射关系，如果存在则应用映射逻辑，如果不存在则正常处理按键事件。

**Validates: Requirements 5.1, 5.5**

## Error Handling

### 配置错误处理

1. **缺少必需字段**
   - 错误：映射配置缺少 `sourceScanCode`、`sourceNeedsE0` 或 `targetKeyId`
   - 处理：忽略该映射，记录警告到 stderr
   - 示例：`Warning: Key mapping missing required field. Mapping ignored.`

2. **无效的目标按键 ID**
   - 错误：`targetKeyId` 不是有效的修饰键 ID
   - 处理：拒绝该映射，记录错误到 stderr
   - 示例：`Error: Invalid target key ID 'invalid_key'. Mapping rejected.`

3. **目标按键未被监控**
   - 错误：`targetKeyId` 对应的按键不在监控列表中
   - 处理：忽略该映射，记录警告到 stderr
   - 示例：`Warning: Key mapping target 'lctrl' is not being monitored. Mapping ignored.`

4. **无效的映射类型**
   - 错误：`mappingType` 不是 "additional" 或 "replace"
   - 处理：使用默认值 "additional"，记录警告到 stderr
   - 示例：`Warning: Invalid mapping type 'invalid'. Using default 'additional'.`

5. **配置文件解析错误**
   - 错误：TOML 格式错误或文件不存在
   - 处理：使用默认配置（空映射列表），记录错误到 stderr
   - 示例：`Error parsing config file: <error details>`

### 运行时错误处理

1. **映射表查找失败**
   - 错误：内部映射表损坏或不一致
   - 处理：跳过映射逻辑，正常处理按键事件
   - 不应该发生（防御性编程）

2. **目标按键状态更新失败**
   - 错误：目标按键在映射表中但在状态列表中找不到
   - 处理：记录错误，跳过状态更新
   - 不应该发生（防御性编程）

### 错误恢复策略

- **配置错误**：使用部分有效的配置继续运行，忽略无效的映射
- **运行时错误**：跳过出错的映射，不影响其他映射和正常功能
- **降级模式**：如果所有映射都无效，系统退化为无映射模式，保持原有功能

## Testing Strategy

### 测试方法

本功能将采用**双重测试方法**：

1. **单元测试（Unit Tests）**：验证特定示例、边界情况和错误条件
2. **基于属性的测试（Property-Based Tests）**：验证通用属性在所有输入上的正确性

两种测试方法是互补的，共同提供全面的测试覆盖：
- 单元测试捕获具体的错误和边界情况
- 基于属性的测试验证通用正确性

### 单元测试策略

**配置解析测试**：
- 测试解析包含单个映射的配置文件
- 测试解析包含多个映射的配置文件
- 测试解析空映射列表
- 测试解析缺少必需字段的配置
- 测试解析包含无效字段类型的配置

**映射初始化测试**：
- 测试使用有效映射配置初始化
- 测试目标按键不在监控列表中的情况
- 测试多个源按键映射到同一目标按键

**按键事件处理测试**：
- 测试源按键按下事件
- 测试源按键释放事件
- 测试目标按键本身的按下/释放事件
- 测试无映射的按键事件

**错误处理测试**：
- 测试各种配置错误的处理
- 测试运行时错误的恢复

### 基于属性的测试策略

**测试框架**：使用 C++ 的 RapidCheck 库进行基于属性的测试。

**测试配置**：
- 每个属性测试运行最少 100 次迭代
- 每个测试使用注释标记对应的设计文档属性
- 标记格式：`// Feature: key-mapping-support, Property N: <property_text>`

**属性测试列表**：

1. **Property 1: 配置解析往返一致性**
   - 生成器：随机生成有效的映射配置列表
   - 测试：序列化 → 解析 → 比较
   - 标记：`// Feature: key-mapping-support, Property 1: 配置解析往返一致性`

2. **Property 2: 无效配置被拒绝**
   - 生成器：随机生成缺少字段或包含无效类型的配置
   - 测试：解析 → 验证返回错误
   - 标记：`// Feature: key-mapping-support, Property 2: 无效配置被拒绝`

3. **Property 3: 源按键按下同步目标按键**
   - 生成器：随机生成映射配置和按键按下事件
   - 测试：处理事件 → 验证目标按键状态为按下
   - 标记：`// Feature: key-mapping-support, Property 3: 源按键按下同步目标按键`

4. **Property 4: 源按键释放同步目标按键**
   - 生成器：随机生成映射配置和按键释放事件
   - 测试：处理事件 → 验证目标按键状态为释放
   - 标记：`// Feature: key-mapping-support, Property 4: 源按键释放同步目标按键`

5. **Property 5: 目标按键本身的状态独立更新**
   - 生成器：随机生成映射配置和目标按键事件
   - 测试：处理事件 → 验证目标按键状态正确更新
   - 标记：`// Feature: key-mapping-support, Property 5: 目标按键本身的状态独立更新`

6. **Property 6: 多源按键同时按下保持目标按键按下**
   - 生成器：随机生成多对一映射配置和按键事件序列
   - 测试：处理事件序列 → 验证目标按键状态
   - 标记：`// Feature: key-mapping-support, Property 6: 多源按键同时按下保持目标按键按下`

7. **Property 7: 目标按键 ID 验证**
   - 生成器：随机生成有效和无效的目标按键 ID
   - 测试：验证 → 检查拒绝无效 ID
   - 标记：`// Feature: key-mapping-support, Property 7: 目标按键 ID 验证`

8. **Property 8: 映射类型验证和默认值**
   - 生成器：随机生成有效和无效的映射类型
   - 测试：验证 → 检查使用默认值
   - 标记：`// Feature: key-mapping-support, Property 8: 映射类型验证和默认值`

9. **Property 9: 初始化时建立映射表**
   - 生成器：随机生成映射配置和监控列表
   - 测试：初始化 → 验证映射表正确建立
   - 标记：`// Feature: key-mapping-support, Property 9: 初始化时建立映射表`

10. **Property 10: 映射查找正确性**
    - 生成器：随机生成映射配置和按键事件
    - 测试：处理事件 → 验证映射查找和应用
    - 标记：`// Feature: key-mapping-support, Property 10: 映射查找正确性`

### 集成测试

**重启功能测试**（手动或自动化 UI 测试）：
- 修改配置文件添加新映射
- 触发 GUI 重启功能
- 验证新映射生效
- 测试错误配置的处理

### 测试数据生成

**有效映射配置生成器**：
```cpp
// 生成随机的有效映射配置
KeyMappingConfig generateValidMapping() {
  unsigned short scanCode = randomScanCode();  // 0x00-0xFF
  bool needsE0 = randomBool();
  std::string targetId = randomModifierKeyId();  // lctrl, rctrl, etc.
  std::string type = randomChoice({"additional", "replace"});
  return KeyMappingConfig(scanCode, needsE0, targetId, type);
}
```

**无效映射配置生成器**：
```cpp
// 生成随机的无效映射配置
KeyMappingConfig generateInvalidMapping() {
  // 随机选择一种无效情况：
  // 1. 无效的 targetKeyId
  // 2. 无效的 mappingType
  // 3. 缺少必需字段（通过特殊标记）
}
```

**按键事件生成器**：
```cpp
// 生成随机的按键事件
InterceptionKeyStroke generateKeyStroke(unsigned short scanCode, 
                                        bool needsE0, 
                                        bool isPressed) {
  InterceptionKeyStroke stroke;
  stroke.code = scanCode;
  stroke.state = (needsE0 ? INTERCEPTION_KEY_E0 : 0) |
                 (isPressed ? 0 : INTERCEPTION_KEY_UP);
  return stroke;
}
```

### 测试覆盖目标

- **代码覆盖率**：目标 > 90%
- **分支覆盖率**：目标 > 85%
- **属性测试迭代**：每个属性 ≥ 100 次
- **错误场景覆盖**：所有已识别的错误情况

### 测试执行

- **开发阶段**：每次代码修改后运行单元测试
- **提交前**：运行完整的单元测试和属性测试套件
- **CI/CD**：自动运行所有测试，包括集成测试
- **发布前**：手动执行集成测试和用户场景测试
