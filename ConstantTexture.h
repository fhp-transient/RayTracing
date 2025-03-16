//
// Created by fhp on 25-3-16.
//

#ifndef CONSTANTTEXTURE_H
#define CONSTANTTEXTURE_H

#include "Texture.h"

class ConstantTexture : public Texture {
public:
    ConstantTexture() = default;

    ConstantTexture(Vector3f c) : color(c) {}

    virtual Vector3f Evaluate(__attribute__((unused))  float u, __attribute__((unused))  float v, __attribute__((unused))  const Vector3f &p) const override {
        return color;
    }

    Vector3f color;

};

#endif //CONSTANTTEXTURE_H
