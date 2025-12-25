# 配置文件说明

## 配置文件位置

程序会按以下顺序查找配置文件：

1. **程序目录**：`config.toml`（与 exe 文件同目录）
2. **用户目录**：`%APPDATA%\ModifierKeyAutoFix\config.toml`

推荐将配置文件放在程序目录，便于便携使用。

## 配置文件格式

配置文件使用 TOML 格式，支持注释。如果配置文件不存在，程序会自动创建默认配置。

## 配置项说明

### [general] - 常规设置

#### thresholdMs
- **类型**：整数
- **默认值**：1000
- **说明**：判定按键卡住的时间阈值（毫秒）
- **建议**：
  - 如果经常误判（不需要时也修复），增加到 1500-2000
  - 如果希望更快修复，减少到 500-800
  - 不建议低于 500，可能导致正常使用时误触发

#### showMessages
- **类型**：布尔值（true/false）
- **默认值**：true
- **说明**：是否显示控制台消息（仅控制台版本有效）
- **用途**：调试时设为 true，正常使用可设为 false

### [notifications] - 通知设置（GUI 版本）

#### enabled
- **类型**：布尔值（true/false）
- **默认值**：true
- **说明**：是否启用气泡通知
- **用途**：如果觉得通知太多，可以设为 false

#### notifyOnFix
- **类型**：布尔值（true/false）
- **默认值**：true
- **说明**：修复按键时是否显示通知
- **用途**：想要静默修复时设为 false

#### notifyOnStartup
- **类型**：布尔值（true/false）
- **默认值**：true
- **说明**：程序启动时是否显示通知
- **用途**：不想看到启动提示时设为 false

### [advanced] - 高级设置

#### tooltipUpdateInterval
- **类型**：整数
- **默认值**：1000
- **说明**：托盘提示更新间隔（毫秒）
- **用途**：控制托盘图标提示信息的更新频率
- **注意**：暂未实现，保留用于未来功能

#### debugMode
- **类型**：布尔值（true/false）
- **默认值**：false
- **说明**：启用调试模式
- **用途**：开发和调试时使用
- **注意**：暂未实现，保留用于未来功能

## 配置文件示例

### 默认配置
```toml
# Modifier Key Auto-Fix Configuration
# 修饰键自动修复工具配置文件

[general]
# Time threshold (ms) before considering a key as stuck
# 判定按键卡住的时间阈值（毫秒）
thresholdMs = 1000

# Show console messages (console version only)
# 是否显示控制台消息（仅控制台版本）
showMessages = true

[notifications]
# Show balloon notifications (GUI version)
# 是否显示气泡通知（GUI 版本）
enabled = true

# Notify when a key is fixed
# 修复按键时是否通知
notifyOnFix = true

# Notify on startup
# 启动时是否通知
notifyOnStartup = true

[advanced]
# Update tray tooltip interval (ms)
# 托盘提示更新间隔（毫秒）
tooltipUpdateInterval = 1000

# Enable debug logging
# 启用调试日志
debugMode = false
```

### 静默模式配置
适合不想看到任何通知的用户：

```toml
[general]
thresholdMs = 1000
showMessages = false

[notifications]
enabled = false
notifyOnFix = false
notifyOnStartup = false

[advanced]
tooltipUpdateInterval = 1000
debugMode = false
```

### 快速修复模式
适合希望更快修复的用户：

```toml
[general]
thresholdMs = 500
showMessages = true

[notifications]
enabled = true
notifyOnFix = true
notifyOnStartup = true

[advanced]
tooltipUpdateInterval = 1000
debugMode = false
```

### 保守模式
适合担心误判的用户：

```toml
[general]
thresholdMs = 2000
showMessages = true

[notifications]
enabled = true
notifyOnFix = true
notifyOnStartup = false

[advanced]
tooltipUpdateInterval = 1000
debugMode = false
```

## 修改配置

### 方法 1：手动编辑
1. 找到配置文件（程序目录或用户目录）
2. 使用文本编辑器打开 `config.toml`
3. 修改相应的配置项
4. 保存文件
5. 重启程序使配置生效

### 方法 2：删除重建
1. 删除现有的 `config.toml`
2. 重启程序
3. 程序会自动创建默认配置文件
4. 再次编辑新创建的文件

## 配置加载顺序

1. 程序启动时自动加载配置
2. 如果配置文件不存在，使用默认值并创建配置文件
3. 如果配置文件格式错误，使用默认值（不会覆盖错误文件）
4. 如果某个配置项缺失，使用该项的默认值

## 故障排除

### 配置不生效
- 确认修改的是正确的配置文件（检查程序目录和用户目录）
- 确认 TOML 格式正确（注意等号、引号、大小写）
- 重启程序使配置生效

### 配置文件格式错误
如果配置文件格式错误，程序会：
1. 在控制台版本显示错误信息
2. 使用默认配置继续运行
3. 不会覆盖错误的配置文件

解决方法：
- 检查 TOML 语法（等号两边有空格，布尔值小写）
- 或删除配置文件让程序重新创建

### 找不到配置文件
程序会自动在以下位置创建：
- 优先：程序目录（如果有写入权限）
- 其次：`%APPDATA%\ModifierKeyAutoFix\`

## 注意事项

1. **TOML 格式要求**：
   - 布尔值必须小写：`true` / `false`
   - 等号两边建议有空格：`thresholdMs = 1000`
   - 注释使用 `#` 开头
   - 字符串不需要引号（除非包含特殊字符）

2. **配置生效时机**：
   - 配置在程序启动时加载
   - 修改配置后需要重启程序
   - 未来可能添加热加载功能

3. **权限问题**：
   - 如果程序目录无写入权限，配置会保存到用户目录
   - 建议将程序放在有写入权限的目录

## 未来计划

- [ ] 热加载配置（无需重启）
- [ ] GUI 配置界面
- [ ] 更多可配置项（禁用特定按键监控等）
- [ ] 配置文件验证和错误提示
