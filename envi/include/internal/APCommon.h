#pragma once
#include "Envi.h"
#include <atomic>
#include <thread>

namespace Envi {

    struct ImageRect {
        ImageRect() : ImageRect(0, 0, 0, 0) {}
        ImageRect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
        int left;
        int top;
        int right;
        int bottom;
        bool Contains(const ImageRect &a) const { return left <= a.left && right >= a.right && top <= a.top && bottom >= a.bottom; }
    };

    struct Image {
        ImageRect Bounds;
        int RowStrideInBytes = 0;
        bool isContiguous = false;
        // alpha is always unused and might contain garbage
        const ImageBGRA *Data = nullptr;
    };

    ENVI_EXTERN Image CreateImage(const ImageRect &imgrect, int rowpadding, const ImageBGRA *data);

    ENVI_EXTERN int Index(const Monitor &mointor);
    // unique identifier
    ENVI_EXTERN int Id(const Monitor &mointor);
    ENVI_EXTERN int Adapter(const Monitor &mointor);
    ENVI_EXTERN int OffsetX(const Monitor &mointor);
    ENVI_EXTERN int OffsetY(const Monitor &mointor);
    ENVI_EXTERN void OffsetX(Monitor &mointor, int x);
    ENVI_EXTERN void OffsetY(Monitor &mointor, int y);
    ENVI_EXTERN int OffsetX(const Window &mointor);
    ENVI_EXTERN int OffsetY(const Window &mointor);
    ENVI_EXTERN void OffsetX(Window &mointor, int x);
    ENVI_EXTERN void OffsetY(Window &mointor, int y);
    ENVI_EXTERN const char *Name(const Monitor &mointor);
    ENVI_EXTERN const char *Name(const Window &mointor);
    ENVI_EXTERN int Height(const Monitor &mointor);
    ENVI_EXTERN int Width(const Monitor &mointor);
    ENVI_EXTERN void Height(Monitor &mointor, int h);
    ENVI_EXTERN void Width(Monitor &mointor, int w);
    ENVI_EXTERN int Height(const Window &mointor);
    ENVI_EXTERN int Width(const Window &mointor);
    ENVI_EXTERN void Height(Window &mointor, int h);
    ENVI_EXTERN void Width(Window &mointor, int w);
    ENVI_EXTERN int Height(const Image &img);
    ENVI_EXTERN int Width(const Image &img);
    ENVI_EXTERN int X(const Point &p);
    ENVI_EXTERN int Y(const Point &p);
    ENVI_EXTERN int Height(const ImageRect &rect);
    ENVI_EXTERN int Width(const ImageRect &rect);
    ENVI_EXTERN const ImageRect &Rect(const Image &img);

    enum DUPL_RETURN {
        DUPL_RETURN_SUCCESS = 0,
        DUPL_RETURN_ERROR_EXPECTED = 1,
        DUPL_RETURN_ERROR_UNEXPECTED = 2
    };
    Monitor CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string &n, float scale);
    Monitor CreateMonitor(int index, int id, int adapter, int h, int w, int ox, int oy, const std::string &n, float scale);
    ENVI_EXTERN Image CreateImage(const ImageRect &imgrect, int rowpadding, const ImageBGRA *data);

    template <typename F, typename W>
    struct CaptureData {
        F OnNewFrame;
        F OnFrameChanged;
        W getThingsToWatch;
    };

    struct CommonData {
        // Used to indicate abnormal error condition
        std::atomic<bool> unexpectedErrorEvent;
        // Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate
        // the duplication interface
        std::atomic<bool> expectedErrorEvent;
        // Used to signal to threads to exit
        std::atomic<bool> terminateThreadsEvent;
        std::atomic<bool> paused;
    };

    struct Thread_Data {
        CaptureData<WindowCaptureCallback, WindowCallback> WindowCaptureData;
        CommonData CommonData_;
    };

    class BaseFrameProcessor {
        public:
            std::shared_ptr<Thread_Data> Data;
            std::unique_ptr<unsigned char[]> ImageBuffer;
            int ImageBufferSize = 0;
            // bool FirstRun = true;
    };


    template<class F, class T>
    void ProcessCapture(const F& data, T& base, Envi::Window& window, const unsigned char *startsrc, int srcrowstride ) {

        ImageRect imageract;
        imageract.left = 0;
        imageract.top = 0;
        imageract.bottom = Height(window);
        imageract.right = Width(window);

        const auto sizeofimgbgra = static_cast<int>(sizeof(ImageBGRA));
        const auto startimgsrc = reinterpret_cast<const ImageBGRA *>(startsrc);
        auto dstrowstride = sizeofimgbgra * Width(window);

        if (data.OnNewFrame) {
            auto wholeimg = CreateImage(imageract, srcrowstride, startimgsrc);
            wholeimg.isContiguous = dstrowstride == srcrowstride;
            data.OnNewFrame(wholeimg, window);
        }

        if (data.OnFrameChanged) {
            auto newimg = CreateImage(imageract, srcrowstride - dstrowstride, startimgsrc);
            auto oldimg = CreateImage(imageract, 0, reinterpret_cast<const ImageBGRA *>(base.ImageBuffer.get()));

            if (Width(newimg) != Width(oldimg) || Height(newimg) != Height(oldimg) ) {
                data.OnFrameChanged(newimg, window);
            }

        }

        // Copy image to FrameProcessor
        // auto startdst = base.ImageBuffer.get();
        // if (dstrowstride == srcrowstride) { // no need for multiple calls, there is no padding here
        //     memcpy(startdst, startsrc, dstrowstride * Height(window));
        // }
        // else {
        //     for (auto i = 0; i < Height(window); i++) {
        //         memcpy(startdst + (i * dstrowstride), startsrc + (i * srcrowstride), dstrowstride);
        //     }
        // }

    //     if (data.OnNewFrame) { // each frame we still let the caller know if asked for
    //         auto wholeimg = CreateImage(imageract, srcrowstride, startimgsrc);
    //         wholeimg.isContiguous = dstrowstride == srcrowstride;
    //         data.OnNewFrame(wholeimg, window);
    //     }
    //     if (data.OnFrameChanged) { // difs are needed!
    //         if (base.FirstRun) {
    //             // first time through, just send the whole image
    //             auto wholeimg = CreateImage(imageract, srcrowstride, startimgsrc);
    //             wholeimg.isContiguous = dstrowstride == srcrowstride;
    //             data.OnFrameChanged(wholeimg, window);
    //             base.FirstRun = false;
    //         }
    //         else {
    //             // user wants difs, lets do it!
    //             auto newimg = CreateImage(imageract, srcrowstride - dstrowstride, startimgsrc);
    //             auto oldimg = CreateImage(imageract, 0, reinterpret_cast<const ImageBGRA *>(base.ImageBuffer.get()));
    //             auto imgdifs = GetDifs(oldimg, newimg);

    //             for (auto &r : imgdifs) {
    //                 auto leftoffset = r.left * sizeofimgbgra;
    //                 auto thisstartsrc = startsrc + leftoffset + (r.top * srcrowstride);

    //                 auto difimg = CreateImage(r, srcrowstride, reinterpret_cast<const ImageBGRA *>(thisstartsrc));
    //                 difimg.isContiguous = false;
    //                 data.OnFrameChanged(difimg, mointor);
    //             }
    //         }
    //     }

    }

};
