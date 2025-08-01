workspace "ClickSounds"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    
    -- Force forward slashes
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.platform}/%{cfg.buildcfg}"
    
project "ClickSounds"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    
    files {
        "src/**.h",
        "src/**.cpp",
        "third_party/nlohmann/json.hpp",
        "third_party/miniaudio/miniaudio.h",
        "third_party/miniaudio/ma_reverb_node.h",
        "third_party/miniaudio/ma_reverb_node.c",
        "third_party/miniaudio/verblib.h",
        "third_party/ThomasMonkman/FileWatch.hpp"
    }
    
    includedirs {
        "src",
        "third_party"
    }
    
    postbuildcommands {
        "{COPY} %{wks.location}/config.json %{cfg.targetdir}",
        "{COPY} %{wks.location}/sounds %{cfg.targetdir}/sounds"
    }
    
    filter "system:windows"
        defines { "PLATFORM_WINDOWS" }
    
    filter "system:linux"
        links { "pthread", "m", "dl" }
        defines { "PLATFORM_LINUX" }
        
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        
    filter "action:gmake*"
        toolset "clang"

