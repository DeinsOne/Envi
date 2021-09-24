#include <cstdio>

#include "Envi.h"
#include "EnviUtils.h"


int main(int argc, char** argv) {
    printf("> Capturing test...\n");

    auto windows = Envi::GetWindows();
    auto monitors = Envi::GetMonitors();

    printf("\n  Monitors:\n");
    for (auto& i : monitors) {
        printf("Name: %s | %dx%d\n", i.Name, i.Height, i.Width );
    }

    printf("\n  Windows:\n");
    for (auto& i : windows) {
        printf("Name: %s | %dx%d\n", i.Name, i.Size.x, i.Size.y );
    }


    auto capCfg = Envi::CreateWindowCaptureConfiguration(
        [&]() {
            // Full target name to find is: 'envinteraction : fish/test'
            return Envi::GetWindowsWithNameKeywords( { "visual studio", "code", "envi" } );
        }
    )->SetTickInterval(20)->SetRecoverImages();

    capCfg->OnNewFrame([&](const Envi::Image& im, const Envi::Window& wnd) {
        printf("New frame\n");
    });

    capCfg->OnFrameChanged([&](const Envi::Window& wnd) {
        printf("Frame changed %d : %dx%d \n", wnd.Handle, wnd.Size.x, wnd.Size.y );
    });


    printf("\n  START CAPTURING\n");
    capCfg->startCapturing();


    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    return 0;
}
