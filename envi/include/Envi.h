#pragma once
#include "EnviCore.h"

#include <assert.h>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

/**
 * Environment interaction
*/
namespace Envi {

    struct Image;
    struct ENVI_EXTERN ImageBGRA {
        unsigned char B, G, R, A;
    };

    struct ENVI_EXTERN Point {
        int x;
        int y;
    };

    struct ENVI_EXTERN Window {
        size_t Handle;
        Point Position;

        Point Size;
        // Name will always be lower case. It is converted to lower case internally by the library for comparisons
        char Name[ENVI_PROPERTY_MAX_LENGTH] = {0};
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
        char Name[ENVI_PROPERTY_MAX_LENGTH] = {0};
        float Scaling = 1.0f;
    };

    ENVI_EXTERN std::vector<Window> GetWindows();
    ENVI_EXTERN std::vector<Monitor> GetMonitors();

    ENVI_EXTERN int Height(const Image &img);
    ENVI_EXTERN int Width(const Image &img);
    ENVI_EXTERN const ImageBGRA* StartSrc(const Image &img);
    ENVI_EXTERN const ImageBGRA* GotoNextRow(const Image &img, const ImageBGRA *current);

    // Callbecks
    typedef std::function<void(const Envi::Image &img, const Window &window)> WindowCaptureCallback;
    typedef std::function<void(const Window &window)> WindowChangeCallback;
    typedef std::function<std::vector<Window>()> WindowCallback;

    class ICapturerManager {
        public:
            // Control flow
            virtual void Pause() = 0;
            virtual bool IsPaused() = 0;
            virtual void Resume() = 0;

    };

    template <typename T, typename C>
    class ICaptureConfiguration {
        public:
            virtual std::shared_ptr<ICaptureConfiguration<T, C>> OnNewFrame(const T &cb) = 0;

            virtual std::shared_ptr<ICaptureConfiguration<T, C>> OnFrameChanged(const C &cb) = 0;

            virtual std::shared_ptr<ICaptureConfiguration<T, C>> SetTickInterval(int milliseconds) = 0;

            virtual std::shared_ptr<ICaptureConfiguration<T, C>> SetRecoverImages(bool recover = true) = 0;

            virtual std::shared_ptr<ICapturerManager> startCapturing() = 0;
    };

    ENVI_EXTERN std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback, WindowChangeCallback>> CreateWindowCaptureConfiguration(const WindowCallback &windowstocapture);

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
                if (now < Deadline)
                {
                    std::this_thread::sleep_for(Deadline - now);
                }
            }
            std::chrono::microseconds duration() const { return Duration; }
    };


    struct ENVI_EXTERN KeyEvent {
        bool Pressed;
        Envi::KeyCodes Key;
        KeyEvent(const bool pressed, const Envi::KeyCodes key) : Pressed(pressed), Key(key) {}
    };

    struct ENVI_EXTERN MouseButtonEvent {
        bool Pressed;
        Envi::MouseButtons Button;
        MouseButtonEvent(const bool pressed, const Envi::MouseButtons button) : Pressed(pressed), Button(button) {}
    };

    struct ENVI_EXTERN MouseScrollEvent {
        int Offset;
        MouseScrollEvent(const int offset) : Offset(offset) {}
    };

    struct ENVI_EXTERN MousePositionOffsetEvent {
        int X = 0;
        int Y = 0;
        MousePositionOffsetEvent(const int x, const int y) : X(x), Y(y) {}
        MousePositionOffsetEvent(const Envi::Point e) : X(e.x), Y(e.y) {}
    };

    struct ENVI_EXTERN MousePositionAbsoluteEvent {
        int X = 0;
        int Y = 0;
        MousePositionAbsoluteEvent(const int x, const int y) : X(x), Y(y) {}
        MousePositionAbsoluteEvent(const Envi::Point e) : X(e.x), Y(e.y) {}
    };

    template <typename Event, typename Duration>
    ENVI_EXTERN void SendInput(const Event &e, const Duration& time);

    typedef std::function<void(const KeyEvent& cb)> KeyCallback;
    typedef std::function<void(const MouseButtonEvent& cb)> MouseButtonCallback;
    typedef std::function<void(const MouseScrollEvent& cb)> MouseScrollCallback;
    typedef std::function<void(const MousePositionOffsetEvent& cb)> MousePositionOffsetCallback;
    typedef std::function<void(const MousePositionAbsoluteEvent& cb)> MousePositionAbsoluteCallback;

    class ENVI_EXTERN IInputManager {
        private:
            using Ms = std::chrono::milliseconds;

        public:
            virtual void PushEvent(const KeyEvent &e, const Ms time = Ms(0)) = 0;
            virtual void PushEvent(const MouseButtonEvent &e, const Ms time = Ms(0)) = 0;
            virtual void PushEvent(const MouseScrollEvent &pos, const Ms time = Ms(0)) = 0;
            virtual void PushEvent(const MousePositionOffsetEvent &pos, const Ms time = Ms(0)) = 0;
            virtual void PushEvent(const MousePositionAbsoluteEvent &pos, const Ms time = Ms(0)) = 0;

            virtual void Pause() = 0;
            virtual bool IsPaused() = 0;
            virtual void Resume() = 0;

            virtual void Flush() = 0;
            virtual void Wait() = 0;

    };

    class ENVI_EXTERN IInputConfiguration {
        public:
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const KeyCallback& cb) = 0;
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const MouseButtonCallback& cb) = 0;
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const MouseScrollCallback& cb) = 0;
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const MousePositionOffsetCallback& cb) = 0;
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const MousePositionAbsoluteCallback& cb) = 0;

            virtual std::shared_ptr<IInputManager> startListening() = 0;
    };

    ENVI_EXTERN std::shared_ptr<IInputConfiguration> CreateInputConfiguration();

};
