# Implementation Plan: Key Mapping Support

## Overview

本实现计划将按键映射支持功能分解为一系列增量式的编码任务。每个任务都建立在前一个任务的基础上，最终实现完整的按键映射功能。实现将遵循以下顺序：配置系统扩展 → 物理按键检测器扩展 → 集成和测试。

## Tasks

- [x] 1. 扩展配置系统以支持按键映射
  - 在 `include/config.h` 中添加 `KeyMappingConfig` 结构体
  - 在 `Config` 类中添加 `keyMappings_` 成员变量和访问方法
  - 在 `src/config.cpp` 的 `load()` 方法中添加读取 `[[keyMappings]]` 的逻辑
  - 在 `src/config.cpp` 的 `save()` 方法中添加保存 `[[keyMappings]]` 的逻辑
  - 添加配置验证逻辑（验证 targetKeyId 和 mappingType）
  - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 3.1, 3.2, 3.3, 3.4_

- [x] 1.1 编写配置解析的单元测试
  - 测试解析包含单个映射的配置
  - 测试解析包含多个映射的配置
  - 测试解析空映射列表
  - 测试解析缺少必需字段的配置
  - 测试解析包含无效字段类型的配置
  - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6_

- [x] 1.2 编写配置解析的属性测试
  - **Property 1: 配置解析往返一致性**
  - **Validates: Requirements 1.1, 1.2, 1.3, 1.4, 1.5**

- [x] 1.3 编写配置验证的属性测试
  - **Property 2: 无效配置被拒绝**
  - **Validates: Requirements 1.6, 3.3**
  - **Property 7: 目标按键 ID 验证**
  - **Validates: Requirements 3.1, 3.3**
  - **Property 8: 映射类型验证和默认值**
  - **Validates: Requirements 3.2, 3.4**

- [x] 2. 扩展物理按键检测器以支持按键映射
  - 在 `include/physical_key_detector.h` 中添加映射相关的成员变量和方法
  - 添加 `keyMappings_` 映射表（`std::map<std::pair<unsigned short, bool>, std::string>`）
  - 添加 `applyKeyMapping()` 辅助方法
  - 扩展 `initializeWithConfig()` 方法以接收映射配置参数
  - 在 `src/physical_key_detector.cpp` 中实现新的 `initializeWithConfig()` 重载
  - 实现映射表的建立逻辑（验证目标按键是否在监控列表中）
  - _Requirements: 4.1, 4.2, 4.3, 4.4_

- [x] 2.1 编写映射初始化的单元测试
  - 测试使用有效映射配置初始化
  - 测试目标按键不在监控列表中的情况
  - 测试多个源按键映射到同一目标按键
  - _Requirements: 4.2, 4.3, 4.4, 3.5_

- [x] 2.2 编写映射初始化的属性测试
  - **Property 9: 初始化时建立映射表**
  - **Validates: Requirements 4.2, 4.3, 4.4**

- [ ] 3. 实现按键事件的映射处理逻辑
  - 修改 `src/physical_key_detector.cpp` 中的 `processKeyStroke()` 方法
  - 在处理修饰键后添加映射查找逻辑
  - 实现 `applyKeyMapping()` 方法（同步更新目标按键状态）
  - 处理 additional 映射类型的逻辑
  - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 2.2, 2.3, 2.4, 2.5_

- [ ] 3.1 编写按键事件处理的单元测试
  - 测试源按键按下事件
  - 测试源按键释放事件
  - 测试目标按键本身的按下/释放事件
  - 测试无映射的按键事件
  - 测试源按键和目标按键同时按下的情况
  - _Requirements: 2.2, 2.3, 2.4, 2.5, 5.1, 5.5_

- [ ] 3.2 编写按键事件处理的属性测试
  - **Property 3: 源按键按下同步目标按键**
  - **Validates: Requirements 2.2, 5.2, 5.3**
  - **Property 4: 源按键释放同步目标按键**
  - **Validates: Requirements 2.3, 5.4**
  - **Property 5: 目标按键本身的状态独立更新**
  - **Validates: Requirements 2.4**
  - **Property 6: 多源按键同时按下保持目标按键按下**
  - **Validates: Requirements 2.5, 3.5**
  - **Property 10: 映射查找正确性**
  - **Validates: Requirements 5.1, 5.5**

- [ ] 4. 集成映射配置到主程序
  - 修改 `src/main.cpp` 和 `src/main_gui.cpp`
  - 在初始化 `PhysicalKeyDetector` 时传递映射配置
  - 确保重启功能能够重新加载映射配置
  - _Requirements: 7.1, 7.2, 7.3_

- [ ] 4.1 编写集成测试
  - 测试完整的配置加载和应用流程
  - 测试重启功能重新加载映射配置
  - 测试错误配置的处理
  - _Requirements: 7.1, 7.2, 7.3, 7.4_

- [ ] 5. 更新配置文件和文档
  - 在 `config.toml` 中添加 `[[keyMappings]]` 示例和注释
  - 提供 CapsLock → Ctrl 的示例配置
  - 更新 `USER_GUIDE.md` 添加按键映射功能说明
  - 说明 sourceKey 的格式（扫描码和 E0 标志）
  - 说明 targetKey 的有效值（修饰键 ID）
  - 说明 mappingType 的可选值和含义
  - 提供常见映射场景的示例
  - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6_

- [ ] 6. Checkpoint - 确保所有测试通过
  - 运行所有单元测试和属性测试
  - 验证配置文件示例可以正确解析
  - 手动测试重启功能
  - 如有问题，询问用户

## Notes

- 每个任务都引用了具体的需求以便追溯
- Checkpoint 任务确保增量验证
- 属性测试验证通用正确性属性
- 单元测试验证特定示例和边界情况
- 建议使用 RapidCheck 库进行基于属性的测试
