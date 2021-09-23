#pragma once
#include "Envi.h"
#include <algorithm>

namespace Envi {

    /**
     * Retrun a list of windows whose name contains |title| 
    */
    inline std::vector<Envi::Window> GetWindowsWithNameContains(const std::string title) {
        std::vector<Envi::Window> ret;
        auto windows = Envi::GetWindows();

        for (const auto& i : windows) {
            if (std::string(i.Name).find(title) != std::string::npos) {
                ret.push_back(i);
            }
        }

        return ret;
    }


    /**
     * Returns a list of windows whose name contains the most number of keywords
    */
    inline std::vector<Envi::Window> GetWindowsWithNameKeywords(std::vector<std::string> keywords) {
        auto windows = Envi::GetWindows();
        std::vector<int> mathces;

        mathces.resize(windows.size());

        for (int i = 0; i < windows.size(); i++) {
            int mtchs = 0;

            for (int j = 0; j < keywords.size(); j++) {
                if (std::string(windows[i].Name).find(keywords[j]) != std::string::npos) { mtchs++; }
            }

            mathces[i] = mtchs;
        }

        int maxMatches = *std::max_element(mathces.begin(), mathces.end());
        std::vector<Envi::Window> ret;

        for (int i = 0; i < mathces.size(); i++) {
            if (mathces[i] == maxMatches) ret.push_back(windows[i]);
        }

        return ret;
    }

}
