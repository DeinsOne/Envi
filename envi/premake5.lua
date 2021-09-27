
project 'envi'
    language 'C++'
    kind 'SharedLib'
    cppdialect 'C++17'
    staticruntime 'on'

    objdir      (bindir)
    location    (bindir)
    targetdir   (bindir .. ('/%{cfg.system}-%{cfg.buildcfg}'):lower() )

    includedirs {
        'include'
    }

    files {
        'src/Envi.cpp',
        'src/EnviCommon.cpp',
        'src/ThreadManager.cpp'
    }

    filter 'system:linux'
        files {
            'src/linux/GetWindows.cpp',
            'src/linux/GetMonitors.cpp',
            'src/linux/x11FrameProcessor.cpp',
            'src/linux/ThreadRunner.cpp',
            'src/linux/x11Input.cpp'
        }

        links {
            'SM',
            'ICE',
            'X11',
            'Xmu',
            'Xinerama',
            'jpeg',
            'stdc++fs',
            'Xtst',
            'Xfixes',
            'dl'
        }

    filter 'system:windows'
        files {
            'src/windows/GetWindows.cpp',
            'src/windows/GetMonitors.cpp',
            'src/windows/GDIFrameProcessor.cpp',
            'src/windows/ThreadRunner.cpp',
            'src/windows/GDIInput.cpp'
        }

        links {
            'dwmapi.lib'
        }

        defines {
            '_CRT_SECURE_NO_WARNINGS'
        }

    filter 'configurations:Debug'
		runtime 'Debug'
		symbols 'on'

	filter 'configurations:Release'
		runtime 'Release'
		optimize 'on'

        defines {
            'ENVI_DLL'
        }
