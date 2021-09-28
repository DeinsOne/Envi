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

    /**
     * @return Returns list of opened windows
    */
    ENVI_EXTERN std::vector<Window> GetWindows();

    /**
     * @return Returns list of connected monitors
    */
    ENVI_EXTERN std::vector<Monitor> GetMonitors();

    ENVI_EXTERN int Height(const Image &img);
    ENVI_EXTERN int Width(const Image &img);
    ENVI_EXTERN const ImageBGRA* StartSrc(const Image &img);
    ENVI_EXTERN const ImageBGRA* GotoNextRow(const Image &img, const ImageBGRA *current);

    // WindowCaptureCallback
    typedef std::function<void(const Envi::Image &img, const Window &window)> WindowCaptureCallback;
    // WindowChangeCallback
    typedef std::function<void(const Window &window)> WindowChangeCallback;
    // WindowCallback is used to define window to be captured
    typedef std::function<std::vector<Window>()> WindowCallback;

    class ICapturerManager {
        public:
            /**
             * Stop window capturing
            */
            virtual void Pause() = 0;

            /**
             * @return Check if capturing is paused
            */
            virtual bool IsPaused() = 0;

            /**
             * Resume capturing if it was stopped
            */
            virtual void Resume() = 0;

    };

    template <typename T, typename C>
    class ICaptureConfiguration {
        public:
            /**
             * Set up callbeck for every tick
             * @param cb [WindowCaptureCallback] callback
            */
            virtual std::shared_ptr<ICaptureConfiguration<T, C>> OnNewFrame(const T &cb) = 0;

            /**
             * Set up callbeck for window resizing
             * @param cb [WindowChangeCallback] callback
            */
            virtual std::shared_ptr<ICaptureConfiguration<T, C>> OnFrameChanged(const C &cb) = 0;

            /**
             * Set up tick time. Analogous of FPS
             * @param milliseconds time restriction for every tick
            */
            virtual std::shared_ptr<ICaptureConfiguration<T, C>> SetTickInterval(int milliseconds) = 0;

            /**
             * Set up image recovering mode. If is enabled every frame from tick will be saved ion disk
             * @param recover Conditional variable
            */
            virtual std::shared_ptr<ICaptureConfiguration<T, C>> SetRecoverImages(bool recover = true) = 0;

            /**
             * Factory to start window capturing and to get contorl flow interface
            */
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

    // KeyCallback
    typedef std::function<void(const KeyEvent& cb)> KeyCallback;
    // MouseButtonCallback
    typedef std::function<void(const MouseButtonEvent& cb)> MouseButtonCallback;
    // MouseScrollCallback
    typedef std::function<void(const MouseScrollEvent& cb)> MouseScrollCallback;
    // MousePositionOffsetCallback
    typedef std::function<void(const MousePositionOffsetEvent& cb)> MousePositionOffsetCallback;
    // MousePositionAbsoluteCallback
    typedef std::function<void(const MousePositionAbsoluteEvent& cb)> MousePositionAbsoluteCallback;

    class ENVI_EXTERN IInputManager {
        private:
            using Ms = std::chrono::milliseconds;

        public:
            /**
             * Add KeyEvent to queue
             * @param e Event
             * @param time event duration
            */
            virtual void PushEvent(const KeyEvent &e, const Ms time = Ms(0)) = 0;

            /**
             * Add MouseButtonEvent to queue
             * @param e Event
             * @param time event duration
            */
            virtual void PushEvent(const MouseButtonEvent &e, const Ms time = Ms(0)) = 0;

            /**
             * Add MouseScrollEvent to queue
             * @param e Event
             * @param time event duration
            */
            virtual void PushEvent(const MouseScrollEvent &pos, const Ms time = Ms(0)) = 0;

            /**
             * Add MousePositionOffsetEvent to queue
             * @param e Event
             * @param time event duration
            */
            virtual void PushEvent(const MousePositionOffsetEvent &pos, const Ms time = Ms(0)) = 0;

            /**
             * Add MousePositionAbsoluteEvent to queue
             * @param e Event
             * @param time event duration
            */
            virtual void PushEvent(const MousePositionAbsoluteEvent &pos, const Ms time = Ms(0)) = 0;

            /**
             * Stop executing event queue
            */
            virtual void Pause() = 0;

            /**
             * Check if event queue is stopped
            */
            virtual bool IsPaused() = 0;

            /**
             * Continue executing event queue
            */
            virtual void Resume() = 0;

            /**
             * Clear event queue
            */
            virtual void Flush() = 0;

            /**
             * Wait until event queue became empty
            */
            virtual void Wait() = 0;

    };

    class ENVI_EXTERN IInputConfiguration {
        public:
            /**
             * Add a callback to be called every time a KeyEvent occurs
            */
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const KeyCallback& cb) = 0;

            /**
             * Add a callback to be called every time a MouseButtonCallback occurs
            */
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const MouseButtonCallback& cb) = 0;

            /**
             * Add a callback to be called every time a MouseScrollCallback occurs
            */
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const MouseScrollCallback& cb) = 0;

            /**
             * Add a callback to be called every time a MousePositionOffsetCallback occurs
            */
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const MousePositionOffsetCallback& cb) = 0;

            /**
             * Add a callback to be called every time a MousePositionAbsoluteCallback occurs
            */
            virtual std::shared_ptr<IInputConfiguration> OnEvent(const MousePositionAbsoluteCallback& cb) = 0;

            /**
             * Start executing event queue
             * @return Input manager to allow flow control
            */
            virtual std::shared_ptr<IInputManager> startListening() = 0;
    };

    ENVI_EXTERN std::shared_ptr<IInputConfiguration> CreateInputConfiguration();

};
