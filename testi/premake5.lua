
project 'test'
    language 'C++'
    kind 'ConsoleApp'
    cppdialect 'C++17'
    staticruntime 'on'

    objdir      (bindir)
    location    (bindir)
    targetdir   (bindir .. '/test')

    files {
        'capturing.cpp'
    }

	runtime 'Debug'
	symbols 'on'

