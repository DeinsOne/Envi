
solution 'Environment'
    architecture 'x86_64'

    configurations {
        'Debug',
        'Release'
    }

    bindir = '%{wks.location}/bin'

    newaction {
        trigger     = 'test',
        description = 'Test envi lib',
        execute     = function ()
            os.execute('./bin/test/test')
        end
    }

    include 'envi'
    include 'testi'

    filter { 'system:linux' }
        postbuildcommands {
            '{MKDIR} %{bindir}/install/lib',
            '{MKDIR} %{bindir}/install/bin',
            '{MKDIR} %{bindir}/install',

            '{COPYDIR} %{wks.location}/envi/include %{bindir}/install',
            '{RMDIR} %{bindir}/install/include/linux %{bindir}/install/include/windows',

            '{COPYFILE} %{bindir}/%{cfg.system}-%{cfg.buildcfg}/libenvi.so %{bindir}/install/bin',
            '{COPYFILE} %{bindir}/%{cfg.system}-%{cfg.buildcfg}/libenvi.a %{bindir}/install/lib'
        }

    filter { 'system:windows' }
        postbuildcommands {
        }

