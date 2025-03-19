//
// Created by fhp on 25-3-16.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include "Vector.hpp"

class Texture {
public:
    virtual Vector3f Evaluate(float u, float v) const = 0;
    virtual float EvaluateAlpha(float u, float v) {
        return 1.0;
    }
};


#endif //TEXTURE_H
