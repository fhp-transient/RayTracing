//
// Created by fhp on 25-3-16.
//

#ifndef IMAGETEXTURE_H
#define IMAGETEXTURE_H
#include "Texture.h"

class ImageTexture : public Texture {
public:
    ImageTexture() = default;

    ImageTexture(unsigned char *pixels, int _nx, int _ny, int _channel)
            : data(pixels), nx(_nx), ny(_ny), channel(_channel) {}

    ~ImageTexture() {
        delete[] data;
    }

    [[nodiscard]] Vector3f Evaluate(float u, float v) const override {
        int i = (u) * nx;
        int j = (1 - v) * ny - 0.001;
        if (i < 0) i = 0;
        if (j < 0) j = 0;
        if (i > nx - 1) i = nx - 1;
        if (j > ny - 1) j = ny - 1;
        int start = channel * i + channel * nx * j;
        float r = int(data[start]) / 255.0;
        float g = int(data[start + 1]) / 255.0;
        float b = int(data[start + 2]) / 255.0;
        return Vector3f(r, g, b);
    }

    virtual float EvaluateAlpha(float u, float v) {
        if (channel == 4) {
            int i = (u) * nx;
            int j = (1 - v) * ny - 0.001;
            if (i < 0) i = 0;
            if (j < 0) j = 0;
            if (i > nx - 1) i = nx - 1;
            if (j > ny - 1) j = ny - 1;
            return int(data[channel * i + channel * nx * j + 3]) / 255.0;
        } else {
            return 1.0;
        }
    }

    unsigned char *data;
    int nx, ny, channel;
};

#endif //IMAGETEXTURE_H
