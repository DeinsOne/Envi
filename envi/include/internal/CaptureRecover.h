#pragma once
#include "Envi.h"

#if defined(WINDOWS) || defined(WIN32)
  #include <filesystem>
  namespace fs = std::filesystem;
#else
  #include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#endif

namespace Envi {

    /**
     * @b Removes dir's content
     * @param dir directors to be emptied 
    */
    void RemoveDir(const fs::path& dir) {
        for (auto& path: fs::directory_iterator(dir)) {
            fs::remove_all(path);
        }
    }

    /**
     * @b Prepares recover directory for specified window: |ENVI_RECOVER_DIR/wnd.Handle|
     * @param wnd window instances of which will be recovering
    */
    inline void InitRecoverDir(const Window& wnd) {
        std::string RecoverDir = ENVI_RECOVER_DIR + std::to_string(wnd.Handle);

        if (fs::is_directory(ENVI_RECOVER_DIR)) {
            if (fs::is_directory(RecoverDir)) {
                RemoveDir(RecoverDir);
            }

            else {
                fs::create_directory(RecoverDir);
            }
        }
        else {
            fs::create_directory(ENVI_RECOVER_DIR);
            fs::create_directory(RecoverDir);
        }

    }

    /**
     * @b Chech whether |ENVI_RECOVER_DIR/wnd.Handle|
     * @param instances filenames already saved in recover directory
     * @param wnd window instances of which was recovered
    */
    inline void UpdateRecoverDir(std::vector<std::string>* instances, const Window& wnd) {
        if (instances->size() > ENVI_RECOVER_INSTANCES_LIMIT) {
            auto over = instances->size() - ENVI_RECOVER_INSTANCES_LIMIT;
            for (int i = 0; i < over; i++ ) {
                fs::remove(ENVI_RECOVER_DIR + std::to_string(wnd.Handle) + "/" + instances->at(0));
                instances->erase(instances->begin());
            }

            instances->shrink_to_fit();
        }
    }

}
