#pragma once
#include "internal/EnviCommon.h"
#include <memory>
#include <X11/Xlib.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

namespace Envi {

    class X11FrameProcessor : public BaseFrameProcessor {
        private:
            // Monitor SelectedMonitor;
		    Display* SelectedDisplay = nullptr;
            XID SelectedWindow = 0;
		    XImage* XImage_ = nullptr;
		    std::unique_ptr<XShmSegmentInfo> ShmInfo;

        public:
            X11FrameProcessor() { }
            ~X11FrameProcessor();

            void Pause() {}
            void Resume() {}

            virtual DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow) override;
            DUPL_RETURN ProcessFrame(Window& selectedwindow);

    };

}
