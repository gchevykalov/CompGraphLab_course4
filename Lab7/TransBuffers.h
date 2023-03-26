#include "Macros.h"
#include "LightCalc.h"

cbuffer WorldMatrixBuffer : register (b0) {
    float4x4 worldMatrix;
    float4 color;
};

cbuffer SceneMatrixBuffer : register (b1) {
    float4x4 viewProjectionMatrix;
};