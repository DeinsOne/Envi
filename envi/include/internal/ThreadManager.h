#pragma once
#include "Envi.h"
#include "APCommon.h"
#include <thread>

namespace Envi {

    class ThreadManager {
        std::vector<std::thread> _handlers;

        std::shared_ptr<std::atomic_bool> _terminateThreadsEvent;

        public:
            ThreadManager() { }

            void Init(std::shared_ptr<Thread_Data> data);

            void Join();

    };

    template<class T, class F> bool TryCaptureWindow(const F &data, Window &wnd) {
        T frameprocessor;
        frameprocessor.ImageBufferSize = wnd.Size.x * wnd.Size.y * sizeof(ImageBGRA);

        if (data->WindowCaptureData.OnFrameChanged) { // only need the old buffer if difs are needed. If no dif is needed, then the
                                                      // image is always new
            frameprocessor.ImageBuffer = std::make_unique<unsigned char[]>(frameprocessor.ImageBufferSize);
        }

        auto ret = frameprocessor.Init(data, wnd);
        if (ret != DUPL_RETURN_SUCCESS) {
            return false;
        }

        while (!data->CommonData_.terminateThreadsEvent) {
            // get a copy of the shared_ptr in a safe way
            // auto timer = std::atomic_load(&data->WindowCaptureData.FrameTimer);
            // timer->start();
            ret = frameprocessor.ProcessFrame(wnd);

            if (ret != DUPL_RETURN_SUCCESS) {
                if (ret == DUPL_RETURN_ERROR_EXPECTED) {
                    // The system is in a transition state so request the duplication be restarted
                    data->CommonData_.expectedErrorEvent = true;
                    // std::cout << "Exiting Thread due to expected error " << std::endl;
                    printf("Exiting Thread due to expected error\n");
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }
                else {
                    // Unexpected error so exit the application
                    data->CommonData_.unexpectedErrorEvent = true;
                    // std::cout << "Exiting Thread due to Unexpected error " << std::endl;
                }
                return true;
            }
            // timer->wait();
            while (data->CommonData_.paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }
        return true;
    }

    void RunCaptureWindow(std::shared_ptr<Thread_Data> data, Window window);

}
