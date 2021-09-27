
project 'testcapturing'
    language 'C++'
    kind 'ConsoleApp'
    cppdialect 'C++17'
    staticruntime 'on'

    objdir      (bindir)
    location    (bindir)
    targetdir   (bindir .. '/test')

    includedirs {
        '../envi/include'
    }

    files {
        'capturing.cpp'
    }

    links {
        'envi'
    }

    filter 'system:linux'
        links {
            'pthread',
            'SM',
            'ICE',
            'X11',
            'Xmu',
            'Xinerama',
            'stdc++fs',
            'Xtst',
            'Xfixes',
            'dl'
        }

    filter 'system:windows'
        links {
            'dwmapi.dll'
        }

    filter 'configurations:Debug'
		runtime 'Debug'
		symbols 'on'

	filter 'configurations:Release'
		runtime 'Release'
		optimize 'on'


project 'testinput'
    language 'C++'
    kind 'ConsoleApp'
    cppdialect 'C++17'
    staticruntime 'on'

    objdir      (bindir)
    location    (bindir)
    targetdir   (bindir .. '/test')

    includedirs {
        '../envi/include'
    }

    files {
        'input.cpp'
    }

    links {
        'envi'
    }

    filter 'system:linux'
        links {
            'pthread',
            'SM',
            'ICE',
            'X11',
            'Xmu',
            'Xinerama',
            'stdc++fs',
            'Xtst',
            'Xfixes',
            'dl'
        }

    filter 'system:windows'
        links {
            'dwmapi.dll'
        }

    filter 'configurations:Debug'
		runtime 'Debug'
		symbols 'on'

	filter 'configurations:Release'
		runtime 'Release'
		optimize 'on'

