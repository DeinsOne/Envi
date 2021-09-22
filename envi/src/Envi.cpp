
#include "Envi.h"
#include "internal/EnviCommon.h"
#include "internal/ThreadManager.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>

namespace Envi {

    int clamp(int value, int lowest, int highest) {
        if (value < lowest)
            return lowest;
        else if (value > highest)
            return highest;
        return value;
    }

    int GetMonitors(Monitor *monitors, int monitors_size) {
        auto local_monitors = Envi::GetMonitors();
        auto maxelements = clamp(static_cast<int>(local_monitors.size()), 0, monitors_size);
        memcpy(monitors, local_monitors.data(), maxelements * sizeof(Monitor));
        return static_cast<int>(local_monitors.size());
    }

    int GetWindows(Window *windows, int monitors_size) {
        auto local_windows = Envi::GetWindows();
        auto maxelements = clamp(static_cast<int>(local_windows.size()), 0, monitors_size);
        memcpy(windows, local_windows.data(), maxelements * sizeof(Window));
        return static_cast<int>(local_windows.size());
    }

    static bool ScreenCaptureManagerExists = false;
    class WindowCaptureManager : public ICapturerManager {
        public:
            std::shared_ptr<Thread_Data> _threadData;

            std::thread _thread;

            WindowCaptureManager() {
                // There must be the only one WindowCaptureManager instance
                assert(!ScreenCaptureManagerExists);
                ScreenCaptureManagerExists = true;
                _threadData = std::make_shared<Thread_Data>();
                _threadData->CommonData_.paused = false;
            }

            virtual ~WindowCaptureManager() {
                _threadData->CommonData_.terminateThreadsEvent = true; // set the exit flag for the threads
                _threadData->CommonData_.paused = false;               // unpaused the threads to let everything exit

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            void start() {
                _thread = std::thread([&]() {
                    ThreadManager manager;

                    manager.Init(_threadData);

                    while (!_threadData->CommonData_.terminateThreadsEvent) {

                        // Window doesn't exist any more
                        if (_threadData->CommonData_.unexpectedErrorEvent) {
                            _threadData->CommonData_.terminateThreadsEvent = true;
                            continue;
                        }

                        // Reinit manager
                        if (_threadData->CommonData_.expectedErrorEvent) {
                            _threadData->CommonData_.expectedErrorEvent = false;
                            manager.Init(_threadData);
                            continue;
                        }

                        // Pause detection
                        if (_threadData->CommonData_.paused) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(30));
                            continue;
                        }

                        // Process callbacks
                        manager.Join();
                    }

                });

                _threadData->CommonData_.terminateThreadsEvent = false;
                _threadData->CommonData_.unexpectedErrorEvent = false;
                _threadData->CommonData_.paused = false;
                _thread.detach();
            }

            virtual void Pause() override { _threadData->CommonData_.paused = true; }

            virtual bool IsPaused() override { return _threadData->CommonData_.paused; }

            virtual void Resume() override { _threadData->CommonData_.paused = false; }

    };

    class WindowCaptureConfiguration : public ICaptureConfiguration<WindowCaptureCallback, WindowChangeCallback> {
        private:
            std::shared_ptr<WindowCaptureManager> _impl;

        public:
            WindowCaptureConfiguration(const std::shared_ptr<WindowCaptureManager> &impl) : _impl(impl) {}

            virtual std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback, WindowChangeCallback>> OnNewFrame(const WindowCaptureCallback &cb) override {
                _impl->_threadData->WindowCaptureData.OnNewFrame = cb;
                return std::make_shared<WindowCaptureConfiguration>(_impl);
            }

            virtual std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback, WindowChangeCallback>> OnFrameChanged(const WindowChangeCallback &cb) override {
                _impl->_threadData->WindowCaptureData.OnFrameChanged = cb;
                return std::make_shared<WindowCaptureConfiguration>(_impl);
            }

            virtual std::shared_ptr<ICapturerManager> startCapturing() override {
                _impl->start();
                return _impl;
            }
    };


    std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback, WindowChangeCallback>> CreateWindowCaptureConfiguration(const WindowCallback &windowstocapture) {
        auto _impl = std::make_shared<WindowCaptureManager>();
        _impl->_threadData->WindowCaptureData.GetThingsToWatch = windowstocapture;
        return std::make_shared<WindowCaptureConfiguration>(_impl);
    }

};
