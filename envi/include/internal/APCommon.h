#pragma once
#include "Envi.h"
#include <atomic>
#include <thread>

namespace AP {

    namespace Envi {
        ENVI_EXTERN int Index(const Monitor &mointor);
        // unique identifier
        ENVI_EXTERN int Id(const Monitor &mointor);
        ENVI_EXTERN int Adapter(const Monitor &mointor);
        ENVI_EXTERN int OffsetX(const Monitor &mointor);
        ENVI_EXTERN int OffsetY(const Monitor &mointor);
        ENVI_EXTERN void OffsetX(Monitor &mointor, int x);
        ENVI_EXTERN void OffsetY(Monitor &mointor, int y);
        ENVI_EXTERN int OffsetX(const Window &mointor);
        ENVI_EXTERN int OffsetY(const Window &mointor);
        ENVI_EXTERN void OffsetX(Window &mointor, int x);
        ENVI_EXTERN void OffsetY(Window &mointor, int y);
        ENVI_EXTERN const char *Name(const Monitor &mointor);
        ENVI_EXTERN const char *Name(const Window &mointor);
        ENVI_EXTERN int Height(const Monitor &mointor);
        ENVI_EXTERN int Width(const Monitor &mointor);
        ENVI_EXTERN void Height(Monitor &mointor, int h);
        ENVI_EXTERN void Width(Monitor &mointor, int w);
        ENVI_EXTERN int Height(const Window &mointor);
        ENVI_EXTERN int Width(const Window &mointor);
        ENVI_EXTERN void Height(Window &mointor, int h);
        ENVI_EXTERN void Width(Window &mointor, int w);
        ENVI_EXTERN int X(const Point &p);
        ENVI_EXTERN int Y(const Point &p);

        enum DUPL_RETURN { DUPL_RETURN_SUCCESS = 0, DUPL_RETURN_ERROR_EXPECTED = 1, DUPL_RETURN_ERROR_UNEXPECTED = 2 };
        Monitor CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string &n, float scale);
        Monitor CreateMonitor(int index, int id, int adapter, int h, int w, int ox, int oy, const std::string &n, float scale); 


        template <typename F, typename W> struct CaptureData {
            std::shared_ptr<Timer> FrameTimer;
            F OnNewFrame;
            F OnFrameChanged;
            W getThingsToWatch;
        };

        struct CommonData {
            // Used to indicate abnormal error condition
            std::atomic<bool> UnexpectedErrorEvent;
            // Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate
            // the duplication interface
            std::atomic<bool> ExpectedErrorEvent;
            // Used to signal to threads to exit
            std::atomic<bool> TerminateThreadsEvent;
            std::atomic<bool> Paused;
        };

        struct Thread_Data {
            CaptureData<WindowCaptureCallback, WindowCallback> WindowCaptureData;
            CommonData CommonData_;
        };

        class BaseFrameProcessor {
            public:
                std::shared_ptr<Thread_Data> Data;
                std::unique_ptr<unsigned char[]> ImageBuffer;
                int ImageBufferSize = 0;
                bool FirstRun = true;
        };

    };


};
