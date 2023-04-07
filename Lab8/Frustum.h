#pragma once

#include "framework.h"

class Frustum {
public:
    Frustum(float screenDepth);

    void ConstructFrustum(XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
    bool CheckRectangle(XMFLOAT4 bbMin, XMFLOAT4 bbMax);
    XMFLOAT4* GetPlanes() { return planes_; };

    ~Frustum() = default;
private:
    float screenDepth_;
    XMFLOAT4 planes_[6];
};
