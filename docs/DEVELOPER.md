# 开发者文档

## 项目架构

本项目采用模块化设计，核心逻辑与界面分离，便于维护和扩展。

### 核心模块

#### 1. PhysicalKeyDetector（物理按键检测器）

**功能：** 通过 Interception 驱动检测硬件级别的按键状态

**特点：**
- 事件驱动，实时检测
- 无法被软件模拟按键欺骗
- 需要管理员权限

**API 接口：**

```cpp
class PhysicalKeyDetector {
public:
  PhysicalKeyDetector();
  
  // 初始化检测器
  void initialize();
  
  // 处理按键事件
  void processKeyStroke(const InterceptionKeyStroke &stroke);
  
  // 获取当前状态
  const ModifierKeyStates& getStates() const;
  
  // 检查组合状态
  bool isCtrlPressed() const;
  bool isShiftPressed() const;
  bool isAltPressed() const;
  bool isWinPressed() const;
};
```

**使用示例：**

```cpp
#include "physical_key_detector.h"

PhysicalKeyDetector detector;
detector.initialize();

// 在 Interception 事件循环中
InterceptionKeyStroke stroke;
// ... 接收按键事件 ...
detector.processKeyStroke(stroke);

// 查询状态
if (detector.isCtrlPressed()) {
    // Ctrl 键被按下
}
```

---

#### 2. VirtualKeyDetector（虚拟按键检测器）

**功能：** 通过 Windows API 检测系统层面的按键状态

**特点：**
- 轮询式检测（50ms 间隔）
- 可以检测到软件模拟的按键
- 不需要特殊权限

**API 接口：**

```cpp
class VirtualKeyDetector {
public:
  VirtualKeyDetector();
  
  // 初始化检测器
  void initialize();
  
  // 更新状态（需要定期调用）
  void update();
  
  // 获取当前状态
  const VirtualKeyStates& getStates() const;
  
  // 检查组合状态
  bool isCtrlPressed() const;
  bool isShiftPressed() const;
  bool isAltPressed() const;
  bool isWinPressed() const;
  
  // 检查单个按键
  bool isLeftCtrlPressed() const;
  bool isRightCtrlPressed() const;
  // ... 其他按键 ...
};
```

**使用示例：**

```cpp
#include "virtual_key_detector.h"

VirtualKeyDetector detector;
detector.initialize();

// 在主循环中定期更新
while (running) {
    detector.update();
    
    if (detector.isCtrlPressed()) {
        // 虚拟 Ctrl 键被按下
    }
    
    Sleep(50);
}
```

---

#### 3. ModifierKeyFixer（修饰键修复器）

**功能：** 核心修复逻辑，整合物理和虚拟检测，自动修复卡住的按键

**特点：**
- 智能检测（区分正常使用和真正卡住）
- 可配置阈值
- 支持暂停/恢复
- 统计功能

**API 接口：**

```cpp
class ModifierKeyFixer {
public:
  ModifierKeyFixer();
  
  // 初始化
  bool initialize();
  void cleanup();
  
  // 主处理循环（返回 false 表示出错）
  bool processEvents(int timeoutMs = 50);
  
  // 状态访问
  const ModifierKeyStates& getPhysicalStates() const;
  const VirtualKeyStates& getVirtualStates() const;
  const ModifierMismatchTrackers& getMismatchTrackers() const;
  const FixStatistics& getStatistics() const;
  
  // 控制
  void pause();
  void resume();
  bool isPaused() const;
  
  // 配置
  void setThreshold(int ms);        // 设置卡住判断阈值
  int getThreshold() const;
  void setShowMessages(bool show);  // 是否显示修复消息
  bool getShowMessages() const;
  
  // 检查是否已初始化
  bool isInitialized() const;
};
```

**使用示例：**

```cpp
#include "modifier_key_fixer.h"

ModifierKeyFixer fixer;

// 初始化
if (!fixer.initialize()) {
    // 初始化失败
    return 1;
}

// 配置
fixer.setThreshold(1000);      // 1 秒阈值
fixer.setShowMessages(false);  // 静默模式

// 主循环
while (running) {
    fixer.processEvents(50);
    
    // 获取统计信息
    const auto& stats = fixer.getStatistics();
    printf("Total fixes: %d\n", stats.totalFixes);
}

// 清理
fixer.cleanup();
```

---

### 数据结构

#### ModifierKeyStates（修饰键状态）

```cpp
struct ModifierKeyStates {
  bool lctrl;   // 左 Ctrl
  bool rctrl;   // 右 Ctrl
  bool lshift;  // 左 Shift
  bool rshift;  // 右 Shift
  bool lalt;    // 左 Alt
  bool ralt;    // 右 Alt
  bool lwin;    // 左 Win
  bool rwin;    // 右 Win
  
  // 辅助方法
  bool anyCtrl() const;
  bool anyShift() const;
  bool anyAlt() const;
  bool anyWin() const;
  
  // 比较运算符
  bool operator!=(const ModifierKeyStates &other) const;
};
```

#### VirtualKeyStates（虚拟按键状态）

```cpp
struct VirtualKeyStates {
  bool lctrl;
  bool rctrl;
  bool lshift;
  bool rshift;
  bool lalt;
  bool ralt;
  bool lwin;
  bool rwin;
  
  // 辅助方法
  bool anyCtrl() const;
  bool anyShift() const;
  bool anyAlt() const;
  bool anyWin() const;
  
  // 比较运算符
  bool operator!=(const VirtualKeyStates &other) const;
};
```

#### MismatchTracker（不一致追踪器）

```cpp
struct MismatchTracker {
  bool isMismatched;
  std::chrono::steady_clock::time_point startTime;
  
  void reset();
  void start();
  int getDurationMs() const;
  bool isStuck(int thresholdMs) const;
};
```

#### FixStatistics（修复统计）

```cpp
struct FixStatistics {
  int totalFixes;
  int lctrlFixes;
  int rctrlFixes;
  int lshiftFixes;
  int rshiftFixes;
  int laltFixes;
  int raltFixes;
  int lwinFixes;
  int rwinFixes;
};
```

---

## 构建系统

项目使用 xmake 构建系统。

### 构建目标

```lua
-- 控制台版本
target("escModKey")
    set_kind("binary")
    add_files("src/main.cpp", ...)

-- GUI 版本
target("escModKey_gui")
    set_kind("binary")
    add_files("src/main_gui.cpp", ...)
    add_ldflags("/SUBSYSTEM:WINDOWS", ...)

-- 测试程序
target("test_physical_detector")
target("test_virtual_detector")
```

### 构建命令

```bash
# 构建所有目标
xmake

# 构建特定目标
xmake build escModKey_gui

# 清理
xmake clean

# 运行
xmake run escModKey_gui
```

---

## 扩展开发

### 添加新的检测模块

1. 创建头文件 `include/your_detector.h`
2. 创建实现文件 `src/your_detector.cpp`
3. 在 `ModifierKeyFixer` 中集成
4. 更新 `xmake.lua`

### 创建新的界面版本

参考 `src/main.cpp`（控制台）和 `src/main_gui.cpp`（GUI）的实现方式：

1. 创建新的 main 文件
2. 实例化 `ModifierKeyFixer`
3. 调用 `initialize()` 和 `processEvents()`
4. 实现自己的界面逻辑

### 修改修复逻辑

核心逻辑在 `ModifierKeyFixer::shouldCheckForFix()` 和 `ModifierKeyFixer::fixStuckKeys()` 中。

**修改触发条件：**
编辑 `shouldCheckForFix()` 函数

**修改修复行为：**
编辑 `fixStuckKeys()` 函数

---

## 调试技巧

### 1. 使用控制台版本

控制台版本可以实时显示状态，便于调试：

```bash
.\scripts\run.ps1
```

### 2. 启用调试消息

```cpp
fixer.setShowMessages(true);
```

### 3. 查看诊断信息

```bash
xmake run escModKey
```

### 4. 使用测试程序

```bash
.\scripts\run_test.ps1           # 测试物理检测
.\scripts\run_test_virtual.ps1   # 测试虚拟检测
```

---

## 常见问题

### Q: 如何修改卡住判断的阈值？

```cpp
fixer.setThreshold(2000);  // 改为 2 秒
```

### Q: 如何禁用某个修饰键的修复？

编辑 `ModifierKeyFixer::fixStuckKeys()`，注释掉对应的修复代码。

### Q: 如何添加新的按键支持？

1. 在 `ModifierKeyStates` 中添加新字段
2. 在 `PhysicalKeyDetector` 中添加检测逻辑
3. 在 `VirtualKeyDetector` 中添加检测逻辑
4. 在 `ModifierKeyFixer` 中添加修复逻辑

### Q: 如何更改扫描码？

扫描码定义在 `src/physical_key_detector.cpp` 和 `src/modifier_key_fixer.cpp` 中：

```cpp
#define SCANCODE_LCTRL 0x1D
#define SCANCODE_LSHIFT 0x2A
// ...
```

---

## 依赖项

- **Interception Driver** - 硬件级按键拦截
- **Windows API** - 虚拟按键检测和 GUI
- **C++11** - 标准库（chrono, iostream 等）

---

## 许可证

见项目根目录的 LICENSE 文件。

