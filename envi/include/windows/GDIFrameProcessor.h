#pragma once
#include "Envi.h"
#include "internal/EnviCommon.h"
#include "GDIHelpers.h"
#include <memory>

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
            // Monitor SelectedMonitor;
            HWND SelectedWindow;
            std::unique_ptr<unsigned char[]> NewImageBuffer;

            std::shared_ptr<Thread_Data> Data;

            std::unique_ptr<unsigned char[]> ImageBuffer;
            int ImageBufferSize = 0;

            std::vector<std::string> Recovered;
            uint RecoverThreads = 0;
            // std::mutex CapturingMutex;
            void RecoverImage(Envi::Window& wnd);

        public: 
            void Pause() {}
            void Resume() {}
    
            // DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Monitor& monitor);
            // DUPL_RETURN ProcessFrame(const Monitor& currentmonitorinfo);
            virtual DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow) override;
            DUPL_RETURN ProcessFrame(Window& selectedwindow);
    };
}