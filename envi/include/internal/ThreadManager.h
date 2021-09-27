#pragma once
#include "Envi.h"
#include "EnviCommon.h"
#include <thread>

namespace Envi {

    class ThreadManager {
        std::vector<std::thread> _handlers;

        public:
            ThreadManager() { }

            void Init(std::shared_ptr<Thread_Data> data);

            void Join();

    };

    template<class T, class F> bool TryCaptureWindow(const F &data, Window &wnd) {
        T frameprocessor;

        auto ret = frameprocessor.Init(data, wnd);
        if (ret != DUPL_RETURN_SUCCESS) {
            return false;
        }

        while (!data->CommonData_.terminateThreadsEvent) {
            Envi::Timer tm(std::chrono::milliseconds(data->WindowCaptureData.Interval));

            ret = frameprocessor.ProcessFrame(wnd);

            if (ret != DUPL_RETURN_SUCCESS) {
                if (ret == DUPL_RETURN_ERROR_EXPECTED) {
                    // The system is in a transition state so request the duplication be restarted
                    data->CommonData_.expectedErrorEvent = true;
                    break;
                }
                else {
                    data->CommonData_.unexpectedErrorEvent = true;
                    data->CommonData_.terminateThreadsEvent = true;
                }
                return true;
            }

            while (data->CommonData_.paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(ENVI_INTERAPTION_MS));
            }

            tm.wait();
        }
        return true;
    }

    void RunCaptureWindow(std::shared_ptr<Thread_Data> data, Window window);

}
