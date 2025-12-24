# 修饰键自动修复工具

自动修复由自动化软件导致的"卡住"的修饰键（Ctrl、Shift、Alt、Win）。

## 问题描述

使用键盘自动化工具（AutoHotkey、Quicker 等）时，修饰键可能会卡在虚拟按下状态，即使物理按键已经释放，导致产生意外的组合键。

**示例场景：**
- 运行一个使用 `Alt+X` 的 AutoHotkey 脚本
- 脚本执行完毕，但系统仍认为 Alt 处于按下状态
- 当你按下 `X` 时，触发的是 `Alt+X` 而不是单独的 `X`
- 必须手动按一次 Alt 键才能"解除卡住"

## 解决方案

本工具实时监控物理按键和虚拟按键状态，当检测到按键卡住时自动释放。

## 功能特性

- ✅ 实时监控物理和虚拟按键状态
- ✅ 智能自动修复（仅在真正卡住时修复，不干扰正常使用）
- ✅ 防止误判（兼容按键映射工具、脚本等）
- ✅ 区分左右修饰键
- ✅ 统计追踪
- ✅ 可配置阈值（默认：1000ms）
- ✅ 无干扰运行
- ✅ 系统托盘模式（GUI 版本）

## 快速开始

```powershell
# 编译
xmake

# 运行控制台版本（需要管理员权限）
.\scripts\run.ps1

# 运行 GUI 版本（系统托盘，需要管理员权限）
.\run_gui.ps1
```

**详细使用说明请参阅 [USER_GUIDE.md](USER_GUIDE.md)**

## 工作原理

1. **监控** 物理按键（硬件）和虚拟按键（系统）状态
2. **检测** 修饰键卡住（虚拟按下，物理释放）
3. **等待** 阈值时间（1000ms）以避免误判
4. **自动修复** 当你按下任意按键时，发送释放事件

## 系统要求

- Windows 操作系统
- 已安装 Interception 驱动
- 管理员权限
- Visual Studio 2019 或更高版本（用于编译）

## 项目结构

```
escModKey/
├── include/                          # 头文件
│   ├── interception.h               # Interception 驱动 API
│   ├── physical_key_detector.h      # 物理按键检测模块
│   ├── virtual_key_detector.h       # 虚拟按键检测模块
│   └── modifier_key_fixer.h         # 修饰键修复核心模块
├── src/                             # 源文件
│   ├── main.cpp                     # 主程序（控制台版本）
│   ├── main_gui.cpp                 # GUI 版本（系统托盘）
│   ├── physical_key_detector.cpp    # 物理检测器实现
│   ├── virtual_key_detector.cpp     # 虚拟检测器实现
│   └── modifier_key_fixer.cpp       # 修复器实现
├── test/                            # 测试程序
│   ├── test_physical_detector.cpp   # 物理检测器测试
│   └── test_virtual_detector.cpp    # 虚拟检测器测试
├── lib/                             # 库文件
│   ├── interception.dll
│   └── interception.lib
├── resources/                       # 资源文件
│   ├── app.rc                       # 资源脚本
│   ├── resource.h                   # 资源头文件
│   ├── app_icon.ico                 # 应用图标（需自行添加）
│   └── README.md                    # 图标说明
├── docs/                            # 文档
│   ├── ARCHITECTURE.md              # 架构设计文档
│   └── DEVELOPER.md                 # 开发者指南
├── scripts/                         # 辅助脚本
│   ├── run.ps1                      # 运行控制台版本
│   ├── run_test.ps1                 # 运行物理检测器测试
│   └── run_test_virtual.ps1         # 运行虚拟检测器测试
├── USER_GUIDE.md                    # 用户指南
├── run_gui.ps1                      # 运行 GUI 版本（主要使用）
└── xmake.lua                        # 构建配置
```

## 核心模块

### PhysicalKeyDetector（物理按键检测器）

通过 Interception 驱动进行硬件级别的修饰键状态检测。
- 无法被软件按键模拟欺骗
- 事件驱动，实时检测
- 需要管理员权限

### VirtualKeyDetector（虚拟按键检测器）

通过 Windows API 进行软件级别的修饰键状态检测。
- 检测系统视角的按键状态
- 轮询式检测（50ms 间隔）
- 无需特殊权限

### ModifierKeyFixer（修饰键修复器）

核心修复逻辑模块，整合物理和虚拟检测器。
- 不一致检测和时间追踪
- 智能修复触发条件判断
- 统计和配置管理

## 编译

**重要：编译前需要准备图标文件**

1. 准备一个 .ico 格式的图标文件
2. 将图标文件命名为 `app_icon.ico`
3. 放置在 `resources/` 目录下
4. 参考 `resources/README.md` 了解如何创建图标

然后执行编译：

```bash
xmake
```

编译产物：
- `escModKey.exe` - 控制台版本主程序
- `escModKey_gui.exe` - GUI 版本（系统托盘）
- `test_physical_detector.exe` - 物理检测器测试
- `test_virtual_detector.exe` - 虚拟检测器测试

## 运行

### 主程序

**GUI 版本（推荐）：**
```powershell
.\run_gui.ps1
```
- 无窗口运行
- 系统托盘图标
- 右键菜单（暂停/恢复、显示统计、退出）
- 双击显示统计信息
- 气泡通知

**控制台版本：**
```powershell
.\scripts\run.ps1
```
- 实时显示按键状态
- 按 P 键暂停/恢复
- 按 ESC 键退出

### 测试程序

```powershell
# 测试物理检测器（需要管理员权限）
.\scripts\run_test.ps1

# 测试虚拟检测器（无需管理员权限）
.\scripts\run_test_virtual.ps1
```

## 配置

编辑 `include/modifier_key_fixer.h`：

```cpp
const int MISMATCH_THRESHOLD_MS = 1000;  // 调整阈值
```

## 使用场景

- 修复 AutoHotkey 脚本执行后的卡键
- 从自动化软件故障中恢复
- 防止意外的组合键触发
- 改善使用自动化工具的工作流程

## 故障排除

**"Failed to create Interception context"**
- 安装 Interception 驱动
- 以管理员权限运行

**修复后按键仍然卡住**
- 增加阈值
- 检查自动化软件是否仍在运行

**误判（不需要时也修复）**
- 增加 `MISMATCH_THRESHOLD_MS` 到 2000 以上

更多故障排除提示请参阅 [USER_GUIDE.md](USER_GUIDE.md)。

## 文档

- [USER_GUIDE.md](USER_GUIDE.md) - 详细用户指南
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) - 架构设计文档
- [docs/DEVELOPER.md](docs/DEVELOPER.md) - 开发者指南

## 许可证

参见 LICENSE 文件。
