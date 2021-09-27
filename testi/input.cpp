#include <cstdio>
#include "Envi.h"

int main(int argc, char** argv) {

    auto inputCfg = Envi::CreateInputConfiguration()->OnEvent([](const Envi::KeyEvent& cb) {
        printf("Key pressed : %d | %d\n", cb.Pressed, cb.Key);
    })->OnEvent([](const Envi::MousePositionOffsetEvent& cb) {
        printf("Moved offset\n");
    });

    auto inputManager = inputCfg->startListening();

    // inputManager->Pause();
    inputManager->PushEvent(Envi::MouseButtonEvent{true, Envi::MouseButtons::LEFT} );
    inputManager->PushEvent(Envi::MousePositionOffsetEvent(100, 20), std::chrono::milliseconds(100) );
    inputManager->PushEvent(Envi::MouseButtonEvent{false, Envi::MouseButtons::LEFT} );
    // inputManager->Resume();

    // inputManager->Flush();
    inputManager->Wait();

    return 0;
}
