#pragma once
#include "Envi.h"
#include "EnviCommon.h"
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

        auto ret = frameprocessor.Init(data, wnd);
        if (ret != DUPL_RETURN_SUCCESS) {
            return false;
        }

        while (!data->CommonData_.terminateThreadsEvent) {
            ret = frameprocessor.ProcessFrame(wnd);

            if (ret != DUPL_RETURN_SUCCESS) {
                if (ret == DUPL_RETURN_ERROR_EXPECTED) {
                    // The system is in a transition state so request the duplication be restarted
                    data->CommonData_.expectedErrorEvent = true;
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }
                else {
                    data->CommonData_.unexpectedErrorEvent = true;
                    data->CommonData_.terminateThreadsEvent = true;
                }
                return true;
            }
        }
        return true;
    }

    void RunCaptureWindow(std::shared_ptr<Thread_Data> data, Window window);

}
