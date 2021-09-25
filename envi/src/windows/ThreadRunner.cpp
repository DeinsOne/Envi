#include "windows/GDIFrameProcessor.h"
#include "Envi.h"
#include "internal/ThreadManager.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <memory>
#include <string>


namespace Envi {

    template <class T> void ProcessExit(DUPL_RETURN Ret, T *TData) {
        if (Ret != DUPL_RETURN_SUCCESS) {
            if (Ret == DUPL_RETURN_ERROR_EXPECTED) {
                // The system is in a transition state so request the duplication be restarted
                TData->CommonData_.expectedErrorEvent = true;
            }
            else {
                // Unexpected error so exit the application
                TData->CommonData_.unexpectedErrorEvent = true;
            }
        }
    }

    template <class T> bool SwitchToInputDesktop(const std::shared_ptr<T> data) {
        HDESK CurrentDesktop = nullptr;
        CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
        if (!CurrentDesktop) {
            // We do not have access to the desktop so request a retry
            data->CommonData_.expectedErrorEvent = true;
            ProcessExit(DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED, data.get());
            return false;
        }

        // Attach desktop to this thread
        bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
        CloseDesktop(CurrentDesktop);
        CurrentDesktop = nullptr;
        if (!DesktopAttached) {
            // We do not have access to the desktop so request a retry
            data->CommonData_.expectedErrorEvent = true;
            ProcessExit(DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED, data.get());
            return false;
        }
        return true;
    }


    void RunCaptureWindow(std::shared_ptr<Thread_Data> data, Window wnd) {
        // need to switch to the input desktop for capturing...
        if (!SwitchToInputDesktop(data))
            return;
        TryCaptureWindow<GDIFrameProcessor>(data, wnd);
    }
}
