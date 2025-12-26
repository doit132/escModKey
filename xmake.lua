add_rules("mode.debug", "mode.release")

-- Set C++ standard to C++17 (required by toml++)
set_languages("c++17")

-- Add include directory
add_includedirs("include")

-- Main program (Console version)
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
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
        -- Copy config file if it doesn't exist
        local config_file = path.join(target_dir, "config.toml")
        if not os.isfile(config_file) then
            os.cp("config.toml", target_dir)
        end
    end)

-- GUI version (System tray)
target("escModKey_gui")
    set_kind("binary")
    set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)")
    add_files("src/main_gui.cpp", "src/physical_key_detector.cpp", 
              "src/virtual_key_detector.cpp", "src/modifier_key_fixer.cpp",
              "src/config.cpp")
    -- Add resource file
    add_files("resources/app.rc")
    add_includedirs("resources")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    -- Set as Windows GUI application (no console)
    add_ldflags("/SUBSYSTEM:WINDOWS", "/ENTRY:WinMainCRTStartup", {force = true})
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
        -- Copy config file if it doesn't exist
        local config_file = path.join(target_dir, "config.toml")
        if not os.isfile(config_file) then
            os.cp("config.toml", target_dir)
        end
    end)

-- Test: Physical Key Detector
target("test_physical_detector")
    set_kind("binary")
    add_files("test/test_physical_detector.cpp", "src/physical_key_detector.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)

-- Test: Virtual Key Detector
target("test_virtual_detector")
    set_kind("binary")
    add_files("test/test_virtual_detector.cpp", "src/virtual_key_detector.cpp")
    add_syslinks("user32")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)

-- Test: Config Key Mappings
target("test_config")
    set_kind("binary")
    add_files("test/test_config.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- Test: Config Key Mappings Properties
target("test_config_properties")
    set_kind("binary")
    add_files("test/test_config_properties.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- Test: Config Validation Properties
target("test_config_validation")
    set_kind("binary")
    add_files("test/test_config_validation.cpp", "src/config.cpp")
    add_syslinks("user32", "shell32")

-- Test: Physical Key Detector Mapping Initialization
target("test_physical_mapping")
    set_kind("binary")
    add_files("test/test_physical_mapping.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)

-- Test: Physical Key Detector Mapping Properties
target("test_physical_mapping_properties")
    set_kind("binary")
    add_files("test/test_physical_mapping_properties.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)

-- Test: Physical Key Event Processing
target("test_physical_event_processing")
    set_kind("binary")
    add_files("test/test_physical_event_processing.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)

-- Test: Physical Key Event Processing Properties
target("test_physical_event_properties")
    set_kind("binary")
    add_files("test/test_physical_event_properties.cpp", "src/physical_key_detector.cpp", "src/config.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)

-- Test: Integration Tests
target("test_integration")
    set_kind("binary")
    add_files("test/test_integration.cpp", "src/config.cpp", "src/physical_key_detector.cpp",
              "src/virtual_key_detector.cpp", "src/modifier_key_fixer.cpp")
    add_linkdirs("lib")
    add_links("interception")
    add_syslinks("user32", "shell32")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)
