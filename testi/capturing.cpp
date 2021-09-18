#include <cstdio>

#include "Envi.h"

using namespace AP;

int main(int argc, char** argv) {
    printf("> Capturing test...\n");

    auto windows = Envi::GetWindows();
    auto monitors = Envi::GetMonitors();

    printf("\nMonitors:\n");
    for (auto& i : monitors) {
        printf("Name: %s | %dx%d\n", i.Name, i.Height, i.Width );
    }

    printf("\n\Windows:\n");
    for (auto& i : windows) {
        printf("Name: %s | %dx%d\n", i.Name, i.Size.x, i.Size.y );
    }


    return 0;
}
