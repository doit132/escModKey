add_rules("mode.debug", "mode.release")

-- 设置源文件编码为 UTF-8（解决 C4819 警告）
add_cxflags("/utf-8", {tools = {"cl"}})

-- 设置 C++ 标准为 C++17（toml++ 库需要）
set_languages("c++17")

-- 添加头文件目录
add_includedirs("include")

-- 主程序（控制台版本）
target("escModKey")
    set_kind("binary")
    add_files("src/main.cpp", "src/physical_key_detector.cpp", 
              "src/virtual_key_detector.cpp", "src/modifier_key_fixer.cpp",
              "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        os.cp("lib/interception.dll", target_dir)
        local config_file = path.join(target_dir, "config.toml")
        if not os.isfile(config_file) then
            os.cp("config.toml", target_dir)
        end
    end)

-- GUI 版本（系统托盘）
target("escModKey_gui")
    set_kind("binary")
    set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)")
    add_files("src/main_gui.cpp", "src/physical_key_detector.cpp", 
              "src/virtual_key_detector.cpp", "src/modifier_key_fixer.cpp",
              "src/config.cpp")
    add_files("resources/app.rc")
    add_includedirs("resources")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    add_ldflags("/SUBSYSTEM:WINDOWS", "/ENTRY:WinMainCRTStartup", {force = true})
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        os.cp("lib/interception.dll", target_dir)
        local config_file = path.join(target_dir, "config.toml")
        if not os.isfile(config_file) then
            os.cp("config.toml", target_dir)
        end
    end)

-- 测试：物理按键检测器
target("test_physical_unit")
    set_kind("binary")
    add_files("test/test_physical_unit.cpp", "src/physical_key_detector.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：虚拟按键检测器
target("test_virtual_unit")
    set_kind("binary")
    add_files("test/test_virtual_unit.cpp", "src/virtual_key_detector.cpp")
    add_syslinks("user32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：配置文件按键映射（单元测试）
target("test_config_unit")
    set_kind("binary")
    add_files("test/test_config_unit.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- 测试：配置文件按键映射（属性测试 - 往返）
target("test_config_pbt_roundtrip")
    set_kind("binary")
    add_files("test/test_config_pbt_roundtrip.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- 测试：配置验证（属性测试）
target("test_config_pbt_validation")
    set_kind("binary")
    add_files("test/test_config_pbt_validation.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- 测试：物理按键检测器映射初始化（单元测试）
target("test_physical_unit_mapping")
    set_kind("binary")
    add_files("test/test_physical_unit_mapping.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：物理按键检测器映射（属性测试）
target("test_physical_pbt_mapping")
    set_kind("binary")
    add_files("test/test_physical_pbt_mapping.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：物理按键事件处理（单元测试）
target("test_physical_unit_events")
    set_kind("binary")
    add_files("test/test_physical_unit_events.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：物理按键事件处理（属性测试）
target("test_physical_pbt_events")
    set_kind("binary")
    add_files("test/test_physical_pbt_events.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：集成测试
target("test_integration_unit")
    set_kind("binary")
    add_files("test/test_integration_unit.cpp", "src/config.cpp", "src/physical_key_detector.cpp",
              "src/virtual_key_detector.cpp", "src/modifier_key_fixer.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)
