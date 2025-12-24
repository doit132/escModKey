# 架构设计文档

## 概述

修饰键自动修复工具采用分层架构设计，将检测、判断、修复三个核心功能模块化，实现了高内聚、低耦合的代码结构。

---

## 系统架构图

```
┌─────────────────────────────────────────────────────────┐
│                    用户界面层                            │
│  ┌──────────────┐              ┌──────────────┐        │
│  │ 控制台界面    │              │  GUI 界面     │        │
│  │ (main.cpp)   │              │(main_gui.cpp)│        │
│  └──────┬───────┘              └──────┬───────┘        │
└─────────┼──────────────────────────────┼───────────────┘
          │                              │
          └──────────────┬───────────────┘
                         │
┌────────────────────────┼───────────────────────────────┐
│                 核心逻辑层                               │
│         ┌───────────────────────────┐                  │
│         │   ModifierKeyFixer        │                  │
│         │  (修饰键修复器)            │                  │
│         │  - 状态管理                │                  │
│         │  - 不一致检测              │                  │
│         │  - 修复触发判断            │                  │
│         │  - 修复执行                │                  │
│         └───────┬───────────────────┘                  │
└─────────────────┼─────────────────────────────────────┘
                  │
        ┌─────────┴─────────┐
        │                   │
┌───────┴────────┐  ┌───────┴────────┐
│  检测层         │  │  检测层         │
│ ┌────────────┐ │  │ ┌────────────┐ │
│ │ Physical   │ │  │ │ Virtual    │ │
│ │ Detector   │ │  │ │ Detector   │ │
│ └─────┬──────┘ │  │ └─────┬──────┘ │
└───────┼────────┘  └───────┼────────┘
        │                   │
┌───────┴────────┐  ┌───────┴────────┐
│ Interception   │  │ Windows API    │
│ Driver         │  │ GetAsyncKey    │
└────────────────┘  └────────────────┘
```

---

## 模块职责

### 1. 检测层（Detection Layer）

#### PhysicalKeyDetector（物理检测器）
**职责：**
- 监听 Interception 驱动的按键事件
- 维护物理按键的实时状态
- 区分左右修饰键

**输入：** InterceptionKeyStroke（按键事件）
**输出：** ModifierKeyStates（物理状态）

**关键方法：**
- `processKeyStroke()` - 处理按键事件
- `isModifierKey()` - 判断是否为修饰键
- `updateModifierState()` - 更新状态

#### VirtualKeyDetector（虚拟检测器）
**职责：**
- 轮询 Windows API 获取虚拟按键状态
- 维护系统层面的按键状态
- 区分左右修饰键

**输入：** 无（主动轮询）
**输出：** VirtualKeyStates（虚拟状态）

**关键方法：**
- `update()` - 更新状态（需定期调用）
- `checkKeyState()` - 检查单个按键状态

---

### 2. 核心逻辑层（Core Logic Layer）

#### ModifierKeyFixer（修复器）
**职责：**
- 整合物理和虚拟检测器
- 检测状态不一致
- 判断是否需要修复
- 执行修复操作
- 管理统计信息

**核心流程：**

```
processEvents()
    ↓
接收 Interception 事件
    ↓
检查是否需要修复？
    ├─ 是 → 执行修复
    │       ├─ 发送释放事件
    │       ├─ 更新统计
    │       └─ 更新虚拟状态
    └─ 否 → 继续
    ↓
更新物理状态
    ↓
转发按键事件
    ↓
更新虚拟状态
    ↓
更新不一致追踪器
```

**关键方法：**
- `processEvents()` - 主处理循环
- `shouldCheckForFix()` - 判断是否触发修复
- `fixStuckKeys()` - 执行修复
- `updateMismatchTrackers()` - 更新不一致追踪

---

### 3. 用户界面层（UI Layer）

#### 控制台界面（main.cpp）
**职责：**
- 实时显示状态
- 接收用户命令（P 暂停，ESC 退出）
- 显示统计信息

**特点：**
- 适合调试和监控
- 实时反馈
- 详细信息显示

#### GUI 界面（main_gui.cpp）
**职责：**
- 系统托盘图标
- 右键菜单
- 气泡通知
- 后台运行

**特点：**
- 静默运行
- 用户友好
- 适合日常使用

---

## 数据流

### 正常按键流程

```
用户按下 Ctrl
    ↓
Interception 捕获事件
    ↓
PhysicalKeyDetector 更新状态 (Ctrl = PRESSED)
    ↓
转发事件到系统
    ↓
VirtualKeyDetector 检测到 (Ctrl = PRESSED)
    ↓
状态一致，无需修复
```

### 卡键修复流程

```
脚本执行 Ctrl+C
    ↓
脚本结束，但虚拟 Ctrl 未释放
    ↓
PhysicalKeyDetector: Ctrl = RELEASED
VirtualKeyDetector: Ctrl = PRESSED
    ↓
MismatchTracker 开始计时
    ↓
持续时间 > 阈值 (1000ms)
    ↓
标记为 STUCK
    ↓
用户按下任意键
    ↓
shouldCheckForFix() 返回 true
    ↓
fixStuckKeys() 发送 Ctrl 释放事件
    ↓
VirtualKeyDetector 更新: Ctrl = RELEASED
    ↓
状态恢复一致
```

---

## 关键算法

### 1. 不一致检测算法

```cpp
void updateMismatchTrackers() {
    for (每个修饰键) {
        if (物理状态 == RELEASED && 虚拟状态 == PRESSED) {
            // 不一致
            if (!tracker.isMismatched) {
                tracker.start();  // 开始计时
            }
        } else {
            // 一致
            tracker.reset();  // 重置
        }
    }
}
```

### 2. 修复触发判断算法

```cpp
bool shouldCheckForFix(stroke) {
    // 条件 1: 必须是按键按下事件
    if (stroke 是释放事件) return false;
    
    // 条件 2: 必须有键卡住
    if (没有键卡住) return false;
    
    // 条件 3: 没有其他物理键按下（避免干扰映射）
    if (有其他物理键按下) return false;
    
    return true;
}
```

### 3. 修复执行算法

```cpp
int fixStuckKeys() {
    int count = 0;
    
    for (每个修饰键) {
        if (tracker.isStuck(阈值)) {
            sendKeyRelease(扫描码, E0标志);
            count++;
            stats.增加计数();
        }
    }
    
    if (count > 0) {
        延迟 20ms;
        更新虚拟状态();
    }
    
    return count;
}
```

---

## 线程模型

### 控制台版本（单线程）

```
主线程:
  ├─ Interception 事件循环
  ├─ 状态更新
  ├─ 界面显示
  └─ 用户输入处理
```

### GUI 版本（双线程）

```
主线程 (UI):
  ├─ 消息循环
  ├─ 托盘图标
  ├─ 菜单处理
  └─ 通知显示

工作线程:
  ├─ Interception 事件循环
  ├─ 状态更新
  ├─ 修复执行
  └─ 通知主线程
```

**线程同步：**
- 工作线程通过 `g_running` 标志控制
- 主线程通过 `PostMessage` 接收通知
- 使用 `WaitForSingleObject` 等待线程结束

---

## 性能考虑

### 1. 事件处理
- Interception 事件驱动，无轮询开销
- 超时设置为 50ms，平衡响应性和 CPU 使用

### 2. 状态更新
- 虚拟检测器每 50ms 轮询一次
- 只在状态变化时更新界面

### 3. 修复延迟
- 修复后延迟 20ms 等待系统处理
- 避免过快的连续修复

### 4. 内存使用
- 所有状态结构都是栈分配
- 无动态内存分配
- 内存占用 < 1MB

---

## 扩展性设计

### 1. 新增检测器
实现相同的接口即可集成：
```cpp
class NewDetector {
    void initialize();
    void update();
    const States& getStates() const;
};
```

### 2. 新增界面
只需实例化 `ModifierKeyFixer` 并调用 `processEvents()`

### 3. 新增修饰键
在状态结构中添加字段，更新检测和修复逻辑

### 4. 自定义修复策略
继承 `ModifierKeyFixer` 并重写 `shouldCheckForFix()`

---

## 安全性考虑

### 1. 权限要求
- 需要管理员权限（Interception 驱动要求）
- 不修改系统文件
- 不访问网络

### 2. 按键转发
- 所有按键都会被转发，不会丢失
- 暂停时也转发按键

### 3. 错误处理
- 初始化失败时友好提示
- 驱动异常时安全退出

---

## 未来优化方向

1. **配置文件支持** - 从文件读取阈值等配置
2. **日志系统** - 记录修复历史
3. **自定义规则** - 用户定义修复条件
4. **性能监控** - 统计 CPU 和内存使用
5. **插件系统** - 支持第三方扩展

---

## 参考资料

- [Interception Driver Documentation](https://github.com/oblitum/Interception)
- [Windows API - GetAsyncKeyState](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getasynckeystate)
- [Shell_NotifyIcon](https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicon)

