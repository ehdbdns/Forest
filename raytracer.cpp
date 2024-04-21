﻿// d3d12practicelast.cpp : 定义应用程序的入口点。
//
#pragma once
#include"rayTracer.h"
#define MAX_LOADSTRING 100
#define GRS_WND_CLASS_NAME _T("GRS Game Window Class")
#define GRS_WND_TITLE	_T("GRS DirectX12 Trigger Sample")

UINT samplerindex = 0;
UINT samplercount = 5;
const UINT nFrameBackBufCount = 3u;

#include <D3DCompiler.h>
UINT __stdcall threadfunc(void* ppara);
class CGRSCOMException
{
public:
    CGRSCOMException(HRESULT hr) : m_hrError(hr)
    {
    }
    HRESULT Error() const
    {
        return m_hrError;
    }
private:
    const HRESULT m_hrError;
};
UINT64 starttime = 0;
UINT64 currenttime = 0;
XMVECTOR eyepos = XMVectorSet(0.0f, 10.0f, -20.0f, 1.0f);
XMVECTOR target = XMVectorSet(0.0f, .0f, 1.0f, 0.0f);
XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
XMFLOAT2 mousepospre;
float theta, phi;
float timesecond;
void OnMouseDown(WPARAM wparam, int x, int y) {
    mousepospre.x = x;
    mousepospre.y = y;
}
void OnMouseMove(WPARAM wparam, int x, int y) {
    if ((wparam & MK_LBUTTON) != 0) {
        WriteConsole(g_hOutput, "1", 2, NULL, NULL);
        float dx = x - mousepospre.x;
        float dy = (y - mousepospre.y);
        float dtheta = XMConvertToRadians(0.25f * static_cast<float>(dx));
        float dphi = XMConvertToRadians(0.25f * static_cast<float>(dy));
        theta -= dtheta;
        phi -= dphi;
        if (phi >= XM_PI)
            phi -= XM_2PI;
        if (phi <= -XM_PI)
            phi += XM_2PI;
        if (theta >= XM_PI)
            theta -= XM_2PI;
        if (theta <= -XM_2PI)
            theta += XM_2PI;
        mousepospre.x = x;
        mousepospre.y = y;
    }
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYUP:
    {
        if (VK_SPACE == (wParam & 0xFF)) {
            samplerindex++;
            WriteConsole(g_hOutput, L"1", 2, NULL, NULL);
            if (samplerindex == samplercount)
                samplerindex = 0;
        }

    }break;
    case WM_LBUTTONDOWN:

    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN: {
        OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    }break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:


    case WM_MOUSEMOVE: {
        OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

struct threadparas {
    UINT								nIndex;				//序号
    DWORD								dwThisThreadID;
    HANDLE								hThisThread;
    DWORD								dwMainThreadID;
    HANDLE								hMainThread;
    HANDLE								hRunEvent;
    HANDLE								hEventRenderOver;
    UINT*								nCurrentFrameIndex;//当前渲染后缓冲序号
    ULONGLONG							nStartTime;		   //当前帧开始时间
    ULONGLONG							nCurrentTime;	   //当前时间
    D3D12_VIEWPORT*                     stViewPort;
    D3D12_RECT*							stScissorRect;
    std::unordered_map<std::string,std::unique_ptr< RenderItem>>(*RenderItemTable);
    std::unordered_map<std::string,std::unique_ptr< GeometryItem>>(*GeometryItemTable);
    std::unordered_map<std::string,std::unique_ptr< BufferResourceItem<passconstant>>>(*BufferResourceItemTable);
    std::unordered_map<std::string,std::unique_ptr< TextureResourceItem>>(*TextureResourceItemTable);
    std::unordered_map<std::string,std::unique_ptr< SamplerResourceItem>>(*SamplerResourceItemTable);
    std::unordered_map<std::string,std::unique_ptr< RootSignatureItem>>(*RootSignatureItemTable);
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>(*PSOTable);
    ID3D12Device4* device;
    ID3D12RootSignature* rs;
    ID3D12GraphicsCommandList* cmdlist;
    ID3D12CommandAllocator* cmdalloc;
    ID3D12PipelineState* PSO;
    ID3D12Resource*               pIARenderTargets[nFrameBackBufCount];
    ID3D12DescriptorHeap* RTVHeap;
    std::string                         RenderItemName;
    std::string                         RootSignatureItemName;
    std::string                         SamplerResourceItemItemName;
    std::string                         TextureResourceItemName;
    std::string                         BufferResourceItemName;
    std::string                         PSOName;
};

class FirstAPP:APP {
public:
    FirstAPP() = default;
    void init(HINSTANCE hInstance, int       nCmdShow);
    void startRender();
    void createMemoryManager();
    void createDescriptorHeap();
    void createCommandQueueAndSwapChain();
    void createRootSignature();
    void createCommandList();
    void compileShadersAndCreatePSO();
    void createDepthBufferAndSampler();
    void createGeometryItemAndRenderItem();
    void createResourceItem();
    void startSubThread();
    void startRenderLoop();
    int createNewThreadToRender(std::string RenderItemName, std::string BufferResourceItemName, std::string TextureResourceItemName, std::string RootSignatureItemName, std::string SamplerResourceItemItemName, std::string PSOName,int order);
    void startAndWaitNewThread(int newThreadNum);
    void killThread(int ThreadNote);
private:


    D3D12_RECT							stScissorRect = { 0, 0, static_cast<LONG>(iWidth), static_cast<LONG>(iHeight) };
    ATOM                MyRegisterClass(HINSTANCE hInstance);
    BOOL                InitInstance(HINSTANCE, int, HWND, UINT, UINT);
    HINSTANCE hInst;                                // 当前实例
    WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
    WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
    TCHAR								pszAppPath[MAX_PATH] = {};
    MSG  msg = {};
    D3D12_VIEWPORT stViewPort = { 0.0f, 0.0f, static_cast<float>(iWidth), static_cast<float>(iHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
    // 此代码模块中包含的

    UINT nFrameIndex = 0;
    UINT nFrame = 0;
  
    UINT nRTVDescriptorSize = 0U;
    std::vector<threadparas> paras;
    std::vector<int> existThreadList;
    std::vector<int> deadThreadList;

    std::vector<HANDLE> HandleMessage[4];
    std::vector<ID3D12CommandList*>cmdlists;
    float fAspectRatio = 3.0f;
    float rotationAngle = 0.0f;


    UINT64 n64FenceValue = 0ui64;
    HANDLE hFenceEvent = nullptr;
    UINT nSamplerDescriptorSize;


    ComPtr<ID3D12CommandQueue>           pICommandQueue;
    ComPtr<IDXGISwapChain1>              pISwapChain1;
    ComPtr<IDXGISwapChain3>              pISwapChain3;

    ComPtr<ID3D12Resource>               pIARenderTargets[nFrameBackBufCount];
    ComPtr<ID3D12CommandAllocator>       pIcmdallpre;
    ComPtr<ID3D12CommandAllocator>       pIcmdallpost;
    ComPtr<ID3D12RootSignature>          pIRootSignature;
    ComPtr<ID3D12RootSignature>          pICSRootSignature;
    ComPtr<ID3D12PipelineState>          pIPipelineState;

    ComPtr<ID3D12GraphicsCommandList>    pIcmdlistpre;
    ComPtr<ID3D12GraphicsCommandList>    pIcmdlistpost;
    ComPtr<ID3D12Fence>                  pIFence;
    ComPtr<ID3D12DescriptorHeap>		 pISRVHeap;
    ComPtr<ID3D12DescriptorHeap>         pISamheap;
    passconstant currpasscb;
    ComPtr<ID3D12DescriptorHeap>                  pIpresrvheap;
    ComPtr<ID3D12DescriptorHeap>                  pIpresamheap;
    ComPtr<ID3D12DescriptorHeap>         pIRTVHeap;
    ComPtr<ID3D12DescriptorHeap>         pIDSVHeap;


    std::unordered_map<std::string, std::unique_ptr< RenderItem>>RenderItemTable;
    std::unordered_map<std::string, std::unique_ptr< GeometryItem>>GeometryItemTable;
    std::unordered_map<std::string, std::unique_ptr< BufferResourceItem<passconstant>>>BufferResourceItemTable;
    std::unordered_map<std::string, std::unique_ptr< TextureResourceItem>>TextureResourceItemTable;
    std::unordered_map<std::string, std::unique_ptr< SamplerResourceItem>>SamplerResourceItemTable;
    std::unordered_map<std::string, std::unique_ptr< RootSignatureItem>>RootSignatureItemTable;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>PSOTable;
    std::unordered_map<std::string, std::unique_ptr< RT_DS_TextureSegregatedFreeLists>>RTDSSFLTable;
    std::unordered_map<std::string, std::unique_ptr< NON_RT_DS_TextureSegregatedFreeLists>>NONRTDSSFLTable;
    std::unordered_map<std::string, std::unique_ptr< uploadBuddySystem>>upBSTable;
    std::unordered_map<std::string, std::unique_ptr< defaultBuddySystem>>defBSTable;
};
void FirstAPP::init(HINSTANCE hInstance, int       nCmdShow) {
    initDX12(hInstance, WndProc, nCmdShow);
    createDescriptorHeap();
    createCommandQueueAndSwapChain();
    createRootSignature();
    createCommandList();
    compileShadersAndCreatePSO();
    createMemoryManager();
    createDepthBufferAndSampler();
    createGeometryItemAndRenderItem();
    createResourceItem();
    startSubThread();
}
void FirstAPP::startRender() {

    startRenderLoop();
}
void FirstAPP::createDescriptorHeap() {
    //描述符堆及采样器堆
    D3D12_DESCRIPTOR_HEAP_DESC rtvheapdesc = {};
    rtvheapdesc.NumDescriptors = nFrameBackBufCount;//之后会加数量
    rtvheapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvheapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(pID3DDevice->CreateDescriptorHeap(&rtvheapdesc, IID_PPV_ARGS(&pIRTVHeap)));
    D3D12_DESCRIPTOR_HEAP_DESC presrvheapdesc = {};
    presrvheapdesc.NumDescriptors = 10;//先创建一个静态堆，之后等东西多了再加个动态管理器
    presrvheapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    presrvheapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    ThrowIfFailed(pID3DDevice->CreateDescriptorHeap(&presrvheapdesc, IID_PPV_ARGS(&pIpresrvheap)));
    presrvheapdesc.NumDescriptors = 1;
    presrvheapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    presrvheapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    ThrowIfFailed(pID3DDevice->CreateDescriptorHeap(&presrvheapdesc, IID_PPV_ARGS(&pIpresamheap)));
    presrvheapdesc.NumDescriptors = 1;
    presrvheapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    presrvheapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    ThrowIfFailed(pID3DDevice->CreateDescriptorHeap(&presrvheapdesc, IID_PPV_ARGS(&pIDSVHeap)));
    //全局堆偏移表，记录目前堆一共绑定了多少描述符
    HeapOffsetTable[pIpresrvheap.Get()] = 0;
    HeapOffsetTable[pIpresamheap.Get()] = 0;
    HeapOffsetTable[pIDSVHeap.Get()] = 0;
    HeapOffsetTable[pIRTVHeap.Get()] = 0;
}
void FirstAPP::createCommandQueueAndSwapChain() {
    //创建命令队列
    {
        D3D12_COMMAND_QUEUE_DESC commandqueuedesc = {};
        commandqueuedesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ThrowIfFailed(pID3DDevice->CreateCommandQueue(&commandqueuedesc, IID_PPV_ARGS(&pICommandQueue)));
    }
    //创建交换链及其资源
    {
        DXGI_SWAP_CHAIN_DESC1 swapchaindesc = {};
        swapchaindesc.BufferCount = nFrameBackBufCount;
        swapchaindesc.Width = iWidth;
        swapchaindesc.Height = iHeight;
        swapchaindesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchaindesc.SampleDesc.Count = 1;
        if (hWnd == nullptr)
            WriteConsole(g_hOutput, L"no", 3, NULL, NULL);
        ThrowIfFailed(pIDXGIFactory5->CreateSwapChainForHwnd(pICommandQueue.Get(), hWnd, &swapchaindesc, nullptr, nullptr, &pISwapChain1));
        swapchaindesc.BufferCount = 2;

        ThrowIfFailed(pISwapChain1.As(&pISwapChain3));

        nFrameIndex = pISwapChain3->GetCurrentBackBufferIndex();

        nRTVDescriptorSize = pID3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvcpuhandle(pIRTVHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0;i < 3;i++) {
            ThrowIfFailed(pISwapChain3->GetBuffer(i, IID_PPV_ARGS(&pIARenderTargets[i])));
            pID3DDevice->CreateRenderTargetView(pIARenderTargets[i].Get(), nullptr, rtvcpuhandle);
            rtvcpuhandle.Offset(1, nRTVDescriptorSize);
            HeapOffsetTable[pIRTVHeap.Get()]++;
        }
    }
}
void FirstAPP::createRootSignature() {
    {//跟签名
        D3D12_FEATURE_DATA_ROOT_SIGNATURE stFeatureData = {};
        // 检测是否支持V1.1版本的根签名
        stFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        if (FAILED(pID3DDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &stFeatureData, sizeof(stFeatureData))))
        {
            stFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }
        //描述符表
        CD3DX12_DESCRIPTOR_RANGE roottables[4] = {};
        roottables[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        roottables[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        roottables[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
        roottables[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

        //跟参数
        CD3DX12_ROOT_PARAMETER rootparas[4] = {};
        rootparas[0].InitAsDescriptorTable(1, &roottables[0], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[1].InitAsDescriptorTable(1, &roottables[1], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[2].InitAsDescriptorTable(1, &roottables[2], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[3].InitAsDescriptorTable(1, &roottables[3], D3D12_SHADER_VISIBILITY_ALL);



        CD3DX12_ROOT_SIGNATURE_DESC rootdesc(4, rootparas, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        ComPtr<ID3DBlob>rootsignature = nullptr;
        ComPtr<ID3DBlob>error = nullptr;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootdesc, D3D_ROOT_SIGNATURE_VERSION_1, rootsignature.GetAddressOf(), error.GetAddressOf()));
        assert(rootsignature != nullptr);
        ThrowIfFailed(pID3DDevice->CreateRootSignature(0, rootsignature->GetBufferPointer(), rootsignature->GetBufferSize(), IID_PPV_ARGS(&pIRootSignature)));
        assert(pIRootSignature != nullptr);
        //创建跟签名项
        auto rsRI = std::make_unique<RootSignatureItem>(pIRootSignature.Get(), 4, 1, 2, 0, 1, roottables);
        RootSignatureItemTable["default"] = std::move(rsRI);
    }
}
void FirstAPP::createCommandList() {
    {
        ThrowIfFailed(pID3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pIcmdallpre)));
        ThrowIfFailed(pID3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pIcmdallpost)));
        ThrowIfFailed(pID3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pIcmdallpre.Get(), pIPipelineState.Get(), IID_PPV_ARGS(&pIcmdlistpre)));
        ThrowIfFailed(pID3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pIcmdallpost.Get(), pIPipelineState.Get(), IID_PPV_ARGS(&pIcmdlistpost)));
    }
}
void FirstAPP::compileShadersAndCreatePSO() {
    {
        ComPtr<ID3DBlob>vsshader = nullptr;
        ComPtr<ID3DBlob>psshader = nullptr;

        BYTE* cbvmapped = nullptr;
#if defined(_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif
        compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

        vsshader = d3dUtil::CompileShader(L"Shaders//drawPlane.hlsl", nullptr, "VS", "vs_5_0");
        psshader = d3dUtil::CompileShader(L"Shaders//drawPlane.hlsl", nullptr, "PS", "ps_5_0");
        D3D12_INPUT_ELEMENT_DESC inputdesc[] = {
            {"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"AOk",0,DXGI_FORMAT_R32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };
        //创建PSO
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psodesc = {};
        psodesc.InputLayout = { inputdesc,_countof(inputdesc) };
        psodesc.pRootSignature = pIRootSignature.Get();
        psodesc.VS = { reinterpret_cast<BYTE*>(vsshader->GetBufferPointer()),vsshader->GetBufferSize() };
        psodesc.PS = { reinterpret_cast<BYTE*>(psshader->GetBufferPointer()),psshader->GetBufferSize() };
        psodesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        psodesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psodesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        psodesc.BlendState.AlphaToCoverageEnable = FALSE;
        psodesc.BlendState.IndependentBlendEnable = FALSE;
        psodesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        psodesc.DepthStencilState.DepthEnable = TRUE;
        psodesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psodesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        psodesc.DepthStencilState.StencilEnable = FALSE;
        psodesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psodesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psodesc.NumRenderTargets = 1;
        psodesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psodesc.SampleMask = UINT_MAX;
        psodesc.SampleDesc.Count = 1;
        psodesc.SampleDesc.Quality = 0;
        ThrowIfFailed(pID3DDevice->CreateGraphicsPipelineState(&psodesc, IID_PPV_ARGS(&pIPipelineState)));
        PSOTable["default"] = pIPipelineState;
    }
}
void FirstAPP::createMemoryManager() {
  auto RTDSsfl= std::make_unique< RT_DS_TextureSegregatedFreeLists> (3200, 4, pID3DDevice.Get());
  RTDSSFLTable["RTDS"] = std::move(RTDSsfl);

  auto upBS = std::make_unique<uploadBuddySystem>(1024, 5, pID3DDevice.Get());//最小64KB对齐,也就是最精细的buddy最小是64KB及其倍数
  upBSTable["default"] = std::move(upBS);

  auto defBS = std::make_unique<defaultBuddySystem>(1024, 5, pID3DDevice.Get());
  defBSTable["default"] = std::move(defBS);
}
void FirstAPP::createDepthBufferAndSampler() {
    //创建深度缓冲及其资源项
    D3D12_RESOURCE_DESC dsdesc = {};
    dsdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    dsdesc.Alignment = 0;
    dsdesc.Width = iWidth;
    dsdesc.Height = iHeight;
    dsdesc.DepthOrArraySize = 1;
    dsdesc.MipLevels = 1;
    dsdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsdesc.SampleDesc.Count = 1;
    dsdesc.SampleDesc.Quality = 0;
    dsdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dsdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvdesc = {};
    dsvdesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvdesc.Texture2D.MipSlice = 0;
    D3D12_CLEAR_VALUE dsclear = {};
    dsclear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsclear.DepthStencil.Depth = 1.0f;
    dsclear.DepthStencil.Stencil = 0;
    auto DSRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, pIDSVHeap.Get(), false);
    DSRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &dsdesc, D3D12_HEAP_FLAG_NONE, &dsclear,RTDSSFLTable["RTDS"].get());
    DSRI->createDSVforResourceItem(&dsvdesc, pID3DDevice.Get(), HeapOffsetTable[pIDSVHeap.Get()], pIcmdlistpre.Get());
    TextureResourceItemTable["DS"] = std::move(DSRI);


    //创建一个默认采样器及其资源项
    D3D12_SAMPLER_DESC preSamplerDesc = {};
    preSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    preSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    preSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    preSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    preSamplerDesc.MipLODBias = 0;
    preSamplerDesc.MaxAnisotropy = 1;
    preSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    preSamplerDesc.MinLOD = 0.0f;
    preSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    auto SamplerRI = std::make_unique<SamplerResourceItem>(pID3DDevice.Get(), pIpresamheap.Get(), preSamplerDesc, HeapOffsetTable[pIpresamheap.Get()]);
    SamplerResourceItemTable["default"] = std::move(SamplerRI);
}
void FirstAPP::createGeometryItemAndRenderItem() {
    //顶点
    std::vector<Vertex>planev = {
{XMFLOAT4{-60.0f + 6,5,60.0f,1.0f},XMFLOAT2{0,.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
{XMFLOAT4{60.0f,5,60.0f,1.0f},XMFLOAT2{1.0f,.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
{XMFLOAT4{-60.0f + 6,5,-60.0f,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
{XMFLOAT4{60.0f,  5 ,-60.0f,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},

{XMFLOAT4{-60.0f + 6, 100.0f ,-60.0f,1.0f},XMFLOAT2{0,0},XMFLOAT3{1.0f,0,0},XMFLOAT3{0,0,-1.0f},0},
{XMFLOAT4{-60.0f + 6, 100.0f ,60.0f,1.0f},XMFLOAT2{1.0f,0},XMFLOAT3{1.0f,0,0},XMFLOAT3{0,0,-1.0f},0},
{XMFLOAT4{-60.0f + 6,5,-60.0f,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{1.0f,0,0},XMFLOAT3{0,0,-1.0f},0},
{XMFLOAT4{-60.0f + 6,5,60.0f,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{1.0f,0,0},XMFLOAT3{0,0,-1.0f},0},


 {XMFLOAT4{-60.0f + 6,100,60.0f,1.0f},XMFLOAT2{0,0},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{60.0f,100,60.0f,1.0f},XMFLOAT2{1.0f,.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{-60.0f + 6,0,60.0f,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{60.0f,0,60.0f,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0}
    };
    for (int i = 0;i < 4;i++) {
        planev[i].color = XMFLOAT3{ 1.0f,0,0 };
        planev[i].normal = XMFLOAT3{ 0,1.0f,0 };
    }
    for (int i = 4;i < 8;i++) {
        planev[i].color = XMFLOAT3{ 0,0,1.0f };
        planev[i].normal = XMFLOAT3{ 1.0f,0,0 };
    }
    for (int i = 8;i < 12;i++) {
        planev[i].color = XMFLOAT3{ 0.7f,0.7f,0.7f };
        planev[i].normal = XMFLOAT3{ 0,0,-1.0f };
    }
    std::vector<std::uint16_t>planei = {
        0,1,2,2,1,3,
        4,5,6,6,5,7,
        8,9,10,10,9,11
    };
    //创建几何项以及渲染项
    //几何项
    auto upBS = upBSTable["default"].get();
    auto defBS = defBSTable["default"].get();
    auto planeGeo = std::make_unique<GeometryItem>();
    planeGeo->createStaticGeo(pID3DDevice.Get(), pIcmdlistpre.Get(), &planev, &planei,upBS,defBS);
    GeometryItemTable["plane"] = std::move(planeGeo);
    //渲染项
    objectconstant objc;
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(XMMatrixTranslation(0, 0, 0)));
    auto planeRi = std::make_unique<RenderItem>(GeometryItemTable["plane"].get(), 1, 0, 0, planei.size(), 0, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS,defBS);
    RenderItemTable["plane0"] = std::move(planeRi);
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(XMMatrixTranslation(114, 0, 0)));
    planeRi = std::make_unique<RenderItem>(GeometryItemTable["plane"].get(), 1, 0, 0, planei.size(), 0, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    RenderItemTable["plane1"] = std::move(planeRi);
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(XMMatrixTranslation(228, 0, 0)));
    planeRi = std::make_unique<RenderItem>(GeometryItemTable["plane"].get(), 1, 0, 0, planei.size(), 0, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    RenderItemTable["plane2"] = std::move(planeRi);
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(XMMatrixTranslation(342, 0, 0)));
    planeRi = std::make_unique<RenderItem>(GeometryItemTable["plane"].get(), 1, 0, 0, planei.size(), 0, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    RenderItemTable["plane3"] = std::move(planeRi);
}
void FirstAPP::createResourceItem() {
    //创建纹理资源以及缓冲资源
     //创建资源项
    auto upBS = upBSTable["default"].get();
    auto defBS = defBSTable["default"].get();
    passconstant passc;
    auto passBRI = std::make_unique<BufferResourceItem<passconstant>>(pID3DDevice.Get(), pIcmdlistpre.Get(), &passc, false, pIpresrvheap.Get(), upBS, defBS);
    BufferResourceItemTable["pass"] = std::move(passBRI);

    auto grassTRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true);
    grassTRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\grass.dds");
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVdesc = {};
    SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SRVdesc.Texture2D.MostDetailedMip = 0;
    SRVdesc.Texture2D.ResourceMinLODClamp = 0.0f;
    SRVdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    grassTRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[grassTRI->SRVUAVHeap]);
    TextureResourceItemTable["grass"] = std::move(grassTRI);

    auto brickTRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true);
    brickTRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\bricks3.dds");
    brickTRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[brickTRI->SRVUAVHeap]);
    TextureResourceItemTable["brick"] = std::move(brickTRI);

    auto cbTRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true);
    cbTRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\checkboard.dds");
    cbTRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[cbTRI->SRVUAVHeap]);
    TextureResourceItemTable["checkboard"] = std::move(cbTRI);
}
int FirstAPP::createNewThreadToRender(std::string RenderItemName, std::string BufferResourceItemName, std::string TextureResourceItemName, std::string RootSignatureItemName, std::string SamplerResourceItemItemName, std::string PSOName,int order) {
    //order为当前创建线程在本次创建的多个线程中的序号（从0开始）
    int i;
    int dtlsize = deadThreadList.size();
    if(order+1>dtlsize)
        i = ThreadNum;
    else {
        i = deadThreadList[dtlsize - 1 - order];
    }
    RenderItemTable[RenderItemName].get()->threadNote = i;//为渲染项赋值它的线程标记，方便删除对应线程
    HandleMessage->clear();
    //reserve会改变内存地址，resize过程可能会调用reserve，所以
    paras[i].RenderItemName = RenderItemName;
    paras[i].BufferResourceItemName = BufferResourceItemName;
    paras[i].TextureResourceItemName = TextureResourceItemName;
    paras[i].RootSignatureItemName = RootSignatureItemName;
    paras[i].SamplerResourceItemItemName = SamplerResourceItemItemName;
    paras[i].PSOName = PSOName;
    paras[i].stViewPort = &stViewPort;
    paras[i].stScissorRect = &stScissorRect;
    paras[i].rs = pIRootSignature.Get();
    paras[i].device = pID3DDevice.Get();
    paras[i].dwMainThreadID = ::GetCurrentThreadId();
    paras[i].hMainThread = ::GetCurrentThread();
    paras[i].hEventRenderOver = ::CreateEvent(NULL, FALSE, NULL, FALSE);
    paras[i].hRunEvent = ::CreateEvent(NULL, FALSE, NULL, FALSE);
    ThrowIfFailed(pID3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&paras[i].cmdalloc)));
    ThrowIfFailed(pID3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, paras[i].cmdalloc, nullptr, IID_PPV_ARGS(&paras[i].cmdlist)));
    paras[i].cmdlist->Close();
    paras[i].nIndex = i;
    paras[i].RenderItemTable = &RenderItemTable;
    paras[i].GeometryItemTable = &GeometryItemTable;
    paras[i].TextureResourceItemTable = &TextureResourceItemTable;
    paras[i].BufferResourceItemTable = &BufferResourceItemTable;
    paras[i].SamplerResourceItemTable = &SamplerResourceItemTable;
    paras[i].RootSignatureItemTable = &RootSignatureItemTable;
    paras[i].PSOTable = &PSOTable;
    paras[i].nCurrentFrameIndex = &nFrameIndex;
    paras[i].RTVHeap = pIRTVHeap.Get();
    paras[i].PSO = pIPipelineState.Get();
    for (int ii = 0;ii < nFrameBackBufCount;ii++)
        paras[i].pIARenderTargets[ii] = pIARenderTargets[ii].Get();
    paras[i].hThisThread = (HANDLE)_beginthreadex(nullptr, 0, threadfunc, (void*)&paras[i], CREATE_SUSPENDED, (UINT*)&(paras[i].dwThisThreadID));
    HandleMessage->push_back(paras[i].hEventRenderOver);
    existThreadList.push_back(i);
    ThreadNum++;
    return i;
}
void FirstAPP::startAndWaitNewThread(int newThreadNum) {
    int deadNum = 0;
    int bornNum = 0;
    for (int i =0;i <newThreadNum;i++) {
        int threadNoteToStart;
        if (!deadThreadList.empty()) {//死亡列表非空所以复活死亡列表中线程
            deadNum++;
            threadNoteToStart = deadThreadList.back();
            deadThreadList.pop_back();
        }
        else {//没有死亡线程就创建新的线程
            threadNoteToStart = ThreadNum - newThreadNum + deadNum + bornNum;
            bornNum++;
        }
        ::ResumeThread(paras[threadNoteToStart].hThisThread);//线程~~启动！！
    }
    //等待线程初始化完成
    ::MsgWaitForMultipleObjects(HandleMessage->size(), HandleMessage->data(), TRUE, INFINITE, QS_ALLINPUT);
}
void FirstAPP::killThread(int threadNote) {
    existThreadList.erase(std::find(std::begin(existThreadList), std::end(existThreadList), threadNote));
    deadThreadList.push_back(threadNote);
    ::TerminateThread(
        paras[threadNote].hThisThread,NULL
    );
    ThreadNum--;
    //置零paras[threadNote]
}
void FirstAPP::startSubThread() {
    //线程初始化传参并启动
    HandleMessage->clear();
    paras.resize(20);
    createNewThreadToRender("plane0", "pass", "grass", "default", "default", "default",0);
    startAndWaitNewThread(1);
}
void FirstAPP::startRenderLoop() {
    try {

        {
            ThrowIfFailed(pID3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pIFence)));
            n64FenceValue = 1;
            hFenceEvent = CreateEvent(nullptr, false, false, nullptr);
            if (hFenceEvent == nullptr) {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
        DWORD dwRet = 0;
        BOOL bExit = FALSE;
        UINT nStates = 0; //初始状态为0
        UINT n64fence = 0;
        ThrowIfFailed(pIcmdlistpre->Close());
        //处理上传纹理以及buffer的指令
        cmdlists.push_back(pIcmdlistpre.Get());
        pICommandQueue->ExecuteCommandLists(cmdlists.size(), cmdlists.data());
        n64fence = n64FenceValue;
        n64FenceValue++;
        pICommandQueue->Signal(pIFence.Get(), n64fence);
        pIFence->SetEventOnCompletion(n64fence, hFenceEvent);
        HandleMessage->clear();
        HandleMessage->push_back(hFenceEvent);
        SetTimer(hWnd, WM_USER + 100, 1, nullptr);//waitfor需要消息来停止阻塞
        //主渲染循环
        while (!bExit)
        {
            if (nStates == 2) {//渲染完一帧再处理消息,不然一个state处理一次就会卡顿
                while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))//用if会卡顿
                {
                    if (WM_QUIT != msg.message)
                    {
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                    }
                    else
                    {
                        bExit = TRUE;
                    }
                }
            }
            dwRet = ::MsgWaitForMultipleObjects(HandleMessage->size(), HandleMessage->data(), TRUE, INFINITE, QS_ALLINPUT);//返回接受到事件在事件数组中索引,否则阻塞
            UINT a = (UINT)(dwRet - WAIT_OBJECT_0);
            if ((a) == 0)
            {
                switch (nStates) {
                case 0:
                {
                    ThrowIfFailed(pIcmdlistpre->Reset(pIcmdallpre.Get(), nullptr));
                    //预处理pass渲染（整个程序只需要渲染一次）
                    //··············
                    //··············
                    //··············
                    //处理指令
                    pIcmdlistpre->Close();
                    nStates = 1;
                    cmdlists.clear();
                    cmdlists.push_back(pIcmdlistpre.Get());
                    pICommandQueue->ExecuteCommandLists(cmdlists.size(), cmdlists.data());
                    n64fence = n64FenceValue;
                    n64FenceValue++;
                    pICommandQueue->Signal(pIFence.Get(), n64fence);
                    pIFence->SetEventOnCompletion(n64fence, hFenceEvent);
                    nStates = 1;
                    HandleMessage->clear();
                    HandleMessage->push_back(hFenceEvent);

                }
                break;

                case 1://子线程和gpu都加载完了，现在就剩下让子线程开始绘制
                {

                    nFrame++;
                    float x = cos(theta) * sin(phi);
                    float z = sin(theta) * sin(phi);
                    float y = cos(phi);
                    nFrameIndex = pISwapChain3->GetCurrentBackBufferIndex();
                    currenttime = ::GetTickCount();
                    rotationAngle += (currenttime - starttime) * 0.003f;
                    float k = (currenttime - starttime) * 0.001;
                    timesecond += k;
                    if (timesecond >= 1.25f)
                        timesecond = 0;

                    starttime = currenttime;
                    XMVECTOR right = XMVector3Cross(up, XMVectorSet(x, y, z, 0)) * 10.0f;
                    XMVECTOR front;///////////////////////////

                    if (GetAsyncKeyState('W') & 0x8000)
                    {
                        WriteConsole(g_hOutput, "w", 2, NULL, NULL);
                        eyepos = eyepos + XMVectorSet(x, y, z, 0) * 1.5f;

                    }
                    if (GetAsyncKeyState('A') & 0x8000)
                    {
                        WriteConsole(g_hOutput, "a", 2, NULL, NULL);
                        eyepos = eyepos - right * k*7;

                    }
                    if (GetAsyncKeyState('S') & 0x8000)
                    {
                        WriteConsole(g_hOutput, "s", 2, NULL, NULL);
                        eyepos = eyepos - XMVectorSet(x, y, z, 0) * 1.5f;
                    }
                    if (GetAsyncKeyState('D') & 0x8000)
                    {
                        WriteConsole(g_hOutput, "d", 2, NULL, NULL);
                        eyepos = eyepos + right * k*7;

                    }
                    if (rotationAngle >= XM_2PI)
                        rotationAngle = fmod(rotationAngle, XM_2PI);
                    XMMATRIX Viewmat = XMMatrixLookAtLH(eyepos, eyepos + XMVectorSet(x, y, z, 0), up);
                    XMMATRIX projmat = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)1 / (FLOAT)1, 0.1f, 1000.0f);
                    XMMATRIX vpmat = XMMatrixMultiply(Viewmat, projmat);
                    XMMATRIX invP = XMMatrixInverse(nullptr, projmat);
                    XMStoreFloat3(&currpasscb.eyepos, eyepos);
                    XMStoreFloat4x4(&currpasscb.V, XMMatrixTranspose(Viewmat));
                    XMStoreFloat4x4(&currpasscb.VP, XMMatrixTranspose(vpmat));
                    XMStoreFloat4x4(&currpasscb.P, XMMatrixTranspose(projmat));
                    XMStoreFloat4x4(&currpasscb.invP, XMMatrixTranspose(invP));
                    currpasscb.lightdir = XMFLOAT3{ 1.0f,1.0f,.0f };
                    currpasscb.AL = XMFLOAT3{ 50.5f,50.5f,50.5f };
                    currpasscb.BL = XMFLOAT3{ 1.0f,1.0f,1.0f };
                    //每帧都变化的CBmemcpy

                    BufferResourceItemTable["pass"]->updateCB(&currpasscb);

                    //每帧都变化的点
                    // ·······
                    // ·······
                    // ·······
                    //这一帧是否增加新的渲染线程或删除线程
                    if (nFrame == 200) {
                        createNewThreadToRender("plane2", "pass", "checkboard", "default", "default", "default",0);//create和kill都是针对渲染项的函数
                        startAndWaitNewThread(1);
                    }
                    if (nFrame == 300) {
                        createNewThreadToRender("plane1", "pass", "brick", "default", "default", "default",0);
                        startAndWaitNewThread(1);
                    }
                    if (nFrame == 400) {
                        killThread(RenderItemTable["plane2"].get()->threadNote);
                    }
                    if (nFrame == 425) {
                        createNewThreadToRender("plane2", "pass", "checkboard", "default", "default", "default",0);
                        startAndWaitNewThread(1);
                    }
                    if (nFrame == 450) {
                        killThread(RenderItemTable["plane2"].get()->threadNote);
                    }
                    if (nFrame == 500) {
                        killThread(RenderItemTable["plane0"].get()->threadNote);
                        killThread(RenderItemTable["plane1"].get()->threadNote);
                    }
                    if (nFrame == 550) {
                        createNewThreadToRender("plane2", "pass", "checkboard", "default", "default", "default",0);
                        createNewThreadToRender("plane1", "pass", "brick", "default", "default", "default",1);
                        createNewThreadToRender("plane0", "pass", "grass", "default", "default", "default",2);
                        startAndWaitNewThread(3);
                    }
                    if (nFrame == 600) {
                        killThread(RenderItemTable["plane0"].get()->threadNote);
                        createNewThreadToRender("plane0", "pass", "grass", "default", "default", "default", 0);
                        createNewThreadToRender("plane3", "pass", "grass", "default", "default", "default", 1);
                        startAndWaitNewThread(2);
                    }
                    ThrowIfFailed(pIcmdlistpre->Reset(pIcmdallpre.Get(), nullptr));
                    //每帧都要渲染的预处理pass(变化的纹理)
                    // ············
                    // ············
                    // ············
                    //cmdlistpre传入命令队列进行处理
                    pIcmdlistpre->Close();
                    nStates = 2;
                    cmdlists.clear();
                    cmdlists.push_back(pIcmdlistpre.Get());
                    pICommandQueue->ExecuteCommandLists(cmdlists.size(), cmdlists.data());
                    n64fence = n64FenceValue;
                    n64FenceValue++;
                    pICommandQueue->Signal(pIFence.Get(), n64fence);
                    pIFence->SetEventOnCompletion(n64fence, hFenceEvent);
                    ::MsgWaitForMultipleObjects(1, &hFenceEvent, TRUE, INFINITE, QS_ALLINPUT);
                    HandleMessage->clear();
                    for (int i = 0;i < ThreadNum;i++)
                        HandleMessage->push_back(paras[existThreadList[i]].hEventRenderOver);
                    for (int i = 0;i < ThreadNum;i++)
                        SetEvent(paras[existThreadList[i]].hRunEvent);//通知子线程开始将渲染命令计入cmdlist

                }
                break;
                case 2: {
                    //线程渲染结束，进行后处理
                    ThrowIfFailed(pIcmdallpre->Reset());
                    ThrowIfFailed(pIcmdlistpre->Reset(pIcmdallpre.Get(), nullptr));
                    if (nFrame != 1) {
                        ThrowIfFailed(pIcmdallpost->Reset());
                        ThrowIfFailed(pIcmdlistpost->Reset(pIcmdallpost.Get(), nullptr));
                    }
                    CD3DX12_CPU_DESCRIPTOR_HANDLE stRTVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(pIRTVHeap->GetCPUDescriptorHandleForHeapStart(), nFrameIndex, pID3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
                    pIcmdlistpre->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition
                    (pIARenderTargets[nFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));//交换链资源在创建完后可以直接由present转换到rendertarget，我猜应该是common状态
                    pIcmdlistpre->ClearRenderTargetView(stRTVHandle, Colors::AliceBlue, 0, nullptr);
                    pIcmdlistpre->ClearDepthStencilView(TextureResourceItemTable["DS"].get()->getDSVCPU(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                    pIcmdlistpre->Close();
                    pIcmdlistpost->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition
                    (pIARenderTargets[nFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
                    pIcmdlistpost->Close();
                    cmdlists.clear();
                    cmdlists.push_back(pIcmdlistpre.Get());
                    for (int i = 0;i < ThreadNum;i++)
                        cmdlists.push_back(paras[existThreadList[i]].cmdlist);
                    cmdlists.push_back(pIcmdlistpost.Get());
                    pICommandQueue->ExecuteCommandLists(cmdlists.size(), cmdlists.data());
                    ThrowIfFailed(pISwapChain3->Present(1, 0));
                    n64fence = n64FenceValue;
                    n64FenceValue++;
                    pICommandQueue->Signal(pIFence.Get(), n64fence);
                    pIFence->SetEventOnCompletion(n64fence, hFenceEvent);
                    nStates = 1;
                    HandleMessage->clear();
                    HandleMessage->push_back(hFenceEvent);

                } break;
                default: {

                }
                       break;
                }
            }
        }
    }
    catch (DxException& e)
    {//发生了COM异常

        WriteConsole(g_hOutput, e.ToString().c_str(), e.ToString().size(), NULL, NULL);
        while (1);
    }
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    FirstAPP myapp;
    myapp.init(hInstance, nCmdShow);
    myapp.startRender();

return 0;
}

UINT __stdcall threadfunc(void* ppara) {
    threadparas* para = reinterpret_cast<threadparas*>(ppara);
    //线程的预处理操作
    //·········
    //·········
    //·········
    //while (1);
    SetEvent(para->hEventRenderOver);
    DWORD dwRet = 0;
    BOOL  bQuit = FALSE;
    MSG   msg = {};
    while (!bQuit) {
        dwRet = ::MsgWaitForMultipleObjects(1, &para->hRunEvent, FALSE, INFINITE, QS_ALLPOSTMESSAGE);
        switch (dwRet - WAIT_OBJECT_0) {
        case 0:
        {
            RootSignatureItem* RSI = (*para->RootSignatureItemTable)[para->RootSignatureItemName].get();
            RenderItem* RI = (*para->RenderItemTable)[para->RenderItemName].get();
            auto BRI = (*para->BufferResourceItemTable)[para->BufferResourceItemName].get();
            SamplerResourceItem* SRI = (*para->SamplerResourceItemTable)[para->SamplerResourceItemItemName].get();
            TextureResourceItem* TRI = (*(para->TextureResourceItemTable))[para->TextureResourceItemName].get();
            ID3D12PipelineState* PSO = (*para->PSOTable)[para->PSOName].Get();
            para->cmdalloc->Reset();
            para->cmdlist->Reset(para->cmdalloc, PSO);
            //必须每个子线程都setRT，视口以及裁剪矩形
            CD3DX12_CPU_DESCRIPTOR_HANDLE stRTVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(para->RTVHeap->GetCPUDescriptorHandleForHeapStart(), *para->nCurrentFrameIndex, para->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
            para->cmdlist->OMSetRenderTargets(1, &stRTVHandle, true,&(*para->TextureResourceItemTable)["DS"]->getDSVCPU());
            para->cmdlist->RSSetViewports(1, para->stViewPort);
            para->cmdlist->RSSetScissorRects(1, para->stScissorRect);
            para->cmdlist->SetGraphicsRootSignature(RSI->rs);
            para->cmdlist->SetPipelineState(PSO);
            ID3D12DescriptorHeap* heaps[2] = {BRI->CBVHeap,SRI->samplerHeap};
            para->cmdlist->SetDescriptorHeaps(2, heaps);
            CD3DX12_GPU_DESCRIPTOR_HANDLE gpuhandle = TRI->getSRVGPU(0);
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(0), gpuhandle);
            gpuhandle = SRI->getSampler();
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSamplerTableIndex(0), gpuhandle);
            gpuhandle = BRI->getCBVGPU();
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getCBVTableIndex(1), gpuhandle);
            drawRenderItem(RI, para->cmdlist, RSI->getCBVTableIndex(0));
            para->cmdlist->Close();
            SetEvent(para->hEventRenderOver);
        }
        break;
        }
    }
    return 0;
}
