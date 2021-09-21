#include "Envi.h"
#include "internal/APCommon.h"
#include "internal/ThreadManager.h"

void Envi::ThreadManager::Init(std::shared_ptr<Thread_Data> data) {
    if (!_handlers.empty()) {
        _handlers.clear();
    }

    auto things = data->WindowCaptureData.getThingsToWatch();
    _handlers.resize(things.size() );

    // // Fill threads for every selected window
    for (int i = 0; i < things.size(); i++ ) {
        try {
            _handlers[i] = std::thread(&Envi::RunCaptureWindow, data, things[i] );
        }
        catch (...) {
            
        }
    }

}

void Envi::ThreadManager::Join() {
    for (int i = 0; i < _handlers.size(); i++) {
        _handlers[i].join();
        // _handlers[i].detach();
    }
}
