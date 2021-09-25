#pragma once
#include "Envi.h"
#include "internal/EnviCommon.h"
#include "GDIHelpers.h"
#include <memory>
#include <mutex>

#include <d3d11.h>
#include <dxgi1_2.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

namespace Envi {

    class GDIFrameProcessor : public BaseFrameProcessor {
        private:
            HDCWrapper MonitorDC;
            HDCWrapper CaptureDC;
            HBITMAPWrapper CaptureBMP;
            HWND SelectedWindow;

            std::shared_ptr<Thread_Data> Data;

            unsigned char* ImageData = nullptr;

            std::vector<std::string> Recovered;
            int RecoverThreads = 0;
            std::mutex CapturingMutex;
            void RecoverImage(Envi::Window& wnd);

        public:
            ~GDIFrameProcessor();

            void Pause() {}
            void Resume() {}
    
            virtual DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow) override;
            DUPL_RETURN ProcessFrame(Window& selectedwindow);
    };
}