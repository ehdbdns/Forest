//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2015 All Rights Reserved.
//
// General helper code.
//***************************************************************************************

#pragma once
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN // 从 Windows 头中排除极少使用的资料
#include <tchar.h>
#include<cmath>
//添加WTL支持 方便使用COM
#include <wrl.h>
using namespace Microsoft;
using namespace Microsoft::WRL;
#include <dxgi1_6.h>
#include <DirectXMath.h>
using namespace DirectX;
//for d3d12
#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
//linker
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include<comdef.h>
#if defined(_DEBUG)
#include <dxgidebug.h>
#include<strsafe.h>
#endif
#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3dx12.h"
#include "DDSTextureLoader.h"
#include "MathHelper.h"
#include<queue>
#include <random>
#include <time.h>
#include<iostream>
#define GRS_WND_CLASS_NAME _T("Game Window Class")
#define GRS_WND_TITLE   _T("DirectX12 Trigger Sample")
#define GRS_UPPER(A,B) ((UINT)(((A)+((B)-1))&~(B - 1)))


#include<string>
extern const int gNumFrameResources;

inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
{
    if (obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
{
    if (obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
{
    if (obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}

inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}
struct BVHtriangle {
    XMFLOAT3 pos[3];
    XMFLOAT3 gravity;
};

struct passconstant {
    XMFLOAT4X4 MVP = MathHelper::Identity4x4();
    XMFLOAT3 eyepos;
    float pad1;
    XMFLOAT3 AL;
    float pad2;
    XMFLOAT3 BL;
    float m;
    XMFLOAT3 lightdir;
    float pad3;
    XMFLOAT4 md;
    XMFLOAT3 r0;
    float pad;
    XMFLOAT4X4 V;
    XMFLOAT4X4 W = MathHelper::Identity4x4();
    XMFLOAT4X4 VP;
    XMFLOAT4X4 WinvT;
    XMFLOAT4X4 P;
    XMFLOAT4X4 S;
    UINT boxNum;
};

struct objectconstant {
    XMFLOAT4X4 world;
    XMFLOAT4X4 invTworld;
    int texIndex;
    int matIndex;
};

struct modelVertex {
    XMFLOAT3 position;
    XMFLOAT2 texuv;
    XMFLOAT3 normal;
    XMFLOAT4 tangent;
    float AOk;
    XMFLOAT3 color;

    XMFLOAT4 blendW;
    XMUINT4 blendIndex;
};
struct keyframe {
    float starttime;
    XMFLOAT3 translation;
    XMFLOAT3 scale;
    XMFLOAT4 quat;
};
struct boneAnimation {
    int numkeyframe;
    std::vector<keyframe> keyframes;
    XMMATRIX interpolation(float time) {

        if (time <= keyframes[0].starttime) {
            XMVECTOR S = XMLoadFloat3(&keyframes[0].scale);
            XMVECTOR P = XMLoadFloat3(&keyframes[0].translation);
            XMVECTOR Q = XMLoadFloat4(&keyframes[0].quat);
            XMVECTOR zero = XMVectorSet(0, 0, 0, 1.0f);
            return  XMMatrixAffineTransformation(S, zero, Q, P);
        }
        if (time >= keyframes[numkeyframe - 1].starttime) {
            XMVECTOR S = XMLoadFloat3(&keyframes[numkeyframe - 1].scale);
            XMVECTOR P = XMLoadFloat3(&keyframes[numkeyframe - 1].translation);
            XMVECTOR Q = XMLoadFloat4(&keyframes[numkeyframe - 1].quat);
            XMVECTOR zero = XMVectorSet(0, 0, 0, 1.0f);
            return  XMMatrixAffineTransformation(S, zero, Q, P);
        }
        for (int i = 0;i < numkeyframe;i++) {
            if (time >= keyframes[i].starttime && time <= keyframes[i + 1].starttime) {
                float t = (time - keyframes[i].starttime) / (keyframes[i + 1].starttime - keyframes[i].starttime);
                XMVECTOR S = XMVectorLerp(XMLoadFloat3(&keyframes[i].scale), XMLoadFloat3(&keyframes[i + 1].scale), t);
                XMVECTOR P = XMVectorLerp(XMLoadFloat3(&keyframes[i].translation), XMLoadFloat3(&keyframes[i + 1].translation), t);
                XMVECTOR Q = XMQuaternionSlerp(XMLoadFloat4(&keyframes[i].quat), XMLoadFloat4(&keyframes[i + 1].quat), t);
                XMVECTOR zero = XMVectorSet(0, 0.0f, 0.0f, 1.0f);//旋转轴的所经过的原点
                return  XMMatrixAffineTransformation(S, zero, Q, P);
            }
        }
    }

};
struct animationClips {
    std::string clipname;
    std::vector<boneAnimation>boneAnimations;
    int bonenum;
};

struct meshdata {
    ;
    std::vector<XMFLOAT3> vertices;
    std::vector<std::uint16_t> indices;

};


class d3dUtil
{
public:

    static bool IsKeyDown(int vkeyCode);

    static std::string ToString(HRESULT hr);

    static UINT CalcConstantBufferByteSize(UINT byteSize)
    {

        return (byteSize + 255) & ~255;
    }

    static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        const std::wstring& filename,
        const D3D_SHADER_MACRO* defines,
        const std::string& entrypoint,
        const std::string& target);
};

class DxException
{
public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};






#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
     if(FAILED(hr__)){throw DxException(hr__, L#x, wfn, __LINE__);}  \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

