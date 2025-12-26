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
- ✅ 支持按键映射配置（兼容 AutoHotkey 等按键映射工具）
- ✅ 区分左右修饰键
- ✅ 可配置阈值和行为
- ✅ 系统托盘模式（GUI 版本）
- ✅ 统计追踪

## 快速开始

### 安装

1. 安装 [Interception 驱动](https://github.com/oblitum/Interception)
2. 下载或编译本工具
3. 以管理员权限运行

### 运行

```powershell
# GUI 版本（推荐，系统托盘）
.\scripts\run_gui.ps1

# 控制台版本（调试用）
.\scripts\run.ps1
```

### 配置

编辑 `config.toml` 配置文件：

```toml
[general]
thresholdMs = 1000  # 判定卡键的时间阈值

[keys]
monitorCtrl = true   # 监控 Ctrl 键
monitorShift = true  # 监控 Shift 键
monitorAlt = true    # 监控 Alt 键
monitorWin = true    # 监控 Win 键

# 按键映射配置（如果使用 AutoHotkey 等工具映射按键）
[[keyMappings]]
sourceScanCode = 0x3A      # CapsLock 的扫描码
sourceNeedsE0 = false      # 是否需要 E0 标志
targetKeyId = "lctrl"      # 映射到左 Ctrl
mappingType = "additional" # 映射类型
description = "CapsLock -> Left Ctrl"
```

## 系统要求

- Windows 操作系统
- [Interception 驱动](https://github.com/oblitum/Interception)
- 管理员权限

## 编译

```bash
# 安装 xmake
# 然后执行
xmake
```

编译产物：
- `escModKey.exe` - 控制台版本
- `escModKey_gui.exe` - GUI 版本（推荐）

## 文档

- **[USER_GUIDE.md](USER_GUIDE.md)** - 完整用户指南（使用说明、配置、故障排除）
- **[docs/CONFIG.md](docs/CONFIG.md)** - 配置文件详细说明
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - 架构设计文档
- **[docs/DEVELOPER.md](docs/DEVELOPER.md)** - 开发者指南

## 项目结构

```
escModKey/
├── src/                    # 源代码
├── include/                # 头文件
├── test/                   # 测试程序
├── docs/                   # 文档
├── scripts/                # 运行脚本
├── config.toml             # 配置文件
└── USER_GUIDE.md           # 用户指南
```

## 常见问题

**Q: 如何配置按键映射？**  
A: 参见 [USER_GUIDE.md - 按键映射支持](USER_GUIDE.md#按键映射支持)

**Q: 为什么需要管理员权限？**  
A: Interception 驱动需要管理员权限才能监控硬件按键

**Q: 如何调整卡键判定时间？**  
A: 编辑 `config.toml` 中的 `thresholdMs` 参数

更多问题请参阅 [USER_GUIDE.md](USER_GUIDE.md)。

## 许可证

参见 LICENSE 文件。
