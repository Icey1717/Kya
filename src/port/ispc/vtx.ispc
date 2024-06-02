typedef float<4> float4;

export void gouraudMappingNoFog(
    uniform int vtxCount,
    uniform int vtxReg,
    uniform float objToScreen[4][4],
    uniform float4 pSTQ[],
    uniform float4 pRGBA[],
    uniform float4 pXYZ[])
{
    foreach (vtxIndex = 0 ... vtxCount) {
        float4 stq = pSTQ[vtxIndex];
        float4 rgba = pRGBA[vtxIndex];
        float4 xyz = pXYZ[vtxIndex];

        // Perform transformation
        float4 screenVtx;
        screenVtx.x = xyz.x * objToScreen[0][0] + xyz.y * objToScreen[0][1] + xyz.z * objToScreen[0][2] + objToScreen[0][3];
        screenVtx.y = xyz.x * objToScreen[1][0] + xyz.y * objToScreen[1][1] + xyz.z * objToScreen[1][2] + objToScreen[1][3];
        screenVtx.z = xyz.x * objToScreen[2][0] + xyz.y * objToScreen[2][1] + xyz.z * objToScreen[2][2] + objToScreen[2][3];
        screenVtx.w = xyz.x * objToScreen[3][0] + xyz.y * objToScreen[3][1] + xyz.z * objToScreen[3][2] + objToScreen[3][3];

        float invW = 1.0f / abs(screenVtx.w);

        //pSTQ[0] = stq[0] * invW;
        //pSTQ[1] = stq[1] * invW;
        //pSTQ[2] = stq[2] * invW;

        screenVtx[0] = screenVtx[0] * invW;
        screenVtx[1] = screenVtx[1] * invW;
        screenVtx[2] = screenVtx[2] * invW;
    }
}