//
// Created by fhp on 25-3-16.
//

#ifndef CONSTANTTEXTURE_H
#define CONSTANTTEXTURE_H

#include "Texture.h"

class ConstantTexture : public Texture {
public:
    ConstantTexture() = default;

    explicit ConstantTexture(Vector3f c) : color(c) {}

    [[nodiscard]] Vector3f Evaluate(float u, float v) const {
        return color;
    }

    Vector3f color;

};

#endif //CONSTANTTEXTURE_H
