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
target("test_physical_detector")
    set_kind("binary")
    add_files("test/test_physical_detector.cpp", "src/physical_key_detector.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：虚拟按键检测器
target("test_virtual_detector")
    set_kind("binary")
    add_files("test/test_virtual_detector.cpp", "src/virtual_key_detector.cpp")
    add_syslinks("user32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：配置文件按键映射
target("test_config")
    set_kind("binary")
    add_files("test/test_config.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- 测试：配置文件按键映射属性
target("test_config_properties")
    set_kind("binary")
    add_files("test/test_config_properties.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- 测试：配置验证属性
target("test_config_validation")
    set_kind("binary")
    add_files("test/test_config_validation.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- 测试：物理按键检测器映射初始化
target("test_physical_mapping")
    set_kind("binary")
    add_files("test/test_physical_mapping.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：物理按键检测器映射属性
target("test_physical_mapping_properties")
    set_kind("binary")
    add_files("test/test_physical_mapping_properties.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：物理按键事件处理
target("test_physical_event_processing")
    set_kind("binary")
    add_files("test/test_physical_event_processing.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：物理按键事件处理属性
target("test_physical_event_properties")
    set_kind("binary")
    add_files("test/test_physical_event_properties.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)

-- 测试：集成测试
target("test_integration")
    set_kind("binary")
    add_files("test/test_integration.cpp", "src/config.cpp", "src/physical_key_detector.cpp",
              "src/virtual_key_detector.cpp", "src/modifier_key_fixer.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        os.cp("lib/interception.dll", path.directory(target:targetfile()))
    end)
