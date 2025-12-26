# Requirements Document

## Introduction

本文档定义了按键映射支持功能的需求。该功能允许用户配置非修饰键到修饰键的映射关系，使得当用户通过按键映射软件（如 AutoHotkey）将非修饰键映射为修饰键时，本工具能够正确识别物理按键状态，避免误判为"卡住"并触发不必要的修复。

## Glossary

- **Physical_Key_Detector**: 物理按键检测器，负责监控实际硬件按键状态
- **Source_Key**: 源按键，被映射的原始按键（如 CapsLock）
- **Target_Key**: 目标按键，映射的目标修饰键（如 Ctrl）
- **Key_Mapping**: 按键映射关系，定义源按键到目标按键的映射
- **Mapping_Type**: 映射类型，定义映射行为（additional 或 replace）
- **Config_System**: 配置系统，负责读取和管理配置文件
- **Modifier_Key**: 修饰键，包括 Ctrl、Shift、Alt、Win 等

## Requirements

### Requirement 1: 配置按键映射关系

**User Story:** 作为用户，我想要在配置文件中定义按键映射关系，以便工具能够识别我使用的按键映射软件的映射配置。

#### Acceptance Criteria

1. WHEN 配置文件包含 `[[keyMappings]]` 节时，THE Config_System SHALL 解析所有映射配置项
2. WHEN 解析映射配置时，THE Config_System SHALL 读取 sourceKey（源按键扫描码和 E0 标志）
3. WHEN 解析映射配置时，THE Config_System SHALL 读取 targetKey（目标按键 ID）
4. WHEN 解析映射配置时，THE Config_System SHALL 读取 mappingType（映射类型）
5. WHEN 解析映射配置时，THE Config_System SHALL 读取可选的 description（映射描述）
6. IF 映射配置格式错误或缺少必需字段，THEN THE Config_System SHALL 忽略该映射并记录警告
7. WHEN 配置文件不包含 `[[keyMappings]]` 节时，THE Config_System SHALL 使用空映射列表

### Requirement 2: 支持 Additional 映射类型

**User Story:** 作为用户，我想要使用 additional 映射类型，以便源按键和目标按键都被监控，适用于单向映射场景。

#### Acceptance Criteria

1. WHEN mappingType 为 "additional" 时，THE Physical_Key_Detector SHALL 同时监控源按键和目标按键的物理状态
2. WHEN 源按键物理按下时，THE Physical_Key_Detector SHALL 将目标按键的物理状态标记为按下
3. WHEN 源按键物理释放时，THE Physical_Key_Detector SHALL 将目标按键的物理状态标记为释放
4. WHEN 目标按键本身被物理按下时，THE Physical_Key_Detector SHALL 正常更新目标按键的物理状态
5. WHEN 源按键和目标按键同时被物理按下时，THE Physical_Key_Detector SHALL 保持目标按键的物理状态为按下

### Requirement 3: 验证映射配置

**User Story:** 作为用户，我想要工具验证我的映射配置，以便及早发现配置错误。

#### Acceptance Criteria

1. WHEN 加载映射配置时，THE Config_System SHALL 验证 targetKey 是否为有效的修饰键 ID
2. WHEN 加载映射配置时，THE Config_System SHALL 验证 mappingType 是否为 "additional" 或 "replace"
3. IF targetKey 不是修饰键 ID，THEN THE Config_System SHALL 拒绝该映射并记录错误
4. IF mappingType 不是有效值，THEN THE Config_System SHALL 使用默认值 "additional"
5. WHEN 多个映射指向同一个目标按键时，THE Config_System SHALL 允许该配置并正常工作

### Requirement 4: 初始化映射关系

**User Story:** 作为开发者，我想要在检测器初始化时应用映射配置，以便映射关系在程序运行期间生效。

#### Acceptance Criteria

1. WHEN Physical_Key_Detector 初始化时，THE Physical_Key_Detector SHALL 接收映射配置列表
2. WHEN 接收到映射配置后，THE Physical_Key_Detector SHALL 为每个映射建立源按键到目标按键的关联
3. WHEN 建立关联时，THE Physical_Key_Detector SHALL 验证目标按键是否在监控列表中
4. IF 目标按键不在监控列表中，THEN THE Physical_Key_Detector SHALL 忽略该映射并记录警告
5. WHEN 初始化完成后，THE Physical_Key_Detector SHALL 准备好处理映射的按键事件

### Requirement 5: 处理映射的按键事件

**User Story:** 作为用户，我想要工具正确处理映射的按键事件，以便避免误判为"卡住"。

#### Acceptance Criteria

1. WHEN 检测到源按键的按键事件时，THE Physical_Key_Detector SHALL 检查是否存在该按键的映射
2. WHEN 存在映射且为 additional 类型时，THE Physical_Key_Detector SHALL 同步更新目标按键的物理状态
3. WHEN 源按键按下时，THE Physical_Key_Detector SHALL 将目标按键的物理状态设置为按下
4. WHEN 源按键释放时，THE Physical_Key_Detector SHALL 将目标按键的物理状态设置为释放
5. WHEN 不存在映射时，THE Physical_Key_Detector SHALL 正常处理按键事件

### Requirement 6: 配置文件示例和文档

**User Story:** 作为用户，我想要查看配置示例和文档，以便了解如何配置按键映射。

#### Acceptance Criteria

1. WHEN 用户查看配置文件时，THE Config_System SHALL 提供 `[[keyMappings]]` 的注释说明
2. WHEN 用户查看配置文件时，THE Config_System SHALL 提供至少一个映射配置示例
3. WHEN 用户查看文档时，THE Config_System SHALL 说明 sourceKey 的格式（扫描码和 E0 标志）
4. WHEN 用户查看文档时，THE Config_System SHALL 说明 targetKey 的有效值（修饰键 ID）
5. WHEN 用户查看文档时，THE Config_System SHALL 说明 mappingType 的可选值和含义
6. WHEN 用户查看文档时，THE Config_System SHALL 提供常见映射场景的示例（如 CapsLock 映射为 Ctrl）

### Requirement 7: 支持重启时重新加载映射

**User Story:** 作为用户，我想要在修改映射配置后通过重启功能重新加载，以便无需退出程序即可应用新的映射配置。

#### Acceptance Criteria

1. WHEN 用户通过 GUI 重启功能重启程序时，THE Config_System SHALL 重新读取映射配置
2. WHEN 重新读取映射配置后，THE Physical_Key_Detector SHALL 使用新的映射配置重新初始化
3. WHEN 重新初始化完成后，THE Physical_Key_Detector SHALL 应用新的映射关系
4. IF 新的映射配置包含错误，THEN THE Config_System SHALL 显示错误通知并使用旧配置
5. WHEN 重启成功后，THE Config_System SHALL 显示成功通知
