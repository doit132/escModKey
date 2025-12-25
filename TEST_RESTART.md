# 重启功能测试说明

## 功能描述
GUI 版本现在支持通过托盘图标右键菜单重启程序，重新加载配置文件。

## 测试步骤

1. **启动 GUI 版本**
   ```
   .\build\windows\x64\release\escModKey_gui.exe
   ```

2. **修改配置文件**
   - 打开 `.\build\windows\x64\release\config.toml`
   - 修改任意配置项，例如：
     - 禁用 Win 键监控：`monitorWin = false`
     - 禁用特定按键：`disabledKeys = ["rwin"]`
     - 添加自定义按键：`customKeys = [[0x3A, false, "CapsLock", 0x14]]`

3. **重启程序**
   - 右键点击托盘图标（任务栏右下角）
   - 选择 "Restart (Reload Config)"
   - 等待通知显示 "Configuration reloaded successfully"

4. **验证配置生效**
   - 双击托盘图标查看统计信息
   - 确认监控的按键列表已更新

## 预期结果

- 重启成功：显示 "Restarted - Configuration reloaded successfully" 通知
- 重启失败：显示 "Restart Failed" 通知（配置文件格式错误时）
- 配置立即生效，无需退出程序

## 注意事项

- 重启会清除当前的统计数据
- 如果配置文件有语法错误，重启会失败，程序会保持原有配置继续运行
- 重启过程中会短暂中断按键监控（通常小于 100ms）
