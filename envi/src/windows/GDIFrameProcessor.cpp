#include "windows/GDIFrameProcessor.h"
#include <Dwmapi.h>

#include "CaptureRecover.h"

namespace Envi {

    DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<Thread_Data> data, const Window &selectedwindow) {
        // this is needed to fix AERO BitBlt capturing issues
        ANIMATIONINFO str;
        str.cbSize = sizeof(str);
        str.iMinAnimate = 0;
        SystemParametersInfo(SPI_SETANIMATION, sizeof(str), (void *)&str, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        SelectedWindow = reinterpret_cast<HWND>(selectedwindow.Handle);
        auto Ret = DUPL_RETURN_SUCCESS;
        NewImageBuffer = std::make_unique<unsigned char[]>(ImageBufferSize);
        MonitorDC.DC = GetWindowDC(SelectedWindow);
        CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);

        CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, selectedwindow.Size.x, selectedwindow.Size.y);

        if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
        }

        Data = data;

        if (Data->WindowCaptureData.RecoverImages) {
            Envi::InitRecoverDir(selectedwindow);
        }

        return Ret;
    }

    static bool IsChildWindowToComposite(HWND rootWindow, HWND candidate ) {
        if (rootWindow == candidate)
            return false;

        if (!IsWindowVisible(candidate))
            return false;

        LONG exStyle = GetWindowLong(candidate, GWL_EXSTYLE);
        return 0 != (exStyle & WS_EX_NOREDIRECTIONBITMAP);
    }

    static bool IsTopLevelWindowToComposite(HWND rootWindow, HWND candidate) {
        if (!IsWindowVisible(candidate))
            return false;

        // make sure it's a popup
        LONG style = GetWindowLong(candidate, GWL_STYLE);
        if (0 == (style & WS_POPUP) ) {
            return false;
        }

        // sometimes ownership is described to Windows
        if (GetAncestor(candidate, GA_ROOTOWNER) == rootWindow) {
            return true;
        }

        // for some popups we can use being owned by the same thread as a proxy for
        // ownership
        DWORD topLevelPid = 0;
        DWORD topLevelTid = GetWindowThreadProcessId(rootWindow, &topLevelPid);
        DWORD enumPid = 0;
        DWORD enumTid = GetWindowThreadProcessId(candidate, &enumPid);

        if (enumTid != 0 && enumTid == topLevelTid && enumPid == topLevelPid) {
            return true;
        }

        return false;
    }

    static std::vector<HWND> CollectWindowsToComposite(HWND hRootWindow) {
         DWORD topLevelPid = 0;
        DWORD topLevelTid = GetWindowThreadProcessId(hRootWindow, &topLevelPid);

        std::vector<HWND> compositeWindows;

        // find all top level popup windows that belong to this window and capture those
        auto fnTopLevelCallback = [&hRootWindow, &compositeWindows, topLevelPid, topLevelTid](HWND hwnd, LPARAM unused) {
            (void)unused;

            // EnumWindows calls the callback with windows in top-down order,
            // so once we reach our target window we've been called with all its
            // children already
            if (hwnd == hRootWindow)
                return FALSE;

            if ( IsTopLevelWindowToComposite( hRootWindow, hwnd )) {
                compositeWindows.push_back(hwnd);
            }

            return TRUE;
        };
        EnumWindows([]( HWND hwnd, LPARAM callbackParam) { return (*static_cast<decltype(fnTopLevelCallback) *>((void *)callbackParam))(hwnd, 0); },
                    (LPARAM)&fnTopLevelCallback);

        // find all child popup windows that need compositing
        auto fnChildCallback = [&hRootWindow, &compositeWindows, topLevelPid, topLevelTid](HWND hwnd, LPARAM unused) {
            (void)unused;

            if (IsChildWindowToComposite(hRootWindow, hwnd)) {
                compositeWindows.push_back(hwnd);
            }

            return TRUE;
        };
         EnumChildWindows(
             hRootWindow, [](HWND hwnd, LPARAM callbackParam) { return (*static_cast<decltype(fnChildCallback) *>((void *)callbackParam))(hwnd, 0); },
            (LPARAM)&fnChildCallback);

         return compositeWindows;
    }

    DUPL_RETURN GDIFrameProcessor::ProcessFrame(Window &selectedwindow) {
        auto Ret = DUPL_RETURN_SUCCESS;
        auto windowrect = Envi::GetWindowRect(SelectedWindow);
        ImageRect ret;
        memset(&ret, 0, sizeof(ret));
        ret.bottom = windowrect.ClientRect.bottom;
        ret.left = windowrect.ClientRect.left;
        ret.right = windowrect.ClientRect.right;
        ret.top = windowrect.ClientRect.top;
        selectedwindow.Position.x = windowrect.ClientRect.left;
        selectedwindow.Position.y = windowrect.ClientRect.top;

        if (!IsWindow(SelectedWindow) || selectedwindow.Size.x != Width(ret) || selectedwindow.Size.y != Height(ret)) {
            // Call OnFrameChanged with old window and image
            ProcessFrameChanged(Data->WindowCaptureData, *this, selectedwindow );

            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; // window size changed. This will rebuild everything
        }

        // Selecting an object into the specified DC
        auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);
        auto left = -windowrect.ClientBorder.left;
        auto top = -windowrect.ClientBorder.top;

        BOOL result = PrintWindow((HWND)selectedwindow.Handle, CaptureDC.DC, PW_RENDERFULLCONTENT );

        if ( !result ) {
            result = BitBlt(CaptureDC.DC, left, top, ret.right, ret.bottom, MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT);
        }

        if ( !result ) {
            // if the screen cannot be captured, return
            SelectObject(CaptureDC.DC, originalBmp);
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; // likely a permission issue
        }

        //std::vector<HWND> childrenToComposite = CollectWindowsToComposite((HWND)selectedwindow.Handle);
        //
        //// list is ordered topmost to bottommost, so we visit them in reverse order to let painter's algorithm work
        //for ( auto child = childrenToComposite.rbegin(); child != childrenToComposite.rend(); child++ ) {
        //    auto childRect = SL::Screen_Capture::GetWindowRect( *child );

        //    HDC srcDC = GetWindowDC(*child);

        //    // if this fails we just won't composite this window, so continue with the others to get what we can
        //    BOOL childBlitSuccess = BitBlt(CaptureDC.DC, childRect.ClientRect.left - windowrect.ClientRect.left, childRect.ClientRect.top - windowrect.ClientRect.top,
        //           childRect.ClientRect.right - childRect.ClientRect.left, childRect.ClientRect.bottom - childRect.ClientRect.top, 
        //           srcDC, 0, 0,
        //           SRCCOPY | CAPTUREBLT);
        //    if ( !childBlitSuccess ) {
        //        DWORD err = GetLastError();
        //    }

        //    ReleaseDC(*child, srcDC);
        //}

        BITMAPINFOHEADER bi;
        memset(&bi, 0, sizeof(bi)); 
        bi.biSize = sizeof(BITMAPINFOHEADER); 
        bi.biWidth = Width(ret);
        bi.biHeight = -Height(ret);
        bi.biPlanes = 1;
        bi.biBitCount = sizeof(ImageBGRA) * 8; // always 32 bits damnit!!!
        bi.biCompression = BI_RGB;
        bi.biSizeImage = ((Width(ret) * bi.biBitCount + 31) / (sizeof(ImageBGRA) * 8)) * sizeof(ImageBGRA)  * Height(ret);
        GetDIBits(MonitorDC.DC, CaptureBMP.Bitmap, 0, (UINT)Height(ret), NewImageBuffer.get(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
        SelectObject(CaptureDC.DC, originalBmp);

        if (Data->WindowCaptureData.RecoverImages) {
            RecoverImage(selectedwindow);
            UpdateRecoverDir(&Recovered, selectedwindow);
        }

        ProcessCapture(Data->WindowCaptureData, *this, selectedwindow, NewImageBuffer.get(), Width(selectedwindow)* sizeof(ImageBGRA));

        return Ret;
    }

    void GDIFrameProcessor::RecoverImage(Envi::Window& wnd) {
        auto save = [&]() {
            RecoverThreads++;
            std::chrono::duration<double> now = std::chrono::high_resolution_clock::now().time_since_epoch();

            auto size = Width(wnd) * Height(wnd) * sizeof(Envi::ImageBGRA);
            auto imgbuffer1(std::make_unique<unsigned char[]>(size));
            // CapturingMutex.lock();
            memcpy(imgbuffer1.get(), NewImageBuffer.get(), Width(wnd) * Height(wnd) * size );
            // CapturingMutex.unlock();

            std::chrono::duration<double> ini = Data->WindowCaptureData.TimeStarted.time_since_epoch();
            std::chrono::seconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - ini).count();

            std::string fnm = std::to_string(milliseconds) + ".jpg";
            std::string dir = ENVI_RECOVER_DIR + std::to_string(wnd.Handle);

            ImageRect rect;
            rect.left = 0;
            rect.top = 0;
            rect.bottom = Height(wnd);
            rect.right = Width(wnd);

            auto img = CreateImage(rect, Width(wnd) * sizeof(Envi::ImageBGRA), reinterpret_cast<ImageBGRA *>(imgbuffer1.get()) );
            auto imgbuffer(std::make_unique<unsigned char[]>(size));
            ExtractAndConvertToRGBA(img, imgbuffer1.get(), size);
            tje_encode_to_file((dir+"/"+fnm).c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());

            Recovered.push_back(fnm);
            RecoverThreads--;
        };

        std::thread sv(save);
        if (RecoverThreads > ENVI_RECOVER_THREADS_LIMIT) {
            sv.join();
        }
        else {
            sv.detach();
        }        
    }

}
