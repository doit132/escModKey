add_rules("mode.debug", "mode.release")

-- Add include directory
add_includedirs("include")

-- Main program
target("escModKey")
    set_kind("binary")
    add_files("src/main.cpp", "src/physical_key_detector.cpp")
    add_linkdirs("lib")
    add_links("interception")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)

-- Test: Physical Key Detector
target("test_physical_detector")
    set_kind("binary")
    add_files("test/test_physical_detector.cpp", "src/physical_key_detector.cpp")
    add_linkdirs("lib")
    add_links("interception")
    after_build(function (target)
        local target_dir = path.directory(target:targetfile())
        local dll_file = path.join("lib", "interception.dll")
        os.cp(dll_file, target_dir)
    end)
