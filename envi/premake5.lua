
project 'envi'
    language 'C++'
    kind 'SharedLib'
    cppdialect 'C++17'
    staticruntime 'on'

    objdir      (bindir)
    location    (bindir)
    targetdir   (bindir .. ('/%{cfg.system}-%{cfg.buildcfg}'):lower() )

    -- TODO:

    filter 'configurations:Debug'
		runtime 'Debug'
		symbols 'on'

	filter 'configurations:Release'
		runtime 'Release'
		optimize 'on'
