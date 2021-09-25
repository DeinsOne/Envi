#include <cstdio>

#include "Envi.h"
#include "EnviUtils.h"


int main(int argc, char** argv) {

    auto windows = Envi::GetWindows();
    auto monitors = Envi::GetMonitors();

    // Show list of all monitors
    printf("\n  Monitors:\n");
    for (auto& i : monitors) { printf("id: %d | name: %s | %dx%d\n", i.Id, i.Name, i.Height, i.Width ); }

    // Show list of all windows
    printf("\n  Windows:\n");
    for (auto& i : windows) { printf("id: %d | name: %s | %dx%d\n", i.Handle, i.Name, i.Size.x, i.Size.y ); }


    // Create main capture configuration
    auto capCfg = Envi::CreateWindowCaptureConfiguration(
        [&]() {
            return Envi::GetWindowsWithNameKeywords( { "visual studio", "code", "envi" } );
        }
    )->SetTickInterval(30)->SetRecoverImages();


    // Set event ob new frame
    capCfg->OnNewFrame([&](const Envi::Image& im, const Envi::Window& wnd) {
        printf("new frame : %d\n", wnd.Handle);
    });


    // Set event on frame changed
    capCfg->OnFrameChanged([&](const Envi::Window& wnd) {
        printf("frame changed : %d %dx%d \n", wnd.Handle, wnd.Size.x, wnd.Size.y );
    });


    // Start capturing
    printf("\n  START CAPTURING\n");
    auto manager = capCfg->startCapturing();


    // Imitate long term process
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    return 0;
}
