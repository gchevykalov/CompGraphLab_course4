#include "Frustum.h"

Frustum::Frustum(float screenDepth):
    screenDepth_(screenDepth) {}

void Frustum::ConstructFrustum(XMMATRIX viewMatrix, XMMATRIX projectionMatrix) {
    XMFLOAT4X4 pMatrix;
    XMStoreFloat4x4(&pMatrix, projectionMatrix);

    float zMinimum = -pMatrix._43 / pMatrix._33;
    float r = screenDepth_ / (screenDepth_ - zMinimum);

    pMatrix._33 = r;
    pMatrix._43 = -r * zMinimum;
    projectionMatrix = XMLoadFloat4x4(&pMatrix);

    XMMATRIX finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

    XMFLOAT4X4 matrix;
    XMStoreFloat4x4(&matrix, finalMatrix);

    planes_[0].x = matrix._14 + matrix._13;
    planes_[0].y = matrix._24 + matrix._23;
    planes_[0].z = matrix._34 + matrix._33;
    planes_[0].w = matrix._44 + matrix._43;

    float length = sqrtf((planes_[0].x * planes_[0].x) + (planes_[0].y * planes_[0].y) + (planes_[0].z * planes_[0].z));
    planes_[0].x /= length;
    planes_[0].y /= length;
    planes_[0].z /= length;
    planes_[0].w /= length;

    planes_[1].x = matrix._14 - matrix._13;
    planes_[1].y = matrix._24 - matrix._23;
    planes_[1].z = matrix._34 - matrix._33;
    planes_[1].w = matrix._44 - matrix._43;

    length = sqrtf((planes_[1].x * planes_[1].x) + (planes_[1].y * planes_[1].y) + (planes_[1].z * planes_[1].z));
    planes_[1].x /= length;
    planes_[1].y /= length;
    planes_[1].z /= length;
    planes_[1].w /= length;

    planes_[2].x = matrix._14 + matrix._11;
    planes_[2].y = matrix._24 + matrix._21;
    planes_[2].z = matrix._34 + matrix._31;
    planes_[2].w = matrix._44 + matrix._41;

    length = sqrtf((planes_[2].x * planes_[2].x) + (planes_[2].y * planes_[2].y) + (planes_[2].z * planes_[2].z));
    planes_[2].x /= length;
    planes_[2].y /= length;
    planes_[2].z /= length;
    planes_[2].w /= length;

    planes_[3].x = matrix._14 - matrix._11;
    planes_[3].y = matrix._24 - matrix._21;
    planes_[3].z = matrix._34 - matrix._31;
    planes_[3].w = matrix._44 - matrix._41;

    length = sqrtf((planes_[3].x * planes_[3].x) + (planes_[3].y * planes_[3].y) + (planes_[3].z * planes_[3].z));
    planes_[3].x /= length;
    planes_[3].y /= length;
    planes_[3].z /= length;
    planes_[3].w /= length;

    planes_[4].x = matrix._14 - matrix._12;
    planes_[4].y = matrix._24 - matrix._22;
    planes_[4].z = matrix._34 - matrix._32;
    planes_[4].w = matrix._44 - matrix._42;

    length = sqrtf((planes_[4].x * planes_[4].x) + (planes_[4].y * planes_[4].y) + (planes_[4].z * planes_[4].z));
    planes_[4].x /= length;
    planes_[4].y /= length;
    planes_[4].z /= length;
    planes_[4].w /= length;

    planes_[5].x = matrix._14 + matrix._12;
    planes_[5].y = matrix._24 + matrix._22;
    planes_[5].z = matrix._34 + matrix._32;
    planes_[5].w = matrix._44 + matrix._42;

    length = sqrtf((planes_[5].x * planes_[5].x) + (planes_[5].y * planes_[5].y) + (planes_[5].z * planes_[5].z));
    planes_[5].x /= length;
    planes_[5].y /= length;
    planes_[5].z /= length;
    planes_[5].w /= length;
}

bool Frustum::CheckRectangle(XMFLOAT4 bbMin, XMFLOAT4 bbMax) {
    for (int i = 0; i < 6; i++) {
        float dotProduct = ((planes_[i].x * bbMin.x) + (planes_[i].y * bbMin.y) + (planes_[i].z * bbMin.z) + (planes_[i].w * 1.0f));
        if (dotProduct >= 0.0f) {
            continue;
        }

        dotProduct = ((planes_[i].x * bbMax.x) + (planes_[i].y * bbMin.y) + (planes_[i].z * bbMin.z) + (planes_[i].w * 1.0f));
        if (dotProduct >= 0.0f) {
            continue;
        }

        dotProduct = ((planes_[i].x * bbMin.x) + (planes_[i].y * bbMax.y) + (planes_[i].z * bbMin.z) + (planes_[i].w * 1.0f));
        if (dotProduct >= 0.0f) {
            continue;
        }

        dotProduct = ((planes_[i].x * bbMax.x) + (planes_[i].y * bbMax.y) + (planes_[i].z * bbMin.z) + (planes_[i].w * 1.0f));
        if (dotProduct >= 0.0f) {
            continue;
        }

        dotProduct = ((planes_[i].x * bbMin.x) + (planes_[i].y * bbMin.y) + (planes_[i].z * bbMax.z) + (planes_[i].w * 1.0f));
        if (dotProduct >= 0.0f) {
            continue;
        }

        dotProduct = ((planes_[i].x * bbMax.x) + (planes_[i].y * bbMin.y) + (planes_[i].z * bbMax.z) + (planes_[i].w * 1.0f));
        if (dotProduct >= 0.0f) {
            continue;
        }

        dotProduct = ((planes_[i].x * bbMin.x) + (planes_[i].y * bbMax.y) + (planes_[i].z * bbMax.z) + (planes_[i].w * 1.0f));
        if (dotProduct >= 0.0f) {
            continue;
        }

        dotProduct = ((planes_[i].x * bbMax.x) + (planes_[i].y * bbMax.y) + (planes_[i].z * bbMax.z) + (planes_[i].w * 1.0f));
        if (dotProduct >= 0.0f) {
            continue;
        }

        return false;
    }

    return true;
}