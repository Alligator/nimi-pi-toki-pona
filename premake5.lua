workspace "nimi-pi-toki-pona"
    location "build"
    configurations { "Debug", "Release" }
    targetdir "bin/%{cfg.buildcfg}"
    startproject "parson"

project "nimi"
    kind "WindowedApp"
    language "C++"

    files {
      "*.h", "*.cpp",
      "resources/*.h",
      "impl/*.cpp",
      "impl/*.h",
      "lib/imgui/**.h",
      "lib/imgui/**.cpp",
      "lib/parson/**.h",
      "lib/parson/**.c"
    }
    defines { "GLEW_STATIC" }

    includedirs { "lib/imgui", "lib/parson", "lib/glew" }
    sysincludedirs { "lib/sdl/SDL" }

    libdirs { "lib/glew", "lib/sdl/lib/Win32" }
    links { "SDL2", "SDL2main", "opengl32", "glew32s" }

    postbuildcommands {
      '{COPY} "../dictionary.json" "../bin/%{cfg.buildcfg}"'
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        debugdir "bin/Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        debugdir "bin/Release"
