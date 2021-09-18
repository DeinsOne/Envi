#pragma once

#if defined(WINDOWS) || defined(WIN32)
    #if defined(ENVI_DLL)
        #define ENVI_C_EXTERN extern "C" __declspec(dllexport)
        #define ENVI_EXTERN __declspec(dllexport)
    #else
        #define ENVI_C_EXTERN
        #define ENVI_EXTERN
    #endif
#else
    #if defined(ENVI_DLL)
        #define ENVI_C_EXTERN extern "C"
        #define ENVI_EXTERN
    #else
        #define ENVI_C_EXTERN
        #define ENVI_EXTERN
    #endif
#endif

#define ENVI_PROPERTY_MAX_LENGTH 512

#include <assert.h>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>


// General purpose namespace to save in and mentain hierarchy
namespace AP {

    /**
     * Environment interaction
    */
    namespace Envi {

        struct Image;

        struct ENVI_EXTERN Point {
            int x;
            int y;
        };

        struct ENVI_EXTERN MousePoint {
            Point Position;
            Point HotSpot;
        };

        struct ENVI_EXTERN Window {
            size_t Handle;
            Point Position;

            Point Size;
            // Name will always be lower case. It is converted to lower case internally by the library for comparisons
            char Name[128] = {0};
        };

        struct ENVI_EXTERN Monitor {
            int Id = INT32_MAX;
            int Index = INT32_MAX;
            int Adapter = INT32_MAX;
            int Height = 0;
            int Width = 0;
            int OriginalHeight = 0;
            int OriginalWidth = 0;
            // Offsets are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
            // monitors around so this affects their offset.
            int OffsetX = 0;
            int OffsetY = 0;
            int OriginalOffsetX = 0;
            int OriginalOffsetY = 0;
            char Name[128] = {0};
            float Scaling = 1.0f;
        };

        ENVI_EXTERN std::vector<Window>     GetWindows();
        ENVI_EXTERN std::vector<Monitor>    GetMonitors();
        namespace C_API {
            // GetWindows and GetMonitors expect a pre allocated buffer with the size as the second input parameter.
            // The output of these functions is the actual total number of elements that the library had to return. So, applications should use this value
            // in determining how to preallocate data.
            ENVI_EXTERN int GetWindows(Window *windows, int windows_size);
            ENVI_EXTERN int GetMonitors(Monitor *monitors, int monitors_size);
            ENVI_EXTERN bool isMonitorInsideBounds(const Monitor *monitors, const int monitorsize, const Monitor *monitor);
        }; // namespace C_API
        ENVI_EXTERN bool isMonitorInsideBounds(const std::vector<Monitor> &monitors, const Monitor &monitor);


        // Callbecks
        typedef std::function<void(const AP::Envi::Image &img, const Window &window)> WindowCaptureCallback;
        typedef std::function<std::vector<Window>()> WindowCallback;

        class ICapturerManager {
            public:
                // Control flow
                virtual void pause() = 0;
                virtual void isPaused() = 0;
                virtual void resume() = 0;
                virtual void start() = 0;

                /**
                 * @return Returns COPY of current image. You have to manually clean image data after using
                */
                virtual Image* GetImage() = 0;
        };

        template <typename CALLBACK>
        class ICaptureConfiguration {
            public:
                virtual std::shared_ptr<ICaptureConfiguration<CALLBACK>> onNewFrame(const CALLBACK& cb) = 0;

                virtual std::shared_ptr<ICaptureConfiguration<CALLBACK>> onFrameChanged(const CALLBACK& cb) = 0;

                virtual std::shared_ptr<ICapturerManager> startCapturing() = 0;
        };


        ENVI_EXTERN std::shared_ptr<ICaptureConfiguration<WindowCallback>> CreateCaptureConfiguration(const WindowCallback& windowstocapture);


        class Timer {
            private:
                using Clock =
                    std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

                std::chrono::microseconds Duration;
                Clock::time_point Deadline;

            public:
                template <typename Rep, typename Period>
                Timer(const std::chrono::duration<Rep, Period> &duration)
                    : Duration(std::chrono::duration_cast<std::chrono::microseconds>(duration)), Deadline(Clock::now() + Duration)
                {
                }
                void start() { Deadline = Clock::now() + Duration; }
                void wait() {
                    const auto now = Clock::now();
                    if (now < Deadline) {
                        std::this_thread::sleep_for(Deadline - now);
                    }
                }
                std::chrono::microseconds duration() const { return Duration; }
        };

    };

};

