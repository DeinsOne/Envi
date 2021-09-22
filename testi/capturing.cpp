#include <cstdio>

#include "Envi.h"


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
            auto windows = Envi::GetWindows();
            decltype(windows) ret;

            for (auto i : windows) {
                if (std::string(i.Name).find("envinteraction :") != std::string::npos ) {
                    ret.push_back(i);
                }
                else if (std::string(i.Name).find("firefox") != std::string::npos) {
                    ret.push_back(i);
                }
            }

            return ret;
        }
    )->OnNewFrame([&](const Envi::Image& im, const Envi::Window& wnd) {
        printf("New frame %d : %dx%d\n", wnd.Handle, wnd.Size.x, wnd.Size.y );
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });

    capCfg->OnFrameChanged([&](const Envi::Window& wnd) {
        printf("Frame changed %d : %dx%d \n", wnd.Handle, wnd.Size.x, wnd.Size.y );
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });

    printf("\n  START CAPTURING\n");


    capCfg->startCapturing();


    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    return 0;
}
