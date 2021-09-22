#include "linux/x11FrameProcessor.h"
#include "internal/EnviCommon.h"
#include <X11/Xutil.h> 

Envi::X11FrameProcessor::~X11FrameProcessor() {
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

    XShmAttach(SelectedDisplay, ShmInfo.get());

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
        ProcessFrameChanged(Data->WindowCaptureData, *this, selectedwindow );

        return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; //window size changed. This will rebuild everything
    }

    if (wndattr.map_state != IsViewable) {
        return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; //window was collapsed
    }


    if(!XShmGetImage(SelectedDisplay,
                     selectedwindow.Handle,
                     XImage_,
                     0,
                     0,
                     AllPlanes)) {
        return DUPL_RETURN_ERROR_EXPECTED;
    }

    ProcessCapture(Data->WindowCaptureData, *this, selectedwindow, (unsigned char*)XImage_->data, XImage_->bytes_per_line);
    return Ret;
}
