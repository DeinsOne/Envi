
project 'test'
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
        'pthread',
        'SM',
        'ICE',
        'X11',
        'Xmu',
        'Xinerama',
        'dl',
        'envi'
    }

	runtime 'Debug'
	symbols 'on'

