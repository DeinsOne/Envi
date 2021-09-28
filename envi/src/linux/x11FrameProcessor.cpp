#include "linux/x11FrameProcessor.h"
#include "internal/EnviCommon.h"
#include "internal/CaptureRecover.h"
#include <X11/Xutil.h> 

#include "internal/CaptureRecover.h"
#include "EnviUtils.h"

#define TJE_IMPLEMENTATION
#include "internal/tiny_jpeg.h"

Envi::X11FrameProcessor::~X11FrameProcessor() {
    while (RecoverThreads > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ENVI_INTERAPTION_MS));
    }

    if(ShmInfo) {
        shmdt(ShmInfo->shmaddr);
        shmctl(ShmInfo->shmid, IPC_RMID, 0);
        XShmDetach(SelectedDisplay, ShmInfo.get());
    }
    if(XImage_) {
        XDestroyImage(XImage_);
    }
    if(SelectedDisplay) {
        XCloseDisplay(SelectedDisplay);
    }
}

Envi::DUPL_RETURN Envi::X11FrameProcessor::Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow) {
    auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
    Data = data;
    SelectedDisplay = XOpenDisplay(NULL);
    SelectedWindow = selectedwindow.Handle;

    if(!SelectedDisplay) {
        return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
    }
    int scr = XDefaultScreen(SelectedDisplay);

    ShmInfo = std::make_unique<XShmSegmentInfo>();

    CapturingMutex.lock();
    XImage_ = XShmCreateImage(SelectedDisplay,
                            DefaultVisual(SelectedDisplay, scr),
                            DefaultDepth(SelectedDisplay, scr),
                            ZPixmap,
                            NULL,
                            ShmInfo.get(),
                            selectedwindow.Size.x,
                            selectedwindow.Size.y);
    ShmInfo->shmid = shmget(IPC_PRIVATE, XImage_->bytes_per_line * XImage_->height, IPC_CREAT | 0777);

    ShmInfo->readOnly = False;
    ShmInfo->shmaddr = XImage_->data = (char*)shmat(ShmInfo->shmid, 0, 0);
    CapturingMutex.unlock();

    XShmAttach(SelectedDisplay, ShmInfo.get());

    if (Data->WindowCaptureData.RecoverImages) {
        while (RecoverThreads > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        Envi::InitRecoverDir(selectedwindow);
    }

    return ret;
}

Envi::DUPL_RETURN Envi::X11FrameProcessor::ProcessFrame(Window& selectedwindow){       
    auto Ret = DUPL_RETURN_SUCCESS; 
    XWindowAttributes wndattr;

    if(XGetWindowAttributes(SelectedDisplay, SelectedWindow, &wndattr) == 0){
        return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; //window might not be valid any more
    }

    if(wndattr.width != Width(selectedwindow) || wndattr.height != Height(selectedwindow)) {
        // Call OnFrameChanged with old window and image
        if (Data->WindowCaptureData.OnFrameChanged) {
            ProcessFrameChanged(Data->WindowCaptureData, *this, selectedwindow );
        }

        return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; //window size changed. This will rebuild everything
    }

    if (wndattr.map_state != IsViewable) {
        return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; //window was collapsed
    }


    CapturingMutex.lock();
    if(!XShmGetImage(SelectedDisplay,
                     selectedwindow.Handle,
                     XImage_,
                     0,
                     0,
                     AllPlanes)) {
        return DUPL_RETURN_ERROR_EXPECTED;
    }
    CapturingMutex.unlock();

    if (Data->WindowCaptureData.RecoverImages) {
        RecoverImage(selectedwindow);
        UpdateRecoverDir(&Recovered, selectedwindow);
    }

    ProcessCapture(Data->WindowCaptureData, *this, selectedwindow, (unsigned char*)XImage_->data, XImage_->bytes_per_line);
    return Ret;
}

void Envi::X11FrameProcessor::RecoverImage(Envi::Window& wnd) {
    // ENVI_RECOVER_THREADS_LIMIT
    auto save = [&]() {
        RecoverThreads++;
        std::chrono::duration<double> now = std::chrono::high_resolution_clock::now().time_since_epoch();

        XImage imageBackup;
        CapturingMutex.lock();
        imageBackup = *XImage_;
        imageBackup.data = (char*)malloc(XImage_->width * XImage_->height * (XImage_->bitmap_unit / 8) );
        memcpy(imageBackup.data, XImage_->data, XImage_->width * XImage_->height * (XImage_->bitmap_unit / 8) );
        CapturingMutex.unlock();

        std::chrono::duration<double> ini = Data->WindowCaptureData.TimeStarted.time_since_epoch();
        std::chrono::seconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - ini).count();

        std::string fnm = std::to_string(milliseconds) + ".jpg";
        std::string dir = ENVI_RECOVER_DIR + std::to_string(wnd.Handle);

        ImageRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.bottom = Height(wnd);
        rect.right = Width(wnd);

        auto img = CreateImage(rect, imageBackup.bytes_per_line, reinterpret_cast<ImageBGRA *>(imageBackup.data) );
        auto size = XImage_->width * XImage_->height * sizeof(Envi::ImageBGRA);
        auto imgbuffer(std::make_unique<unsigned char[]>(size));
        ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
        tje_encode_to_file((dir+"/"+fnm).c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());

        free((void*)imageBackup.data);
        Recovered.push_back(fnm);
        RecoverThreads--;
    };

    // process_original(XImage_);
    std::thread sv(save);
    if (RecoverThreads > ENVI_RECOVER_THREADS_LIMIT) {
        sv.join();
    }
    else {
        sv.detach();
    }

}
