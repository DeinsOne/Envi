
solution 'Environment'
    architecture 'x86_64'
    startproject 'envi'

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
