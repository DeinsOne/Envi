// #include "Envi.h"
#include "internal/EnviCommon.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>


namespace Envi {

    int Index(const Monitor &mointor) { return mointor.Index; }
    int Id(const Monitor &mointor) { return mointor.Id; }
    int Adapter(const Monitor &mointor) { return mointor.Adapter; }
    int OffsetX(const Monitor &mointor) { return mointor.OffsetX; }
    int OffsetY(const Monitor &mointor) { return mointor.OffsetY; }
    void OffsetX(Monitor &mointor, int x) { mointor.OffsetX = x; }
    void OffsetY(Monitor &mointor, int y) { mointor.OffsetY = y; }
    int OffsetX(const Window &mointor) { return mointor.Position.x; }
    int OffsetY(const Window &mointor) { return mointor.Position.y; }
    void OffsetX(Window &mointor, int x) { mointor.Position.x = x; }
    void OffsetY(Window &mointor, int y) { mointor.Position.y = y; }
    const char *Name(const Monitor &mointor) { return mointor.Name; }
    const char *Name(const Window &mointor) { return mointor.Name; }
    int Height(const Monitor &mointor) { return mointor.Height; }
    int Width(const Monitor &mointor) { return mointor.Width; }
    void Height(Monitor &mointor, int h) { mointor.Height = h; }
    void Width(Monitor &mointor, int w) { mointor.Width = w; }
    int Height(const Window &mointor) { return mointor.Size.y; }
    int Width(const Window &mointor) { return mointor.Size.x; }
    void Height(Window &mointor, int h) { mointor.Size.y = h; }
    void Width(Window &mointor, int w) { mointor.Size.x = w; }
    int Height(const Image &img) { return Height(img.Bounds); }
    int Width(const Image &img) { return Width(img.Bounds); }
    int Height(const ImageRect &rect) { return rect.bottom - rect.top; }
    int Width(const ImageRect &rect) { return rect.right - rect.left; }
    const ImageRect &Rect(const Image &img) { return img.Bounds; }
    int X(const Point &p) { return p.x; }
    int Y(const Point &p) { return p.y; }

    Monitor CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string &n, float scaling) {
        Monitor ret = {};
        ret.Index = index;

        ret.Id = id;
        assert(n.size() + 1 < sizeof(ret.Name));
        memcpy(ret.Name, n.c_str(), n.size() + 1);
        ret.OriginalOffsetX = ret.OffsetX = ox;
        ret.OriginalOffsetY = ret.OffsetY = oy;
        ret.OriginalWidth = ret.Width = w;
        ret.OriginalHeight = ret.Height = h;
        ret.Scaling = scaling;
        return ret;
    }

    Monitor CreateMonitor(int index, int id, int adapter, int h, int w, int ox, int oy, const std::string &n, float scaling) {
        Monitor ret = CreateMonitor(index, id, h, w, ox, oy, n, scaling);
        ret.Adapter = adapter;
        return ret;
    }

    Image CreateImage(const ImageRect &imgrect, int rowStrideInBytes, const ImageBGRA *data) {
        Image ret;
        ret.Bounds = imgrect;
        ret.Data = data;
        ret.RowStrideInBytes = rowStrideInBytes;
        ret.isContiguous = rowStrideInBytes == sizeof(ImageBGRA) * Width(imgrect);
        return ret;
    }

};

