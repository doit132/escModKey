# 修饰键自动修复工具 - 用户指南

## 这个工具是什么？

本工具自动修复"卡住"的修饰键（Ctrl、Shift、Alt、Win），即使你已经物理释放了按键，但它们仍然保持在按下状态。这种情况通常发生在使用 AutoHotkey 或 Quicker 等自动化软件时。

## 问题描述

使用键盘自动化或按键映射软件时，修饰键有时会"卡"在虚拟按下状态：

- 运行一个使用 `Alt+X` 的 AutoHotkey 脚本
- 脚本执行完毕，但系统仍然认为 Alt 处于按下状态
- 当你再次按下 `X` 时，触发的是 `Alt+X` 而不是单独的 `X`
- 你必须手动按一次物理 Alt 键才能"解除卡住"

## 解决方案

本工具实时监控物理和虚拟按键状态：

1. **检测** 修饰键卡住（虚拟按下，物理释放）
2. **等待** 阈值时间（默认 1000ms）以避免误判
3. **自动修复** 当你按下任意按键时，为卡住的按键发送释放事件

## 功能特性

- ✅ 实时监控所有修饰键
- ✅ 区分左右按键
- ✅ 智能检测（避免干扰按键映射工具）
- ✅ 无干扰运行（仅在需要时修复）
- ✅ 统计追踪
- ✅ 可配置阈值
- ✅ 系统托盘模式（GUI 版本）

## 系统要求

- Windows 操作系统
- 已安装 Interception 驱动
- 管理员权限

## 安装

1. 安装 Interception 驱动
2. 编译项目：`xmake`
3. 以管理员权限运行：`.\run_gui.ps1`（GUI 版本）或 `.\scripts\run.ps1`（控制台版本）

## 使用方法

### GUI 版本（推荐）

1. 以管理员权限运行 `.\run_gui.ps1`
2. 程序在后台运行，系统托盘显示图标
3. 右键托盘图标可以：
   - 暂停/恢复监控
   - 显示统计信息
   - 重启程序（重新加载配置）
   - 退出程序
4. 双击托盘图标显示统计信息
5. 当修复按键时会显示气泡通知

**重启功能**：修改配置文件后，右键托盘图标选择"Restart (Reload Config)"即可重新加载配置，无需退出程序。

### 控制台版本

1. 以管理员权限运行程序
2. 程序在后台监控你的按键
3. 当修饰键卡住时：
   - 等待它被标记为 [STUCK!]（1000ms 后）
   - 按下任意按键触发自动修复
   - 卡住的按键将自动释放
4. 按 P 键暂停/恢复监控
5. 按 ESC 键退出程序

### 理解显示信息

```
Left Ctrl   : Physical[RELEASED] Virtual[PRESSED] <-- MISMATCH (1523ms) [STUCK!]
```

- **Physical**：实际硬件按键状态
- **Virtual**：系统视角的按键状态
- **MISMATCH**：状态不匹配
- **Duration**：不匹配持续时间
- **[STUCK!]**：按键被认为卡住（持续时间 > 阈值）

### 状态消息

- `All keys normal. Monitoring...` - 一切正常
- `Stuck keys detected! Press any key to auto-fix.` - 检测到卡键
- `[Auto-Fix Triggered]` - 修复过程已启动
- `[Fixed] Left Ctrl` - 特定按键已修复

## 配置

程序支持通过 `config.toml` 配置文件进行自定义配置。配置文件位于程序目录或用户配置目录。

### 配置文件位置

- 程序目录：`.\config.toml`（优先）
- 用户目录：`%APPDATA%\ModifierKeyAutoFix\config.toml`

### 重新加载配置

**GUI 版本**：修改配置文件后，右键托盘图标选择"Restart (Reload Config)"即可重新加载配置，无需退出程序。

**控制台版本**：需要退出程序（按 ESC）后重新启动。

### 配置选项

详细配置说明请参考 `config.toml` 文件中的注释。主要配置项包括：

- **阈值设置**：`thresholdMs` - 判定按键卡住的时间阈值（默认 1000ms）
- **按键监控**：`monitorCtrl/Shift/Alt/Win` - 快速开关标准修饰键监控
- **禁用按键**：`disabledKeys` - 禁用特定的左右键
- **自定义按键**：`customKeys` - 添加自定义按键监控（如 CapsLock）
- **按键映射**：`keyMappings` - 配置非修饰键到修饰键的映射关系
- **通知设置**：`notificationsEnabled`, `notifyOnFix`, `notifyOnStartup` - 控制通知行为

### 配置示例

```toml
[general]
thresholdMs = 1000

[keys]
monitorCtrl = true
monitorShift = true
monitorAlt = true
monitorWin = false  # 禁用 Win 键监控

disabledKeys = ["rwin"]  # 禁用右 Win 键
customKeys = [[0x3A, false, "CapsLock", 0x14]]  # 添加 CapsLock 监控

# 按键映射示例
[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
description = "CapsLock -> Left Ctrl"
```

## 按键映射支持

### 什么是按键映射？

如果你使用按键映射软件（如 AutoHotkey、PowerToys、SharpKeys 等）将非修饰键映射为修饰键，本工具需要知道这些映射关系，以便正确追踪物理按键状态。

### 为什么需要配置映射？

**问题场景**：假设你使用 AutoHotkey 将 CapsLock 映射为 Ctrl：

1. 你按下 CapsLock（物理按键）
2. AutoHotkey 将其转换为 Ctrl（虚拟按键）
3. 本工具检测到：
   - 物理 Ctrl：未按下
   - 虚拟 Ctrl：按下
4. 工具误判为"Ctrl 卡住"并尝试修复

**解决方案**：配置映射关系后，工具知道 CapsLock 物理按下时应该同步更新 Ctrl 的物理状态，避免误判。

### 配置按键映射

在 `config.toml` 文件中添加 `[[keyMappings]]` 节：

```toml
[[keyMappings]]
sourceScanCode = 0x3A      # 源按键扫描码
sourceNeedsE0 = false      # 源按键是否需要 E0 标志
targetKeyId = "lctrl"      # 目标修饰键 ID
mappingType = "additional" # 映射类型
description = "CapsLock -> Left Ctrl"  # 可选描述
```

### 配置字段说明

#### sourceScanCode（源按键扫描码）

源按键的硬件扫描码，十六进制格式（0x00-0xFF）。

**常见按键扫描码**：

| 按键 | 扫描码 | 说明 |
|------|--------|------|
| CapsLock | 0x3A | 大写锁定键 |
| Left Ctrl | 0x1D | 左 Ctrl |
| Right Ctrl | 0x1D | 右 Ctrl（需要 E0） |
| Left Alt | 0x38 | 左 Alt |
| Right Alt | 0x38 | 右 Alt（需要 E0） |
| Left Shift | 0x2A | 左 Shift |
| Right Shift | 0x36 | 右 Shift |
| Tab | 0x0F | Tab 键 |
| Backspace | 0x0E | 退格键 |

**如何查找扫描码**：
- 使用在线工具：[Scan Code Mapper](https://www.win.tue.nl/~aeb/linux/kbd/scancodes.html)
- 使用 AutoHotkey 的 `KeyHistory` 命令
- 参考 Windows 扫描码文档

#### sourceNeedsE0（E0 标志）

布尔值，指示源按键是否需要 E0 扩展标志。

**规则**：
- `false`：大多数标准按键（CapsLock、左 Ctrl、左 Alt、左 Shift 等）
- `true`：扩展按键（右 Ctrl、右 Alt、右 Win、方向键、Home/End 等）

**示例**：
```toml
# CapsLock 不需要 E0
sourceScanCode = 0x3A
sourceNeedsE0 = false

# 右 Alt 需要 E0
sourceScanCode = 0x38
sourceNeedsE0 = true
```

#### targetKeyId（目标修饰键 ID）

目标修饰键的标识符，必须是以下值之一：

| 目标键 ID | 说明 |
|-----------|------|
| `"lctrl"` | 左 Ctrl |
| `"rctrl"` | 右 Ctrl |
| `"lshift"` | 左 Shift |
| `"rshift"` | 右 Shift |
| `"lalt"` | 左 Alt |
| `"ralt"` | 右 Alt |
| `"lwin"` | 左 Win |
| `"rwin"` | 右 Win |

**注意**：
- 使用小写字母
- 目标按键必须在监控列表中（通过 `monitorCtrl/Shift/Alt/Win` 启用）
- 如果目标按键未被监控，映射将被忽略并记录警告

#### mappingType（映射类型）

映射行为类型，当前支持：

| 类型 | 说明 | 推荐 |
|------|------|------|
| `"additional"` | 源按键和目标按键都被监控，源按键按下时同步更新目标按键状态 | ✅ 推荐 |
| `"replace"` | 源按键完全替换目标按键（尚未实现） | ❌ 未实现 |

**默认值**：如果未指定或值无效，默认使用 `"additional"`。

**additional 类型行为**：
- 源按键按下 → 目标按键物理状态标记为按下
- 源按键释放 → 目标按键物理状态标记为释放
- 目标按键本身按下 → 正常更新物理状态
- 多个源按键映射到同一目标 → 任意一个按下时目标保持按下

#### description（描述）

可选字段，用于说明映射用途，便于配置文件管理。

```toml
description = "CapsLock -> Left Ctrl for Vim users"
```

### 常见映射场景

#### 1. CapsLock → Left Ctrl（最常见）

**使用场景**：Vim 用户、Emacs 用户、程序员常用映射

**AutoHotkey 脚本**：
```ahk
CapsLock::LCtrl
```

**配置**：
```toml
[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
description = "CapsLock -> Left Ctrl"
```

#### 2. Left Ctrl → Left Win（自定义布局）

**使用场景**：将 Ctrl 功能移到更容易按的位置

**AutoHotkey 脚本**：
```ahk
LCtrl::LWin
```

**配置**：
```toml
[[keyMappings]]
sourceScanCode = 0x1D
sourceNeedsE0 = false
targetKeyId = "lwin"
mappingType = "additional"
description = "Left Ctrl -> Left Win"
```

#### 3. Right Alt → Right Ctrl（国际键盘）

**使用场景**：国际键盘布局优化

**AutoHotkey 脚本**：
```ahk
RAlt::RCtrl
```

**配置**：
```toml
[[keyMappings]]
sourceScanCode = 0x38
sourceNeedsE0 = true  # 注意：右 Alt 需要 E0
targetKeyId = "rctrl"
mappingType = "additional"
description = "Right Alt -> Right Ctrl"
```

#### 4. Tab → Left Ctrl（高级布局）

**使用场景**：极简主义键盘布局

**AutoHotkey 脚本**：
```ahk
Tab::LCtrl
```

**配置**：
```toml
[[keyMappings]]
sourceScanCode = 0x0F
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
description = "Tab -> Left Ctrl"
```

#### 5. 多个源按键映射到同一目标

**使用场景**：多个按键都可以触发同一修饰键

**配置**：
```toml
[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
description = "CapsLock -> Left Ctrl"

[[keyMappings]]
sourceScanCode = 0x0F
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
description = "Tab -> Left Ctrl"
```

### 配置验证

工具会在启动时验证映射配置：

**有效配置**：
- ✅ 所有必需字段都存在
- ✅ targetKeyId 是有效的修饰键 ID
- ✅ 目标按键在监控列表中
- ✅ mappingType 是有效值

**无效配置处理**：
- ❌ 缺少必需字段 → 忽略该映射，记录警告
- ❌ 无效的 targetKeyId → 拒绝该映射，记录错误
- ❌ 目标按键未被监控 → 忽略该映射，记录警告
- ❌ 无效的 mappingType → 使用默认值 "additional"，记录警告

### 测试映射配置

配置映射后，测试是否生效：

1. **启动工具**（GUI 或控制台版本）
2. **检查日志**：确认没有警告或错误
3. **测试映射**：
   - 按下源按键（如 CapsLock）
   - 观察目标按键（如 Ctrl）的物理状态是否更新
   - 确认不会误判为"卡住"
4. **测试目标按键本身**：
   - 直接按下目标按键（如 Ctrl）
   - 确认状态正常更新

### 故障排除

#### 映射未生效

**检查清单**：
1. ✅ 配置文件格式正确（TOML 语法）
2. ✅ 扫描码和 E0 标志正确
3. ✅ targetKeyId 拼写正确（小写）
4. ✅ 目标按键已启用监控（`monitorCtrl = true` 等）
5. ✅ 已重启工具或重新加载配置

**查看日志**：
- 控制台版本：直接显示警告和错误
- GUI 版本：检查是否有错误通知

#### 仍然误判为卡住

**可能原因**：
1. 映射配置不正确
2. 按键映射软件使用了不同的映射方式
3. 扫描码或 E0 标志错误

**解决方法**：
1. 使用 AutoHotkey 的 `KeyHistory` 查看实际扫描码
2. 尝试切换 E0 标志（true/false）
3. 检查按键映射软件的配置

#### 配置文件解析错误

**常见错误**：
```
Error parsing config file: expected '=', found '>'
```

**解决方法**：
- 检查 TOML 语法（特别是引号、等号、逗号）
- 使用 TOML 验证工具检查格式
- 参考 `config.toml` 中的示例

### 高级用法

#### 与自定义按键结合

如果你将非修饰键映射为修饰键，可能还需要监控源按键：

```toml
[keys]
# 监控 CapsLock 作为自定义按键
customKeys = [[0x3A, false, "CapsLock", 0x14]]

# 配置 CapsLock -> Ctrl 映射
[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lctrl"
mappingType = "additional"
description = "CapsLock -> Left Ctrl"
```

#### 禁用特定修饰键

如果你完全用映射替换了某个修饰键，可以禁用原始按键：

```toml
[keys]
# 禁用右 Win 键（因为已映射到其他键）
disabledKeys = ["rwin"]

# 配置映射
[[keyMappings]]
sourceScanCode = 0x3A
sourceNeedsE0 = false
targetKeyId = "lwin"
mappingType = "additional"
description = "CapsLock -> Left Win"
```

## 工作原理

### 检测逻辑

1. **物理检测**：使用 Interception 驱动监控实际按键
2. **虚拟检测**：使用 Windows API 检查系统按键状态
3. **不一致检测**：比较物理和虚拟状态
4. **时间追踪**：记录不一致持续时间

### 修复触发条件

工具仅在满足所有条件时修复：

1. ✅ 修饰键卡住（不一致 > 阈值）
2. ✅ 你按下任意按键（物理按键按下事件）
3. ✅ 没有其他物理按键被按下（避免干扰按键映射）

### 智能保护

工具避免干扰：

- **正常按键**：短暂不一致（< 1000ms）被忽略
- **按键映射工具**：如果你将空格映射为 Ctrl，不会干扰
- **自动化脚本**：仅在脚本完成后修复（阈值延迟）

## 故障排除

### "Failed to create Interception context"

- 确保已安装 Interception 驱动
- 以管理员权限运行
- 检查是否有其他程序正在使用 Interception

### 修复后按键仍然卡住

- 如果修复发生太早，增加阈值
- 检查自动化软件是否持续发送按键
- 尝试手动按下卡住的按键

### 误判（不需要时也修复）

- 增加 `MISMATCH_THRESHOLD_MS` 到 2000 或更高
- 检查是否长时间按住按键

### 修复未触发

- 确保按键显示 [STUCK!] 标记
- 尝试按下非修饰键（如字母键）
- 检查没有其他物理按键被按下

## 统计信息

退出时（按 ESC 或右键菜单退出），工具显示：

- 执行的修复总次数
- 按键分类统计（哪些按键最常卡住）

这有助于识别有问题的脚本或模式。

## 使用技巧

1. **开机启动**：添加到 Windows 启动项以持续保护
2. **监控模式**：检查统计信息以识别有问题的脚本
3. **调整阈值**：根据使用模式微调
4. **后台运行**：使用 GUI 版本在系统托盘静默运行

## 已知限制

1. **需要管理员权限**：Interception 驱动需要
2. **仅限 Windows**：使用 Windows 特定 API
3. **阈值延迟**：修复前有 1 秒延迟（设计如此）
4. **远程桌面**：在 RDP 会话中可能无法正常工作

## 支持

如果遇到问题：

1. 检查 Interception 驱动是否正确安装
2. 验证是否以管理员权限运行
3. 尝试调整阈值
4. 检查统计信息以了解正在修复的内容

## 许可证

参见项目 LICENSE 文件。

