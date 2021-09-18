
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
        'src/APCommon.cpp'
    }

    filter 'system:linux'
        files {
            'src/linux/GetWindows.cpp',
            'src/linux/GetMonitors.cpp'
        }

        links {
            'SM',
            'ICE',
            'X11',
            'Xmu',
            'Xinerama',
            'dl'
        }

    filter 'configurations:Debug'
		runtime 'Debug'
		symbols 'on'

	filter 'configurations:Release'
		runtime 'Release'
		optimize 'on'
