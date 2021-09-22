#include "internal/EnviCommon.h"
#include "internal/ThreadManager.h"
#include "linux/x11FrameProcessor.h"

void Envi::RunCaptureWindow(std::shared_ptr<Envi::Thread_Data> data, Window window) {
    Envi::TryCaptureWindow<X11FrameProcessor>(data, window);
}
