
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
        'envi'
    }

    filter 'system:linux'
        links {
            'pthread'
        }

	runtime 'Debug'
	symbols 'on'

