#pragma once
#include "Envi.h"
#include <mutex>

#include <queue>

namespace Envi {

    template <typename T>
    void clearQueue(std::queue<T> &q ) {
       std::queue<T> empty;
       std::swap(q, empty );
    }
    
    static bool InputManagerExists = false;
    class ENVI_EXTERN InputManager final : public IInputManager {
        private:
            std::thread _thread;
            std::mutex _mutex;

            std::queue<std::function<void()>> _queue;

            bool _paused = false;
            bool _terminateManager = false;

            using Ms = std::chrono::milliseconds;

        public:
            InputManager() {
                assert(!InputManagerExists);
                InputManagerExists = true;
            }
            ~InputManager() { _terminateManager = true; }

            KeyCallback keyCallback;
            MouseButtonCallback mouseButtonCallback;
            MouseScrollCallback mouseScrollCallback;
            MousePositionOffsetCallback mousePositionOffsetCallback;
            MousePositionAbsoluteCallback mousePositionAbsoluteCallback;

            virtual void PushEvent(const KeyEvent &e, const Ms time) override {
                _mutex.lock();
                _queue.push([&, e, time]() {
                    Envi::SendInput(e, time);
                    if (keyCallback) {
                        keyCallback(e);
                    }
                });
                _mutex.unlock();
            }
            virtual void PushEvent(const MouseButtonEvent &e, const Ms time) override {
                _mutex.lock();
                _queue.push([&, e, time]() {
                    Envi::SendInput(e, time); 
                    if (mouseButtonCallback) {
                        mouseButtonCallback(e);
                    }
                });
                _mutex.unlock();
            }
            virtual void PushEvent(const MouseScrollEvent &pos, const Ms time) override {
                _mutex.lock();
                _queue.push([&, pos, time]() {
                    Envi::SendInput(pos, time); 
                    if (mouseScrollCallback) {
                        mouseScrollCallback(pos);
                    }
                });
                _mutex.unlock();
            }
            virtual void PushEvent(const MousePositionOffsetEvent &pos, const Ms time) override {
                _mutex.lock();
                _queue.push([&, pos, time]() { 
                    Envi::SendInput(pos, time);
                    if (mousePositionOffsetCallback) {
                        mousePositionOffsetCallback(pos);
                    };
                });
                _mutex.unlock();
            }
            virtual void PushEvent(const MousePositionAbsoluteEvent &pos, const Ms time) override {
                _mutex.lock();
                _queue.push([&, pos, time]() {
                    Envi::SendInput(pos, time);
                    if (mousePositionAbsoluteCallback) {
                        mousePositionAbsoluteCallback(pos);
                    }
                });
                _mutex.unlock();
            }

            virtual void Pause() override { _paused = true; };
            virtual bool IsPaused() override { return _paused; };
            virtual void Resume() override { _paused = false; };


            virtual void Flush() override {
                _mutex.lock();
                clearQueue(_queue);
                _mutex.unlock();
            }
            virtual void Wait() override {
                while (_queue.size()) std::this_thread::sleep_for(std::chrono::milliseconds(ENVI_INTERAPTION_MS));
            }

            void start() {
                _thread = std::thread([&]() {
                    while (!_terminateManager) {
                        while (_paused) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(ENVI_INTERAPTION_MS));
                        }

                        if (_queue.size()) {
                            _mutex.lock();
                            auto a = _queue.front();
                            _queue.pop();
                            _mutex.unlock();

                            a();
                        }
                    }
                });

                _thread.detach();
            }

    };

}