
#include "Envi.h"
#include "internal/APCommon.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>


namespace AP {

    namespace Envi {

        int clamp(int value, int lowest, int highest ) {
            if (value < lowest) return lowest;
            else if (value > highest) return highest;
            return value;
        }

        int GetMonitors(Monitor *monitors, int monitors_size) {
            auto local_monitors = Envi::GetMonitors();
            auto maxelements = clamp(static_cast<int>(local_monitors.size()), 0, monitors_size);
            memcpy(monitors, local_monitors.data(), maxelements * sizeof(Monitor));
            return static_cast<int>(local_monitors.size());
        }

        int GetWindows(Window *windows, int monitors_size) {
            auto local_windows = Envi::GetWindows();
            auto maxelements = clamp(static_cast<int>(local_windows.size()), 0, monitors_size);
            memcpy(windows, local_windows.data(), maxelements * sizeof(Window));
            return static_cast<int>(local_windows.size());
        }

        template <class MONITORTPE> bool isMonitorInsideBounds(MONITORTPE monitors, const Monitor &monitor) {
            auto totalwidth = 0;
            for (auto &m : monitors) {
                totalwidth += Width(m);
            }
            // if the monitor doesnt exist any more!
            if (std::find_if(begin(monitors), end(monitors), [&](auto &m) { return m.Id == monitor.Id; }) == end(monitors)) {
                return false;
            } // if the area to capture is outside the dimensions of the desktop!!
            auto &realmonitor = monitors[Index(monitor)];
            if (Height(realmonitor) < Height(monitor) ||          // monitor height check
                totalwidth < Width(monitor) + OffsetX(monitor) || // total width check
                Width(monitor) > Width(realmonitor))              // regular width check

            {
                return false;
            } // if the entire screen is capture and the offsets changed, get out and rebuild
            else if (Height(realmonitor) == Height(monitor) && Width(realmonitor) == Width(monitor) &&
                     (OffsetX(realmonitor) != OffsetX(monitor) || OffsetY(realmonitor) != OffsetY(monitor))) {
                return false;
            }
            return true;
        }

        bool isMonitorInsideBounds(const std::vector<Monitor> &monitors, const Monitor &monitor) {
            return isMonitorInsideBounds(monitors, monitor);
        }
        namespace C_API {
            bool isMonitorInsideBounds(const Monitor *monitors, const int monitorsize, const Monitor *monitor) {
                return isMonitorInsideBounds(std::vector<Monitor>(monitors, monitors + monitorsize), *monitor);
            }
        }; // namespace C_API



    };
};