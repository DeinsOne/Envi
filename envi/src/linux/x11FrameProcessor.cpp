#include "linux/x11FrameProcessor.h"
#include "internal/EnviCommon.h"
#include "internal/CaptureRecover.h"
#include <X11/Xutil.h> 

#include <jpeglib.h>

std::vector<std::vector<unsigned char>> process_original(XImage * image) {
    std::vector<std::vector<unsigned char>> image_data;
    std::vector<unsigned char> image_data_row;
    unsigned long red_mask = image->red_mask;
    unsigned long green_mask = image->green_mask;
    unsigned long blue_mask = image->blue_mask;

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            unsigned long pixel = XGetPixel(image, x, y);

            unsigned char blue = pixel & blue_mask;
            unsigned char green = (pixel & green_mask) >> 8;
            unsigned char red = (pixel & red_mask) >> 16;

            image_data_row.push_back(red);
            image_data_row.push_back(green);
            image_data_row.push_back(blue);
        }
        image_data.push_back(image_data_row);
        image_data_row.clear();
    }

    return image_data;
};

bool save_to_jpeg(XImage* image, const char * path, int quality) {
    auto image_data = process_original(image);

    FILE *fp = NULL;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY row;

    fp = fopen(path, "wb");
    if (!fp) {
        // std::cout << "Failed to create file " << path << std::endl;
        return false;
    }
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = image->width;
    cinfo.image_height = image->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality (&cinfo, quality, true);
    jpeg_start_compress(&cinfo, true);
    for(std::vector<std::vector<unsigned char>>::size_type i = 0; i != image_data.size(); i++) {
        row = (JSAMPARRAY) &image_data[i];
        jpeg_write_scanlines(&cinfo, row, 1);
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    if (fp != NULL) fclose(fp);

    return true;
};

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
        ProcessFrameChanged(Data->WindowCaptureData, *this, selectedwindow );

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
        XImage imageBackup;
        CapturingMutex.lock();
        imageBackup = *XImage_;
        imageBackup.data = (char*)malloc(XImage_->width * XImage_->height * (XImage_->bitmap_unit / 8) );
        memcpy(imageBackup.data, XImage_->data, XImage_->width * XImage_->height * (XImage_->bitmap_unit / 8) );
        CapturingMutex.unlock();

        RecoverThreads++;
        std::chrono::duration<double> now = std::chrono::high_resolution_clock::now().time_since_epoch();
        std::chrono::duration<double> ini = Data->WindowCaptureData.TimeStarted.time_since_epoch();
        std::chrono::seconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - ini).count();

        std::string fnm = std::to_string(milliseconds) + ".jpg";

        // TODO: save image
        save_to_jpeg(&imageBackup, (ENVI_RECOVER_DIR + std::to_string(wnd.Handle) + "/" + fnm.c_str()).c_str(), 100);

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
