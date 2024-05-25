// d3d12practicelast.cpp : 定义应用程序的入口点。
//
#pragma once
#include"rayTracer.h"
#include"modelReader.h"
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
XMVECTOR eyepos = XMVectorSet(20.0f, 100.0f, -200.0f, 1.0f);
XMVECTOR target = XMVectorSet(0.0f, .0f, 1.0f, 0.0f);
XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
XMFLOAT2 mousepospre;
float theta = PI /2;
float phi = PI / 2;
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
        phi += dphi;
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
    std::unordered_map<std::string,std::unique_ptr< ConstantBufferResourceItem<passconstant>>>(*BufferResourceItemTable);
    std::unordered_map<std::string,std::unique_ptr< TextureResourceItem>>(*TextureResourceItemTable);
    std::unordered_map<std::string,std::unique_ptr< SamplerResourceItem>>(*SamplerResourceItemTable);
    std::unordered_map<std::string,std::unique_ptr< RootSignatureItem>>(*RootSignatureItemTable);
    std::unordered_map<std::string, std::unique_ptr< PSOItem>>(*PSOITable);
    std::unordered_map<std::string, std::unique_ptr< StructureBufferResourceItem<AABBbox>>>(*boxTable);
    std::unordered_map<std::string, std::unique_ptr< StructureBufferResourceItem<triangle>>>(*triangleTable);
    std::unique_ptr < StructureBufferResourceItem<float>>*randomNumbers;
    std::unique_ptr < StructureBufferResourceItem<PolygonalLight>>*lights;
    std::unique_ptr<StructureBufferResourceItem<material>>*materials;
    ID3D12Device4* device;
    ID3D12RootSignature* rs;
    ID3D12GraphicsCommandList* cmdlist;
    ID3D12CommandAllocator* cmdalloc;
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
    void createVideoMemoryManager();
    void createDescriptorHeap();
    void createCommandQueueAndSwapChain();
    void createRootSignature();
    void createCommandList();
    void compileShadersAndCreatePSO();
    void createDepthBufferAndSampler();
    void createGeometryItemAndBVHData();
    void createRenderItem();
    void createResourceItem();
    void startSubThread();
    void startRenderLoop();
    int createNewThreadToRender(std::string RenderItemName, std::string BufferResourceItemName, std::string TextureResourceItemName, std::string RootSignatureItemName, std::string SamplerResourceItemItemName, std::string PSOName,int order);
    void startAndWaitNewThread(int newThreadNum);
    void killThread(int ThreadNote);
private:


    D3D12_RECT							stScissorRect = { 0, 0, static_cast<LONG>(iWidth), static_cast<LONG>(iHeight) };
    D3D12_RECT							stScissorRectwpos = { 0, 0, static_cast<LONG>(2048), static_cast<LONG>(1536) };
    ATOM                MyRegisterClass(HINSTANCE hInstance);
    BOOL                InitInstance(HINSTANCE, int, HWND, UINT, UINT);
    HINSTANCE hInst;                                // 当前实例
    WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
    WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
    TCHAR								pszAppPath[MAX_PATH] = {};
    MSG  msg = {};
    D3D12_VIEWPORT stViewPort = { 0.0f, 0.0f, static_cast<float>(iWidth), static_cast<float>(iHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
    D3D12_VIEWPORT stViewPortwpos = { 0.0f, 0.0f, static_cast<float>(2048), static_cast<float>(1536), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
    // 此代码模块中包含的

    UINT nFrameIndex = 0;
    UINT nFrame = 0;
  
    UINT nRTVDescriptorSize = 0U;
    EST::vector<threadparas> paras;
    EST::vector<int> existThreadList;
    EST::vector<int> deadThreadList;

    EST::vector<HANDLE> HandleMessage[4];
    EST::vector<ID3D12CommandList*>cmdlists;
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
    modelReader mreader;

    std::unordered_map<std::string, std::unique_ptr< RenderItem>>RenderItemTable;
    std::unordered_map<std::string, std::unique_ptr< GeometryItem>>GeometryItemTable;
    std::unordered_map<std::string, std::unique_ptr< ConstantBufferResourceItem<passconstant>>>BufferResourceItemTable;
    std::unordered_map<std::string, std::unique_ptr< TextureResourceItem>>TextureResourceItemTable;
    std::unordered_map<std::string, std::unique_ptr< SamplerResourceItem>>SamplerResourceItemTable;
    std::unordered_map<std::string, std::unique_ptr< RootSignatureItem>>RootSignatureItemTable;
    std::unordered_map<std::string, std::unique_ptr< RT_DS_TextureSegregatedFreeLists>>RTDSSFLTable;
    std::unordered_map<std::string, std::unique_ptr< NON_RT_DS_TextureSegregatedFreeLists>>NONRTDSSFLTable;
    std::unordered_map<std::string, std::unique_ptr< uploadBuddySystem>>upBSTable;
    std::unordered_map<std::string, std::unique_ptr< defaultBuddySystem>>defBSTable;
    std::unordered_map<std::string, std::unique_ptr< PSOItem>>PSOITable;
    std::unordered_map<std::string, std::unique_ptr< computePSOItem>>computePSOITable;
    std::unordered_map<std::string, std::unique_ptr< StructureBufferResourceItem<AABBbox>>>(boxTable);
    std::unordered_map<std::string, std::unique_ptr< StructureBufferResourceItem<triangle>>>(triangleTable);
    std::unique_ptr < StructureBufferResourceItem<float>> randomNumbers;
    std::unique_ptr < StructureBufferResourceItem<XMINT2>> offsetArray[5];
    std::unique_ptr < StructureBufferResourceItem<float>> hSBRI;
    std::unique_ptr < StructureBufferResourceItem<PolygonalLight>> lights;
    std::unique_ptr<StructureBufferResourceItem<material>>materials;
    std::unique_ptr < ConstantBufferResourceItem <lastVPmat>> lastvpmatRI;
    XMMATRIX last6FramesVP[6];
    SAHtree sahtree;
    EST::vector<triangle>SceneTriangles;
    PolygonalLight l;
    EST::vector<Vertex>lightV;
};
void FirstAPP::init(HINSTANCE hInstance, int       nCmdShow) {
    initDX12(hInstance, WndProc, nCmdShow);
    createDescriptorHeap();
    createCommandQueueAndSwapChain();
    createRootSignature();
    createCommandList();
    compileShadersAndCreatePSO();
    createVideoMemoryManager();
    createDepthBufferAndSampler();
    createGeometryItemAndBVHData();
    createResourceItem();
    createRenderItem();
    startSubThread();
}
void FirstAPP::startRender() {

    startRenderLoop();
}
void FirstAPP::createDescriptorHeap() {
    //描述符堆及采样器堆
    D3D12_DESCRIPTOR_HEAP_DESC rtvheapdesc = {};
    rtvheapdesc.NumDescriptors = nFrameBackBufCount+10;//之后会加数量
    rtvheapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvheapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(pID3DDevice->CreateDescriptorHeap(&rtvheapdesc, IID_PPV_ARGS(&pIRTVHeap)));
    D3D12_DESCRIPTOR_HEAP_DESC presrvheapdesc = {};
    presrvheapdesc.NumDescriptors = 90;//先创建一个静态堆，之后等东西多了再加个动态管理器
    presrvheapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    presrvheapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    ThrowIfFailed(pID3DDevice->CreateDescriptorHeap(&presrvheapdesc, IID_PPV_ARGS(&pIpresrvheap)));
    presrvheapdesc.NumDescriptors = 1;
    presrvheapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    presrvheapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    ThrowIfFailed(pID3DDevice->CreateDescriptorHeap(&presrvheapdesc, IID_PPV_ARGS(&pIpresamheap)));
    presrvheapdesc.NumDescriptors = 3;
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
#define rsNum 12
        //描述符表
        CD3DX12_DESCRIPTOR_RANGE roottables[rsNum] = {};
        roottables[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        roottables[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8,0,0);
        roottables[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
        roottables[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
        roottables[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,1);
        roottables[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1,1);
        roottables[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2,1);
        roottables[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3,1);
        roottables[8].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4,1);
        roottables[9].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5,1);
        roottables[10].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6,1);
        roottables[11].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7,1);
        //跟参数
        CD3DX12_ROOT_PARAMETER rootparas[rsNum] = {};
        rootparas[0].InitAsDescriptorTable(1, &roottables[0], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[1].InitAsDescriptorTable(1, &roottables[1], D3D12_SHADER_VISIBILITY_PIXEL);
        rootparas[2].InitAsDescriptorTable(1, &roottables[2], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[3].InitAsDescriptorTable(1, &roottables[3], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[4].InitAsDescriptorTable(1, &roottables[4], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[5].InitAsDescriptorTable(1, &roottables[5], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[6].InitAsDescriptorTable(1, &roottables[6], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[7].InitAsDescriptorTable(1, &roottables[7], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[8].InitAsDescriptorTable(1, &roottables[8], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[9].InitAsDescriptorTable(1, &roottables[9], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[10].InitAsDescriptorTable(1, &roottables[10], D3D12_SHADER_VISIBILITY_ALL);
        rootparas[11].InitAsDescriptorTable(1, &roottables[11], D3D12_SHADER_VISIBILITY_ALL);



        CD3DX12_ROOT_SIGNATURE_DESC rootdesc(rsNum, rootparas, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        ComPtr<ID3DBlob>rootsignature = nullptr;
        ComPtr<ID3DBlob>error = nullptr;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootdesc, D3D_ROOT_SIGNATURE_VERSION_1, rootsignature.GetAddressOf(), error.GetAddressOf()));
        assert(rootsignature != nullptr);
        ThrowIfFailed(pID3DDevice->CreateRootSignature(0, rootsignature->GetBufferPointer(), rootsignature->GetBufferSize(), IID_PPV_ARGS(&pIRootSignature)));
        assert(pIRootSignature != nullptr);
#define csrsnum 17
        //CS跟签名
        CD3DX12_DESCRIPTOR_RANGE csroottables[csrsnum] = {};
        csroottables[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
        csroottables[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        csroottables[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
        csroottables[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
        csroottables[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
        csroottables[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
        csroottables[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
        csroottables[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        csroottables[8].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
        csroottables[9].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
        csroottables[10].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
        csroottables[11].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);
        csroottables[12].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
        csroottables[13].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);
        csroottables[14].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5);
        csroottables[15].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 6);
        csroottables[16].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);

        //跟参数
        CD3DX12_ROOT_PARAMETER csrootparas[csrsnum] = {};
        csrootparas[0].InitAsDescriptorTable(1, &csroottables[0], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[1].InitAsDescriptorTable(1, &csroottables[1], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[2].InitAsDescriptorTable(1, &csroottables[2], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[3].InitAsDescriptorTable(1, &csroottables[3], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[4].InitAsDescriptorTable(1, &csroottables[4], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[5].InitAsDescriptorTable(1, &csroottables[5], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[6].InitAsDescriptorTable(1, &csroottables[6], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[7].InitAsDescriptorTable(1, &csroottables[7], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[8].InitAsDescriptorTable(1, &csroottables[8], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[9].InitAsDescriptorTable(1, &csroottables[9], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[10].InitAsDescriptorTable(1, &csroottables[10], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[11].InitAsDescriptorTable(1, &csroottables[11], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[12].InitAsDescriptorTable(1, &csroottables[12], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[13].InitAsDescriptorTable(1, &csroottables[13], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[14].InitAsDescriptorTable(1, &csroottables[14], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[15].InitAsDescriptorTable(1, &csroottables[15], D3D12_SHADER_VISIBILITY_ALL);
        csrootparas[16].InitAsDescriptorTable(1, &csroottables[16], D3D12_SHADER_VISIBILITY_ALL);



        CD3DX12_ROOT_SIGNATURE_DESC csrootdesc(csrsnum, csrootparas, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        ComPtr<ID3DBlob>csrootsignature = nullptr;
        ComPtr<ID3DBlob>cserror = nullptr;
        ThrowIfFailed(D3D12SerializeRootSignature(&csrootdesc, D3D_ROOT_SIGNATURE_VERSION_1, csrootsignature.GetAddressOf(), cserror.GetAddressOf()));
        assert(csrootsignature != nullptr);
        ThrowIfFailed(pID3DDevice->CreateRootSignature(0, csrootsignature->GetBufferPointer(), csrootsignature->GetBufferSize(), IID_PPV_ARGS(&pICSRootSignature)));
        assert(pICSRootSignature != nullptr);
        //创建跟签名项
        auto rsRI = std::make_unique<RootSignatureItem>(pIRootSignature.Get(), rsNum, 3, 0, 1, 1, roottables);
        RootSignatureItemTable["default"] = std::move(rsRI);
        rsRI = std::make_unique<RootSignatureItem>(pICSRootSignature.Get(), csrsnum, 3, 0, 1, 1,csroottables);
        RootSignatureItemTable["cs"] = std::move(rsRI);
    }
}
void FirstAPP::createCommandList() {
    {
        ThrowIfFailed(pID3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pIcmdallpre)));
        ThrowIfFailed(pID3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pIcmdallpost)));
        ThrowIfFailed(pID3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pIcmdallpre.Get(), nullptr, IID_PPV_ARGS(&pIcmdlistpre)));
        ThrowIfFailed(pID3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pIcmdallpost.Get(), nullptr, IID_PPV_ARGS(&pIcmdlistpost)));
    }
}
void FirstAPP::compileShadersAndCreatePSO() {
    {
        BYTE* cbvmapped = nullptr;
#if defined(_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif
        compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
        D3D12_INPUT_ELEMENT_DESC inputdesc[] = {
            {"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"AOk",0,DXGI_FORMAT_R32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };
        D3D12_INPUT_ELEMENT_DESC modelinputdesc[] = {
{"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
 {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
 {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
 {"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
   {"AOk",0,DXGI_FORMAT_R32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
   {"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

 {"WEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
 {"BONEINDICES",0,DXGI_FORMAT_R8G8B8A8_UINT, 0, 80, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };
        //创建PSO
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psodesc = {};
        psodesc.InputLayout = { inputdesc,_countof(inputdesc) };
        psodesc.pRootSignature = pIRootSignature.Get();
        psodesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        psodesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
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
        auto PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//DirectLuminance.hlsl", pID3DDevice.Get(),nullptr);
        PSOITable["raytraceDirect"] = std::move(PSOI);
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//IndirectLightLuminance.hlsl", pID3DDevice.Get(), nullptr);
        PSOITable["raytraceIndirect"] = std::move(PSOI);
        PSOI= std::make_unique<PSOItem>(&psodesc, L"Shaders//returnColor.hlsl", pID3DDevice.Get(),nullptr);
        PSOITable["color"] = std::move(PSOI);
        psodesc.InputLayout = { modelinputdesc,_countof(modelinputdesc) };
        psodesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//drawModel.hlsl", pID3DDevice.Get(),nullptr);
        PSOITable["tex"] = std::move(PSOI);
        psodesc.InputLayout = { inputdesc,_countof(inputdesc) };
        psodesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//drawBox.hlsl", pID3DDevice.Get(),nullptr);
        PSOITable["box"] = std::move(PSOI);
        psodesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psodesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//wposPass.hlsl", pID3DDevice.Get(),nullptr);
        PSOITable["wpos"] = std::move(PSOI);
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//normalPass.hlsl", pID3DDevice.Get(),nullptr);
        PSOITable["normal"] = std::move(PSOI);
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//zPass.hlsl", pID3DDevice.Get(), nullptr);
        PSOITable["z"] = std::move(PSOI);
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//gzPass.hlsl", pID3DDevice.Get(), nullptr);
        PSOITable["gz"] = std::move(PSOI);
        psodesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//firstFramePass.hlsl", pID3DDevice.Get(),nullptr);
        PSOITable["firstFramePass"] = std::move(PSOI);
        PSOI = std::make_unique<PSOItem>(&psodesc, L"Shaders//modulatePass.hlsl", pID3DDevice.Get(), nullptr);
        PSOITable["modulatePass"] = std::move(PSOI);
        D3D12_COMPUTE_PIPELINE_STATE_DESC computePSOdesc = {};
        computePSOdesc.pRootSignature = pICSRootSignature.Get();
        computePSOdesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        auto computePSOI = std::make_unique<computePSOItem>(&computePSOdesc, L"Shaders//denoise.hlsl", pID3DDevice.Get(),nullptr);
        computePSOITable["denoise"] = std::move(computePSOI);
        computePSOI = std::make_unique<computePSOItem>(&computePSOdesc, L"Shaders//mixPass.hlsl", pID3DDevice.Get(),nullptr);
        computePSOITable["mixPass"] = std::move(computePSOI);
        computePSOI = std::make_unique<computePSOItem>(&computePSOdesc, L"Shaders//accumulationPass.hlsl", pID3DDevice.Get(), nullptr);
        computePSOITable["accumulationPass"] = std::move(computePSOI);
        computePSOI = std::make_unique<computePSOItem>(&computePSOdesc, L"Shaders//variancePass.hlsl", pID3DDevice.Get(), nullptr);
        computePSOITable["variancePass"] = std::move(computePSOI);
    }
}
void FirstAPP::createVideoMemoryManager() {
  auto RTDSsfl= std::make_unique< RT_DS_TextureSegregatedFreeLists> (GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  RTDSSFLTable["RTDS"] = std::move(RTDSsfl);
  RTDSsfl = std::make_unique< RT_DS_TextureSegregatedFreeLists>(GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  RTDSSFLTable["RTDS1"] = std::move(RTDSsfl);
  RTDSsfl = std::make_unique< RT_DS_TextureSegregatedFreeLists>(GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  RTDSSFLTable["RTDS2"] = std::move(RTDSsfl);
  RTDSsfl = std::make_unique< RT_DS_TextureSegregatedFreeLists>(GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  RTDSSFLTable["RTDS3"] = std::move(RTDSsfl);
  RTDSsfl = std::make_unique< RT_DS_TextureSegregatedFreeLists>(GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  RTDSSFLTable["RTDS4"] = std::move(RTDSsfl);

  auto NONRTDSsfl = std::make_unique< NON_RT_DS_TextureSegregatedFreeLists>(GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  NONRTDSSFLTable["NONRTDS"] = std::move(NONRTDSsfl);
   NONRTDSsfl = std::make_unique< NON_RT_DS_TextureSegregatedFreeLists>(GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  NONRTDSSFLTable["NONRTDS1"] = std::move(NONRTDSsfl);
  NONRTDSsfl = std::make_unique< NON_RT_DS_TextureSegregatedFreeLists>(GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  NONRTDSSFLTable["NONRTDS2"] = std::move(NONRTDSsfl);
  NONRTDSsfl = std::make_unique< NON_RT_DS_TextureSegregatedFreeLists>(GRS_UPPER(3200, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), 5, pID3DDevice.Get());
  NONRTDSSFLTable["NONRTDS3"] = std::move(NONRTDSsfl);

  auto upBS = std::make_unique<uploadBuddySystem>(1024*256, 8, pID3DDevice.Get());//最小64KB对齐,也就是最精细的buddy最小是64KB及其倍数
  upBSTable["UPBS"] = std::move(upBS);
   upBS = std::make_unique<uploadBuddySystem>(1024 * 256, 8, pID3DDevice.Get());//最小64KB对齐,也就是最精细的buddy最小是64KB及其倍数
  upBSTable["UPBS1"] = std::move(upBS);

  auto defBS = std::make_unique<defaultBuddySystem>(1024*256, 8, pID3DDevice.Get());
  defBSTable["DEFBS"] = std::move(defBS);

   defBS = std::make_unique<defaultBuddySystem>(1024 * 256, 8, pID3DDevice.Get());
  defBSTable["DEFBS1"] = std::move(defBS);
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
    auto DSRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, pIDSVHeap.Get(), false,-1);
    DSRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &dsdesc, &dsclear,RTDSSFLTable["RTDS3"].get());
    DSRI->createDSVforResourceItem(&dsvdesc, pID3DDevice.Get(), HeapOffsetTable[pIDSVHeap.Get()], pIcmdlistpre.Get());
    TextureResourceItemTable["DS"] = std::move(DSRI);
     DSRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, pIDSVHeap.Get(), false, -1);
    DSRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &dsdesc, &dsclear, RTDSSFLTable["RTDS3"].get());
    DSRI->createDSVforResourceItem(&dsvdesc, pID3DDevice.Get(), HeapOffsetTable[pIDSVHeap.Get()], pIcmdlistpre.Get());
    TextureResourceItemTable["DS1"] = std::move(DSRI);
    dsdesc.Width = 2048;
    dsdesc.Height = 1536;
    DSRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, pIDSVHeap.Get(), false, -1);
    DSRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &dsdesc, &dsclear, RTDSSFLTable["RTDS3"].get());
    DSRI->createDSVforResourceItem(&dsvdesc, pID3DDevice.Get(), HeapOffsetTable[pIDSVHeap.Get()], pIcmdlistpre.Get());
    TextureResourceItemTable["wposDS"] = std::move(DSRI);
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
void FirstAPP::createGeometryItemAndBVHData() {
    //模型顶点
    mreader.readFileHeader("C:\\mytinyrenderproj\\rayTracer\\rayTracer\\soldier.m3d");
    int istarr[6] = { 0,7230 * 3,11679 * 3,18258 * 3,22065 * 3,10000000000 };
    UINT isizearr[5] = { 7230 * 3 ,4449 * 3 ,6579 * 3 ,3807 * 3 ,442 * 3 };
    int ModelTexIndex = -1;
    int nextIndex = 0;
    for (int i = 0;i < mreader.mindices.size() / 333;i++) {
        triangle tri;
        if (i * 3 >= nextIndex) {
            ModelTexIndex++;
            nextIndex = istarr[ModelTexIndex + 1];
        }
        tri.texIndex = ModelTexIndex + 3;
        modelVertex v1 = mreader.mvertices[mreader.mindices[i * 3]];
        modelVertex v2 = mreader.mvertices[mreader.mindices[i * 3 + 1]];
        modelVertex v3 = mreader.mvertices[mreader.mindices[i * 3 + 2]];
        tri.pos1 = XMFLOAT3{v1.position.x,v1.position.y,v1.position.z};
        tri.pos2 = XMFLOAT3{ v2.position.x,v2.position.y,v2.position.z };
        tri.pos3 = XMFLOAT3{ v3.position.x,v3.position.y,v3.position.z };
        tri.n = mreader.mvertices[mreader.mindices[i * 3]].normal;
        tri.uv12 = XMFLOAT4{v1.texuv.x,v1.texuv.y, v2.texuv.x,v2.texuv.y};
        tri.uv3 = XMFLOAT2{ v3.texuv.x,v3.texuv.y };
        tri.color = v1.color;
        SceneTriangles.push_back(tri);
    }


    //房间顶点
    Vertex arrv[] = {
{XMFLOAT4{-60.0f + 6,5,60.0f,1.0f},XMFLOAT2{0,.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},//底
{XMFLOAT4{150.0f,5,60.0f,1.0f},XMFLOAT2{1.0f,.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
{XMFLOAT4{-60.0f + 6,5,-120.0f,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
{XMFLOAT4{150.0f,  5 ,-120.0f,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},

{XMFLOAT4{-60.0f + 6, 200.0f ,-120.0f,1.0f},XMFLOAT2{0,0},XMFLOAT3{1.0f,0,0},XMFLOAT3{0,0,-1.0f},0},//左
{XMFLOAT4{-60.0f + 6, 200.0f ,60.0f,1.0f},XMFLOAT2{1.0f,0},XMFLOAT3{1.0f,0,0},XMFLOAT3{0,0,-1.0f},0},
{XMFLOAT4{-60.0f + 6,5,-120.0f,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{1.0f,0,0},XMFLOAT3{0,0,-1.0f},0},
{XMFLOAT4{-60.0f + 6,5,60.0f,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{1.0f,0,0},XMFLOAT3{0,0,-1.0f},0},


 {XMFLOAT4{-60.0f + 6,200,60.0f,1.0f},XMFLOAT2{0,0},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},//前
  {XMFLOAT4{150.0f,200,60.0f,1.0f},XMFLOAT2{1.0f,.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{-60.0f + 6,0,60.0f,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{150.0f,0,60.0f,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},

   {XMFLOAT4{150.0f,200,60.0f,1.0f},XMFLOAT2{0,0},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},//右
  {XMFLOAT4{150.0f,200,-120.0f,1.0f},XMFLOAT2{1.0f,.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{150.0f,0,60.0f,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{150.0f,0,-120.0f,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},

     {XMFLOAT4{-60.0f,200,-120.0f,1.0f},XMFLOAT2{0,0},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},//上
  {XMFLOAT4{150.0f,200,-120.0f,1.0f},XMFLOAT2{1.0f,.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{-60.0f,200,60.0f,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
  {XMFLOAT4{150.0f,200,60.0f,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{0,1.0f,0},XMFLOAT3{1.0f,0,0},0},
    };
    EST::vector<Vertex>planev(arrv, 20);
    for (int i = 0;i < 4;i++) {
        planev[i].color = XMFLOAT3{ 0.4f,0.4f,0.4f };
        planev[i].normal = XMFLOAT3{ 0,1.0f,0 };
    }
    for (int i = 4;i < 8;i++) {
        planev[i].color = XMFLOAT3{ 0.4f,0,0 };
        planev[i].normal = XMFLOAT3{ 1.0f,0,0 };
    }
    for (int i = 8;i < 12;i++) {
        planev[i].color = XMFLOAT3{ 0.4f,0.4f,0.4f };
        planev[i].normal = XMFLOAT3{ 0,0,-1.0f };
    }
    for (int i = 12;i < 16;i++) {
        planev[i].color = XMFLOAT3{ 0,0.4f,0 };
        planev[i].normal = XMFLOAT3{ -1.0f,0,0 };
    }
    for (int i = 16;i < 20;i++) {
        planev[i].color = XMFLOAT3{ 0.4f,0.4f,0.4f };
        planev[i].normal = XMFLOAT3{ 0,-1.0f,0 };
    }
    std::uint16_t arri[] = {
         0,1,2,2,1,3,
         4,5,6,6,5,7,
         8,9,10,10,9,11,
         12,13,14,14,13,15,
         16,17,18,18,17,19
    };
    EST::vector<std::uint16_t>planei(arri, 30);
    AddTrianglesToScene(SceneTriangles, planev, planei,-1);
    sahtree.init(SceneTriangles);
    currpasscb.boxNum = sahtree.boxNum;

    //包围盒顶点
    EST::vector<Vertex>boxesv;
    for (int i = 0;i < sahtree.leafBox.size();i++) {
        XMFLOAT3 c = sahtree.leafBox[i].Center;
        XMFLOAT3 e = sahtree.leafBox[i].Extents;
        Vertex v, v1, v2, v3, v4, v5, v6, v7;
        v.position = XMFLOAT4{ c.x - e.x,c.y - e.y,c.z - e.z,1.0f };
        boxesv.push_back(v);
        v1.position = XMFLOAT4{ c.x - e.x,c.y - e.y,c.z + e.z,1.0f };
        boxesv.push_back(v1);
        v2.position = XMFLOAT4{ c.x + e.x,c.y - e.y,c.z - e.z,1.0f };
        boxesv.push_back(v2);
        v3.position = XMFLOAT4{ c.x + e.x,c.y - e.y,c.z + e.z,1.0f };
        boxesv.push_back(v3);
        v4.position = XMFLOAT4{ c.x - e.x,c.y + e.y,c.z - e.z,1.0f };
        boxesv.push_back(v4);
        v5.position = XMFLOAT4{ c.x - e.x,c.y + e.y,c.z + e.z,1.0f };
        boxesv.push_back(v5);
        v6.position = XMFLOAT4{ c.x + e.x,c.y + e.y,c.z - e.z,1.0f };
        boxesv.push_back(v6);
        v7.position = XMFLOAT4{ c.x + e.x,c.y + e.y,c.z + e.z,1.0f };
        boxesv.push_back(v7);

    }
    EST::vector<std::uint16_t>bvhI;
    for (int i = 0;i < boxesv.size() / 8;i++) {
        bvhI.push_back(i * 8);
        bvhI.push_back(i * 8 + 1);
        bvhI.push_back(i * 8 + 1);
        bvhI.push_back(i * 8 + 3);
        bvhI.push_back(i * 8 + 3);
        bvhI.push_back(i * 8 + 2);
        bvhI.push_back(i * 8 + 2);
        bvhI.push_back(i * 8);
        bvhI.push_back(i * 8 + 4);
        bvhI.push_back(i * 8 + 5);
        bvhI.push_back(i * 8 + 5);
        bvhI.push_back(i * 8 + 7);

        bvhI.push_back(i * 8 + 7);
        bvhI.push_back(i * 8 + 6);
        bvhI.push_back(i * 8 + 6);
        bvhI.push_back(i * 8 + 4);
        bvhI.push_back(i * 8);
        bvhI.push_back(i * 8 + 4);
        bvhI.push_back(i * 8 + 1);
        bvhI.push_back(i * 8 + 5);
        bvhI.push_back(i * 8 + 2);
        bvhI.push_back(i * 8 + 6);
        bvhI.push_back(i * 8 + 3);
        bvhI.push_back(i * 8 + 7);

    }
    //面光源顶点
    Vertex Lightv[] = {
     {XMFLOAT4{0,199.5,-55,1.0f},XMFLOAT2{0,0},XMFLOAT3{0,-1.0f,0},XMFLOAT3{1.0f,0,0},0,XMFLOAT3{1.0,1.0,1.0}},
  {XMFLOAT4{50,199.5,-55,1.0f},XMFLOAT2{1.0f,.0f},XMFLOAT3{0,-1.0f,0},XMFLOAT3{1.0f,0,0},0,XMFLOAT3{1.0,1.0,1.0}},
  {XMFLOAT4{0,199.5,-5,1.0f},XMFLOAT2{0,1.0f},XMFLOAT3{0,-1.0f,0},XMFLOAT3{1.0f,0,0},0,XMFLOAT3{1.0,1.0,1.0}},
  {XMFLOAT4{50,199.5,-5,1.0f},XMFLOAT2{1.0f,1.0f},XMFLOAT3{0,-1.0f,0},XMFLOAT3{1.0f,0,0},0,XMFLOAT3{1.0,1.0,1.0}}
    };
    std::uint16_t Lighti[] = {
        0,1,2,2,1,3
    };
    EST::vector<Vertex>lightv(Lightv, 4);
    EST::vector<std::uint16_t>lighti(Lighti, 6);
    //康奈尔盒子顶点
    EST::vector<Vertex>cornellBoxv;
    EST::vector<std::uint16_t>cornellBoxi;
    CreateBox(30, 80, 30, cornellBoxv, cornellBoxi, XMFLOAT3{0.6f,0.6f,0.6f});
 
    //创建几何项以及渲染项
    //几何项
    auto upBS = upBSTable["UPBS"].get();
    auto defBS = defBSTable["DEFBS"].get();
    auto planeGeo = std::make_unique<GeometryItem>();
    planeGeo->createStaticGeo<Vertex>(pID3DDevice.Get(), pIcmdlistpre.Get(), &planev, &planei, upBS, defBS);
    GeometryItemTable["plane"] = std::move(planeGeo);
    auto modelGeo = std::make_unique<GeometryItem>();
    modelGeo->createStaticGeo<modelVertex>(pID3DDevice.Get(), pIcmdlistpre.Get(), &mreader.mvertices, &mreader.mindices, upBS, defBS);
    GeometryItemTable["model"] = std::move(modelGeo);
    auto boxGeo = std::make_unique<GeometryItem>();
    boxGeo->createStaticGeo<Vertex>(pID3DDevice.Get(), pIcmdlistpre.Get(), &boxesv, &bvhI, upBS, defBS);
    GeometryItemTable["box"] = std::move(boxGeo);
    auto LightGeo = std::make_unique<GeometryItem>();
    LightGeo->createDynamicGeo<Vertex>(pID3DDevice.Get(), &lightv, &lighti, upBS);
    GeometryItemTable["light"] = std::move(LightGeo);
    auto cornellBoxGeo = std::make_unique<GeometryItem>();
    cornellBoxGeo->createStaticGeo<Vertex>(pID3DDevice.Get(), pIcmdlistpre.Get(), &cornellBoxv, &cornellBoxi, upBS, defBS);
    GeometryItemTable["cornellBox"] = std::move(cornellBoxGeo);
}
void FirstAPP::createResourceItem() {
    //创建纹理资源以及缓冲资源
     //创建资源项
    auto upBS = upBSTable["UPBS1"].get();
    auto defBS = defBSTable["DEFBS1"].get();
    passconstant passc;
    auto passBRI = std::make_unique<ConstantBufferResourceItem<passconstant>>(pID3DDevice.Get(), pIcmdlistpre.Get(), &passc, false, pIpresrvheap.Get(), upBS, defBS);
    BufferResourceItemTable["pass"] = std::move(passBRI);
    lastVPmat lvp;
    lastvpmatRI= std::make_unique<ConstantBufferResourceItem<lastVPmat>>(pID3DDevice.Get(), pIcmdlistpre.Get(), &lvp, false, pIpresrvheap.Get(), upBS, defBS);
    std::unique_ptr<StructureBufferResourceItem<AABBbox>>boxRI;
    std::unique_ptr<StructureBufferResourceItem<triangle>>triangleRI;
    BuildBoxAndTriangleSBRI(&sahtree, boxRI, triangleRI, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    boxTable["model"] = std::move(boxRI);
    triangleTable["model"] = std::move(triangleRI);
    std::unique_ptr<StructureBufferResourceItem<float>>randomNumberRI;
    EST::vector<float>randnums;
    GenerateRandomNum(randnums, 1000);
    randomNumberRI = std::make_unique<StructureBufferResourceItem<float>>(pID3DDevice.Get(), pIcmdlistpre.Get(), randnums.Getdata(), true, pIpresrvheap.Get(), upBS, defBS, randnums.size());
    randomNumbers = std::move(randomNumberRI);
    std::unique_ptr<StructureBufferResourceItem<XMINT2>>offsetRI;
    EST::vector<XMINT2>offsets;
    GenerateOffsets(offsets);
    for (int i = 0;i < 5;i++) {
        offsetRI = std::make_unique<StructureBufferResourceItem<XMINT2>>(pID3DDevice.Get(), pIcmdlistpre.Get(), offsets.Getdata()+i*25, true, pIpresrvheap.Get(), upBS, defBS, 25);
        offsetArray[i] = std::move(offsetRI);
    }
    std::unique_ptr<StructureBufferResourceItem<float>>hRI;
    EST::vector<float>h;
    h.push_back(1.0f/16.0f);
    h.push_back(1.0f/4.0f);
    h.push_back(3.0f/8.0f);
    h.push_back(1.0f/4.0f);
    h.push_back(1.0f/16.0f);
    hRI = std::make_unique<StructureBufferResourceItem<float>>(pID3DDevice.Get(), pIcmdlistpre.Get(), h.Getdata(), true, pIpresrvheap.Get(), upBS, defBS,h.size() );
    hSBRI = std::move(hRI);
    std::unique_ptr<StructureBufferResourceItem<PolygonalLight>>lightRI;
    l.area = 2500.0f;
    l.color = XMFLOAT3{ 1.0,1.0,1.0 };
    l.Xstart = 0;l.Xend = 50;
    l.Zstart = -55;l.Zend = -5;
    l.normal = XMFLOAT3{ 0,-1.0f,0 };
    lightRI = std::make_unique<StructureBufferResourceItem<PolygonalLight>>(pID3DDevice.Get(), pIcmdlistpre.Get(), &l, false, pIpresrvheap.Get(), upBS, defBS, 1);
    lights = std::move(lightRI);

    EST::vector<material>mats;
    material mat1;
    mat1.albedo = XMFLOAT3{ 0.6f,0,0 };
    mat1.F0 = XMFLOAT3{ 1.0,1.0,1.0 };
    mat1.roughness = 0.4f;
    mats.push_back(mat1);
    auto matRI = std::make_unique<StructureBufferResourceItem<material>>(pID3DDevice.Get(), pIcmdlistpre.Get(), mats.Getdata(), false, pIpresrvheap.Get(), upBS, defBS, mats.size());
    materials = std::move(matRI);

    auto grassTRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true,0);
    grassTRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\grass.dds");
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVdesc = {};
    SRVdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SRVdesc.Texture2D.MostDetailedMip = 0;
    SRVdesc.Texture2D.ResourceMinLODClamp = 0.0f;
    SRVdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    grassTRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[grassTRI->SRVUAVHeap]);
    TextureResourceItemTable["grass"] = std::move(grassTRI);//众多纹理图

    auto brickTRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true,1);
    brickTRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\bricks3.dds");
    brickTRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[brickTRI->SRVUAVHeap]);
    TextureResourceItemTable["brick"] = std::move(brickTRI);

    auto cbTRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true,2);
    cbTRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\checkboard.dds");
    cbTRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[cbTRI->SRVUAVHeap]);
    TextureResourceItemTable["checkboard"] = std::move(cbTRI);

    auto modelRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true,3);
    modelRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\head_diff.dds");
    modelRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[modelRI->SRVUAVHeap]);
    TextureResourceItemTable["model0"] = std::move(modelRI);

    modelRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true,4);
    modelRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\jacket_diff.dds");
    modelRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[modelRI->SRVUAVHeap]);
    TextureResourceItemTable["model1"] = std::move(modelRI);

    modelRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true,5);
    modelRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\pants_diff.dds");
    modelRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[modelRI->SRVUAVHeap]);
    TextureResourceItemTable["model2"] = std::move(modelRI);

    modelRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true,6);
    modelRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\upBody_diff.dds");
    modelRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[modelRI->SRVUAVHeap]);
    TextureResourceItemTable["model3"] = std::move(modelRI);

    modelRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, true, 7);
    modelRI->createStickerTex(pID3DDevice.Get(), pIcmdlistpre.Get(), L"Textures\\head_diff.dds");
    modelRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[modelRI->SRVUAVHeap]);
    TextureResourceItemTable["model4"] = std::move(modelRI);

    auto filteredRI=std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr,false, NULL);
    D3D12_RESOURCE_DESC filteredTexDesc = {};
    filteredTexDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    filteredTexDesc.Alignment = 0;
    filteredTexDesc.Width = iWidth;
    filteredTexDesc.Height = iHeight;
    filteredTexDesc.DepthOrArraySize = 1;
    filteredTexDesc.MipLevels = 1;
    filteredTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    filteredTexDesc.SampleDesc.Count = 1;
    filteredTexDesc.SampleDesc.Quality = 0;
    filteredTexDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    filteredTexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS"].get());
    D3D12_UNORDERED_ACCESS_VIEW_DESC filteredTexUavDesc = {};
    filteredTexUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    filteredTexUavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    filteredTexUavDesc.Texture2D.MipSlice = 0;
    filteredTexUavDesc.Texture2D.PlaneSlice = 0;
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap],pIcmdlistpre.Get());
    TextureResourceItemTable["DirectFilteredtex"] = std::move(filteredRI);//降噪过程要用的纹理
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["DirectFilteredtex1"] = std::move(filteredRI);//降噪过程要用的纹理
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["IndirectFilteredtex"] = std::move(filteredRI);//降噪过程要用的纹理
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["IndirectFilteredtex1"] = std::move(filteredRI);//降噪过程要用的纹理
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["DirectMoment1tex"] = std::move(filteredRI);//moment1
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["DirectMoment2tex"] = std::move(filteredRI);//moment2
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["IndirectMoment1tex"] = std::move(filteredRI);//moment1
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["IndirectMoment2tex"] = std::move(filteredRI);//moment2
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS1"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["DirectVariancetex"] = std::move(filteredRI);//variance
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS1"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["IndirectVariancetex"] = std::move(filteredRI);//variance
    for (int i = 0;i < 6;i++) {
        filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
        filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS1"].get());
        filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
        filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
        TextureResourceItemTable["DirectHistorytex"+std::to_string(i)] = std::move(filteredRI);//historyTex
    }
    for (int i = 0;i < 6;i++) {
        filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
        filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS2"].get());
        filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
        filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
        TextureResourceItemTable["IndirectHistorytex" + std::to_string(i)] = std::move(filteredRI);//historyTex
    }
    for (int i = 1;i < 3;i++) {
        filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
        filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS2"].get());
        filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
        filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
        TextureResourceItemTable["DirectLastMoment" + std::to_string(i)] = std::move(filteredRI);//lastmomentTex
    }
    for (int i = 1;i < 3;i++) {
        filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
        filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS3"].get());
        filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
        filteredRI->createUAVforResourceItem(&filteredTexUavDesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap], pIcmdlistpre.Get());
        TextureResourceItemTable["IndirectLastMoment" + std::to_string(i)] = std::move(filteredRI);//lastmomentTex
    }
    filteredTexDesc.Width = 2048;
    filteredTexDesc.Height = 1536;
    filteredTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    filteredRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), nullptr, nullptr, false, NULL);
    filteredRI->createNON_RT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, NONRTDSSFLTable["NONRTDS3"].get());
    filteredRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[filteredRI->SRVUAVHeap]);
    TextureResourceItemTable["lastnormal"] = std::move(filteredRI);//lastNormalTex
    filteredTexDesc.Width = 1024;
    filteredTexDesc.Height = 768;
    filteredTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    for (int i = 0;i < nFrameBackBufCount;i++) {
        auto backBufferRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(),nullptr, nullptr, false, NULL);
        backBufferRI->setTextureToRI(pIARenderTargets[i]);
        backBufferRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[backBufferRI->SRVUAVHeap]);
        TextureResourceItemTable["backBuffer" + std::to_string(i)] = std::move(backBufferRI);//为每个backBuffer创建SRV
    }
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    filteredTexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    auto lastFrameBuffer = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), pIRTVHeap.Get(), nullptr, false, NULL);
    lastFrameBuffer->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc,nullptr, RTDSSFLTable["RTDS1"].get());
    lastFrameBuffer->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[lastFrameBuffer->SRVUAVHeap]);
    lastFrameBuffer->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[lastFrameBuffer->RTVHeap],pIcmdlistpre.Get());
    TextureResourceItemTable["DirectLastFrameBuffer"] = std::move(lastFrameBuffer);
     lastFrameBuffer = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), pIRTVHeap.Get(), nullptr, false, NULL);
    lastFrameBuffer->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, nullptr, RTDSSFLTable["RTDS1"].get());
    lastFrameBuffer->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[lastFrameBuffer->SRVUAVHeap]);
    lastFrameBuffer->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[lastFrameBuffer->RTVHeap], pIcmdlistpre.Get());
    TextureResourceItemTable["IndirectLastFrameBuffer"] = std::move(lastFrameBuffer);
    rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    filteredTexDesc.Width = 2048;
    filteredTexDesc.Height = 1536;
    filteredTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    auto wposTexRI= std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(),pIRTVHeap.Get(), nullptr, false, NULL);
    wposTexRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, nullptr, RTDSSFLTable["RTDS"].get());
    wposTexRI->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[pIRTVHeap.Get()], pIcmdlistpre.Get());
    wposTexRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[pIpresrvheap.Get()]);
    TextureResourceItemTable["wpos"] = std::move(wposTexRI);//wpos图

    auto normalTexRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), pIRTVHeap.Get(), nullptr, false, NULL);
    normalTexRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, nullptr, RTDSSFLTable["RTDS1"].get());
    normalTexRI->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[pIRTVHeap.Get()], pIcmdlistpre.Get());
    normalTexRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[pIpresrvheap.Get()]);
    TextureResourceItemTable["normal"] = std::move(normalTexRI);//normal图
     normalTexRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), pIRTVHeap.Get(), nullptr, false, NULL);
    normalTexRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, nullptr, RTDSSFLTable["RTDS2"].get());
    normalTexRI->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[pIRTVHeap.Get()], pIcmdlistpre.Get());
    normalTexRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[pIpresrvheap.Get()]);
    TextureResourceItemTable["z"] = std::move(normalTexRI);//z图
    normalTexRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), pIRTVHeap.Get(), nullptr, false, NULL);
    normalTexRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, nullptr, RTDSSFLTable["RTDS3"].get());
    normalTexRI->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[pIRTVHeap.Get()], pIcmdlistpre.Get());
    normalTexRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[pIpresrvheap.Get()]);
    TextureResourceItemTable["gz"] = std::move(normalTexRI);//gz图
    filteredTexDesc.Width = 1024;
    filteredTexDesc.Height = 768;
    filteredTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    auto DirectLRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), pIRTVHeap.Get(), nullptr, false, NULL);
    DirectLRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, nullptr, RTDSSFLTable["RTDS3"].get());
    DirectLRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[pIpresrvheap.Get()]);
    DirectLRI->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[pIRTVHeap.Get()], pIcmdlistpre.Get());
    TextureResourceItemTable["DirectL"] = std::move(DirectLRI);//directL图

    auto IndirectLRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), pIRTVHeap.Get(), nullptr, false, NULL);
    IndirectLRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, nullptr, RTDSSFLTable["RTDS3"].get());
    IndirectLRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[pIpresrvheap.Get()]);
    IndirectLRI->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[pIRTVHeap.Get()], pIcmdlistpre.Get());
    TextureResourceItemTable["IndirectL"] = std::move(IndirectLRI);//indirectL图
     IndirectLRI = std::make_unique<TextureResourceItem>(pID3DDevice.Get(), pIpresrvheap.Get(), pIRTVHeap.Get(), nullptr, false, NULL);
    IndirectLRI->createRT_DS_WritableTex(pID3DDevice.Get(), pIcmdlistpre.Get(), &filteredTexDesc, nullptr, RTDSSFLTable["RTDS4"].get());
    IndirectLRI->createSRVforResourceItem(&SRVdesc, pID3DDevice.Get(), HeapOffsetTable[pIpresrvheap.Get()]);
    IndirectLRI->createRTVforResourceItem(&rtvDesc, pID3DDevice.Get(), HeapOffsetTable[pIRTVHeap.Get()], pIcmdlistpre.Get());
    TextureResourceItemTable["modulatetex"] = std::move(IndirectLRI);//modulate图
}
void FirstAPP::createRenderItem() {
    auto upBS = upBSTable["UPBS"].get();
    auto defBS = defBSTable["DEFBS"].get();
    //渲染项
    objectconstant objc;
    XMMATRIX world = XMMatrixTranslation(0, 0, 0);
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objc.invTworld, (XMMatrixInverse( nullptr,world)));
    objc.texIndex = -1;
    auto Geo = GeometryItemTable["plane"].get();
    auto planeRi = std::make_unique<RenderItem>(Geo, 1, 0, 0, Geo->indexNum, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    RenderItemTable["plane0"] = std::move(planeRi);
    world = XMMatrixTranslation(20, 70, 0.0f);
    world = XMMatrixMultiply(world, XMMatrixRotationY(PI/3));
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objc.invTworld, (XMMatrixInverse(nullptr, world)));
    Geo = GeometryItemTable["cornellBox"].get();
    auto cornellBoxRi = std::make_unique<RenderItem>(Geo, 1, 0, 0, Geo->indexNum, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    RenderItemTable["cornellBox"] = std::move(cornellBoxRi);
    world = XMMatrixTranslation(80, 70, 0.0f);
    world = XMMatrixMultiply(world, XMMatrixRotationY(PI / 6));
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objc.invTworld, (XMMatrixInverse(nullptr, world)));
    Geo = GeometryItemTable["cornellBox"].get();
    cornellBoxRi = std::make_unique<RenderItem>(Geo, 1, 0, 0, Geo->indexNum, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    RenderItemTable["cornellBox1"] = std::move(cornellBoxRi);
    //XMStoreFloat4x4(&objc.world, XMMatrixTranspose(XMMatrixTranslation(114, 0, 0)));
    //objc.texIndex = TextureResourceItemTable["brick"].get()->TextureIndex;
    //planeRi = std::make_unique<RenderItem>(Geo, 1, 0, 0, Geo->indexNum, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    //RenderItemTable["plane1"] = std::move(planeRi);
    //objc.texIndex = TextureResourceItemTable["checkboard"].get()->TextureIndex;
    //XMStoreFloat4x4(&objc.world, XMMatrixTranspose(XMMatrixTranslation(228, 0, 0)));
    //planeRi = std::make_unique<RenderItem>(Geo, 1, 0, 0, Geo->indexNum, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    //RenderItemTable["plane2"] = std::move(planeRi);
    //objc.texIndex = TextureResourceItemTable["grass"].get()->TextureIndex;
    //XMStoreFloat4x4(&objc.world, XMMatrixTranspose(XMMatrixTranslation(342, 0, 0)));
    //planeRi = std::make_unique<RenderItem>(Geo, 1, 0, 0, Geo->indexNum, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    //RenderItemTable["plane3"] = std::move(planeRi);

 /*   world = XMMatrixTranslation(80, 0, 0.0f);
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objc.invTworld, (XMMatrixInverse(nullptr, world)));
    int istarr[5] = { 0,7230 * 3,11679 * 3,18258 * 3,22065 * 3 };
    UINT isizearr[5] = { 7230 * 3 ,4449 * 3 ,6579 * 3 ,3807 * 3 ,442 * 3 };
    Geo = GeometryItemTable["model"].get();
    for (int i = 0;i < 5;i++) {
        std::string name = "model";
        objc.texIndex= TextureResourceItemTable[name+std::to_string(i)].get()->TextureIndex;
        auto modelRi = std::make_unique<RenderItem>(Geo, 1, 0, istarr[i], isizearr[i], 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
        RenderItemTable[name+std::to_string(i)] = std::move(modelRi);
    }*/
    //Geo = GeometryItemTable["box"].get();
    //auto boxRi = std::make_unique<RenderItem>(Geo, 1, 0, 0, Geo->indexNum, 0, D3D_PRIMITIVE_TOPOLOGY_LINELIST,&objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    //RenderItemTable["box"] = std::move(boxRi);
    world = XMMatrixTranslation(0, 0, 0.0f);
    XMStoreFloat4x4(&objc.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objc.invTworld, (XMMatrixInverse(nullptr, world)));
    Geo = GeometryItemTable["light"].get();
    auto lightRI = std::make_unique<RenderItem>(Geo, 1, 0, 0, Geo->indexNum, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, &objc, pID3DDevice.Get(), pIcmdlistpre.Get(), pIpresrvheap.Get(), upBS, defBS);
    RenderItemTable["light"] = std::move(lightRI);
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
    paras[i].materials = &materials;
    paras[i].boxTable = &boxTable;
    paras[i].triangleTable = &triangleTable;
    paras[i].randomNumbers = &randomNumbers;
    paras[i].lights = &lights;
    paras[i].PSOITable = &PSOITable;
    paras[i].nCurrentFrameIndex = &nFrameIndex;
    paras[i].RTVHeap = pIRTVHeap.Get();
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
    ::MsgWaitForMultipleObjects(HandleMessage->size(), HandleMessage->Getdata(), TRUE, INFINITE, QS_ALLINPUT);
}
void FirstAPP::killThread(int threadNote) {
    existThreadList.erase(existThreadList.getbegin(), existThreadList.getend(),threadNote);
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
    createNewThreadToRender("plane0", "pass", "checkboard", "default", "default", "raytrace",0);
    //createNewThreadToRender("model0", "pass", "model1", "default", "default", "raytrace",1);
    //createNewThreadToRender("model1", "pass", "model2", "default", "default", "raytrace",2);
    //createNewThreadToRender("model2", "pass", "model3", "default", "default", "raytrace",3);
    //createNewThreadToRender("model3", "pass", "model4", "default", "default", "raytrace",4);
    //createNewThreadToRender("model4", "pass", "grass", "default", "default", "raytrace",5);
    //createNewThreadToRender("box", "pass", "grass", "default", "default", "box",6);
    createNewThreadToRender("light", "pass", "grass", "default", "default", "color",1);
    createNewThreadToRender("cornellBox", "pass", "grass", "default", "default", "raytrace",2);
    createNewThreadToRender("cornellBox1", "pass", "grass", "default", "default", "raytrace",3);
    startAndWaitNewThread(4);
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
        pICommandQueue->ExecuteCommandLists(cmdlists.size(), cmdlists.Getdata());
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
            dwRet = ::MsgWaitForMultipleObjects(HandleMessage->size(), HandleMessage->Getdata(), TRUE, INFINITE, QS_ALLINPUT);//返回接受到事件在事件数组中索引,否则阻塞
            UINT a = (UINT)(dwRet - WAIT_OBJECT_0);
            if ((a) == 0)
            {
                switch (nStates) {
                case 0:
                {
                    float x = cos(theta) * sin(phi);
                    float z = sin(theta) * sin(phi);
                    float y = cos(phi);
                    XMMATRIX Viewmat = XMMatrixLookAtLH(eyepos, eyepos + XMVectorSet(x, y, z, 0), up);
                    XMMATRIX projmat = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)1 / (FLOAT)1, 0.1f, 1000.0f);
                    XMMATRIX vpmat = XMMatrixMultiply(Viewmat, projmat);
                    XMMATRIX invP = XMMatrixInverse(nullptr, projmat);
                    XMStoreFloat3(&currpasscb.eyepos, eyepos);
                    XMStoreFloat4x4(&currpasscb.V, XMMatrixTranspose(Viewmat));
                    XMStoreFloat4x4(&currpasscb.VP, XMMatrixTranspose(vpmat));
                    XMStoreFloat4x4(&currpasscb.P, XMMatrixTranspose(projmat));
                    BufferResourceItemTable["pass"]->updateCB(&currpasscb);
                    lastVPmat lvp;
                    lvp.lastVP =XMMatrixTranspose( vpmat);
                    lvp.nframe = nFrame;
                    lastvpmatRI->updateCB(&lvp);
                    ThrowIfFailed(pIcmdlistpre->Reset(pIcmdallpre.Get(), nullptr));
                    //预处理pass渲染（整个程序只需要渲染一次）
   
                    auto lastDirectRI = TextureResourceItemTable["DirectLastFrameBuffer"].get();
                    auto lastIndirectRI = TextureResourceItemTable["IndirectLastFrameBuffer"].get();

                    ResourceBarrierTrans(lastDirectRI->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ,pIcmdlistpre.Get());
                    ResourceBarrierTrans(lastIndirectRI->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ,pIcmdlistpre.Get());
                    //处理指令
                    pIcmdlistpre->Close();
                    nStates = 1;
                    cmdlists.clear();
                    cmdlists.push_back(pIcmdlistpre.Get());
                    pICommandQueue->ExecuteCommandLists(cmdlists.size(), cmdlists.Getdata());
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
                    currpasscb.curTime = (UINT)currenttime;
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
                    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
                    {
                        WriteConsole(g_hOutput, "d", 2, NULL, NULL);
                        l.Xstart -= 3.0f;
                        l.Xend -= 3.0f;
                    }
                    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
                    {
                        WriteConsole(g_hOutput, "d", 2, NULL, NULL);
                        l.Xstart += 3.0f;
                        l.Xend += 3.0f;
                    }
                    if (GetAsyncKeyState(VK_UP) & 0x8000)
                    {
                        WriteConsole(g_hOutput, "d", 2, NULL, NULL);
                        l.Zstart += 3.0f;
                        l.Zend += 3.0f;
                    }
                    if (GetAsyncKeyState(VK_DOWN) & 0x8000)
                    {
                        WriteConsole(g_hOutput, "d", 2, NULL, NULL);
                        l.Zstart -= 3.0f;
                        l.Zend -= 3.0f;
                    }
                    if (rotationAngle >= XM_2PI)
                        rotationAngle = fmod(rotationAngle, XM_2PI);
                    XMMATRIX Viewmat = XMMatrixLookAtLH(eyepos, eyepos + XMVectorSet(x, y, z, 0), up);
                    XMMATRIX projmat = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)1 / (FLOAT)1, 0.1f, 1000.0f);
                    XMMATRIX vpmat = XMMatrixMultiply(Viewmat, projmat);
                    lastvpmat.lastVP = XMMatrixTranspose( vpmat);
                    lastvpmat.last6VP = XMMatrixTranspose(last6FramesVP[(nFrame) % 6]);
                    last6FramesVP[(nFrame - 1) % 6] = vpmat;
                    lastvpmat.nframe = nFrame;
                    XMMATRIX invP = XMMatrixInverse(nullptr, projmat);
                    XMStoreFloat3(&currpasscb.eyepos, eyepos);
                    XMStoreFloat4x4(&currpasscb.V, XMMatrixTranspose(Viewmat));
                    XMStoreFloat4x4(&currpasscb.VP, XMMatrixTranspose(vpmat));
                    XMStoreFloat4x4(&currpasscb.P, XMMatrixTranspose(projmat));
                    currpasscb.lightdir = XMFLOAT3{ 1.0f,1.0f,.0f };
                    currpasscb.AL = XMFLOAT3{ 50.5f,50.5f,50.5f };
                    currpasscb.BL = XMFLOAT3{ 1.0f,1.0f,1.0f };
                    currpasscb.lightdir = XMFLOAT3(1.0f, 1.0f, 0);
                    currpasscb.nFrame = nFrame;
                    //每帧都变化的CBmemcpy

                    BufferResourceItemTable["pass"]->updateCB(&currpasscb);
                    lights->updateCB(&l);
                    updateLight(GeometryItemTable["light"].get(), l);
                    //每帧都变化的点
                    // ·······
                    // ·······
                    // ·······
                    //这一帧是否增加新的渲染线程或删除线程
              
                    ThrowIfFailed(pIcmdlistpre->Reset(pIcmdallpre.Get(), nullptr));
                    //每帧都要渲染的预处理pass(变化的纹理,比如Gbuffer)
                    auto DSV = TextureResourceItemTable["wposDS"].get()->getDSVCPU();
                    auto RSI = RootSignatureItemTable["default"].get();
                    auto wposRI = TextureResourceItemTable["wpos"].get();
                    auto normalRI = TextureResourceItemTable["normal"].get();
                    auto SRI = SamplerResourceItemTable["default"].get();
                    auto passRI = BufferResourceItemTable["pass"].get();
                    auto zRI = TextureResourceItemTable["z"].get();
                    auto gzRI = TextureResourceItemTable["gz"].get();
                    ResourceBarrierTrans(wposRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET, pIcmdlistpre.Get());
                    ResourceBarrierTrans(normalRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET, pIcmdlistpre.Get());
                    ResourceBarrierTrans(zRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET, pIcmdlistpre.Get());
                    ResourceBarrierTrans(gzRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET, pIcmdlistpre.Get());
                    pIcmdlistpre->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                    pIcmdlistpre->OMSetRenderTargets(1,&wposRI->getRTVCPU(), true, &DSV);
                    pIcmdlistpre->RSSetViewports(1, &stViewPortwpos);
                    pIcmdlistpre->RSSetScissorRects(1, &stScissorRectwpos);
                    pIcmdlistpre->SetGraphicsRootSignature(RSI->rs);
                    pIcmdlistpre->SetPipelineState(PSOITable["wpos"].get()->PSO);
                    ID3D12DescriptorHeap* heaps[2] = { wposRI->SRVUAVHeap,SRI->samplerHeap };
                    pIcmdlistpre->SetDescriptorHeaps(2, heaps);
                    pIcmdlistpre->SetGraphicsRootDescriptorTable(RSI->getCBVTableIndex(1), passRI->getCBVGPU());
                    drawRenderItems(&RenderItemTable, pIcmdlistpre.Get(), RSI->getCBVTableIndex(0),nullptr);
                    pIcmdlistpre->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                    pIcmdlistpre->OMSetRenderTargets(1, &normalRI->getRTVCPU(), true, &DSV);
                    pIcmdlistpre->SetPipelineState(PSOITable["normal"].get()->PSO);
                    drawRenderItems(&RenderItemTable, pIcmdlistpre.Get(), RSI->getCBVTableIndex(0),nullptr);
                    pIcmdlistpre->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                    pIcmdlistpre->OMSetRenderTargets(1, &zRI->getRTVCPU(), true, &DSV);
                    pIcmdlistpre->SetPipelineState(PSOITable["z"].get()->PSO);
                    drawRenderItems(&RenderItemTable, pIcmdlistpre.Get(), RSI->getCBVTableIndex(0),nullptr);
                    pIcmdlistpre->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                    pIcmdlistpre->OMSetRenderTargets(1, &gzRI->getRTVCPU(), true, &DSV);
                    pIcmdlistpre->SetPipelineState(PSOITable["gz"].get()->PSO);
                    drawRenderItems(&RenderItemTable, pIcmdlistpre.Get(), RSI->getCBVTableIndex(0),nullptr);
                    ResourceBarrierTrans(wposRI->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpre.Get());
                    ResourceBarrierTrans(normalRI->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpre.Get());
                    ResourceBarrierTrans(zRI->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpre.Get());
                    ResourceBarrierTrans(gzRI->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpre.Get());
                    //cmdlistpre传入命令队列进行处理
                    pIcmdlistpre->Close();
                    nStates = 2;
                    cmdlists.clear();
                    cmdlists.push_back(pIcmdlistpre.Get());
                    pICommandQueue->ExecuteCommandLists(cmdlists.size(), cmdlists.Getdata());
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
                    auto DirectLRI = TextureResourceItemTable["DirectL"].get();
                    auto IndirectLRI = TextureResourceItemTable["IndirectL"].get();
                    CD3DX12_CPU_DESCRIPTOR_HANDLE stRTVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(pIRTVHeap->GetCPUDescriptorHandleForHeapStart(), nFrameIndex, pID3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
                    ResourceBarrierTrans(pIARenderTargets[nFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, pIcmdlistpre.Get());//交换链资源在创建完后可以直接由present转换到rendertarget，我猜应该是common状态
                    pIcmdlistpre->ClearRenderTargetView(stRTVHandle, Colors::AliceBlue, 0, nullptr);
                    pIcmdlistpre->ClearDepthStencilView(TextureResourceItemTable["DS"].get()->getDSVCPU(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                    pIcmdlistpre->ClearDepthStencilView(TextureResourceItemTable["DS1"].get()->getDSVCPU(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                    pIcmdlistpre->ClearRenderTargetView(DirectLRI->getRTVCPU(), Colors::Black, 0, nullptr);
                    pIcmdlistpre->ClearRenderTargetView(IndirectLRI->getRTVCPU(), Colors::Black, 0, nullptr);
                    pIcmdlistpre->Close();
                    //后处理pass
                    ResourceBarrierTrans(pIARenderTargets[nFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    pIcmdlistpost->SetComputeRootSignature(pICSRootSignature.Get());
                    auto currentBackBufferRI = TextureResourceItemTable["backBuffer" + std::to_string(nFrameIndex)].get();
                    auto wposTexRI = TextureResourceItemTable["wpos"].get();
                    auto normalTexRI = TextureResourceItemTable["normal"].get();
                    auto zTexRI = TextureResourceItemTable["z"].get();
                    auto gzTexRI = TextureResourceItemTable["gz"].get();
                    auto samplerRI = SamplerResourceItemTable["default"].get();
                    auto RSI = RootSignatureItemTable["cs"].get();
                    auto DirectFilteredTexRI = TextureResourceItemTable["DirectFilteredtex"].get();
                    auto IndirectFilteredTexRI = TextureResourceItemTable["IndirectFilteredtex"].get();
                    auto DirectFilteredTex1RI = TextureResourceItemTable["DirectFilteredtex1"].get();
                    auto IndirectFilteredTex1RI = TextureResourceItemTable["IndirectFilteredtex1"].get();
                    auto DirectLastFrameBufferRI = TextureResourceItemTable["DirectLastFrameBuffer"].get();
                    auto IndirectLastFrameBufferRI = TextureResourceItemTable["IndirectLastFrameBuffer"].get();
                    auto DirectMoment1Tex = TextureResourceItemTable["DirectMoment1tex"].get();
                    auto IndirectMoment1Tex = TextureResourceItemTable["IndirectMoment1tex"].get();
                    auto DirectMoment2Tex = TextureResourceItemTable["DirectMoment2tex"].get();
                    auto IndirectMoment2Tex = TextureResourceItemTable["IndirectMoment2tex"].get();
                    auto DirectVarianceTexRI = TextureResourceItemTable["DirectVariancetex"].get();
                    auto IndirectVarianceTexRI = TextureResourceItemTable["IndirectVariancetex"].get();
                    auto DirectlastM1 = TextureResourceItemTable["DirectLastMoment1"].get();
                    auto DirectlastM2 = TextureResourceItemTable["DirectLastMoment2"].get();
                    auto IndirectlastM1 = TextureResourceItemTable["IndirectLastMoment1"].get();
                    auto IndirectlastM2 = TextureResourceItemTable["IndirectLastMoment2"].get();
                    auto lastnormalRI = TextureResourceItemTable["lastnormal"].get();
                    ResourceBarrierTrans(DirectLRI->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectLRI->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    ID3D12DescriptorHeap* heaps[2] = { wposTexRI->SRVUAVHeap,samplerRI->samplerHeap};
                    pIcmdlistpost->SetDescriptorHeaps(2, heaps);
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), DirectLRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(1), wposTexRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(2), normalTexRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(5), zTexRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(6), gzTexRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(4), hSBRI.get()->getSRVGPU());
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(7), lastnormalRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSamplerTableIndex(0), samplerRI->getSampler());
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(0), DirectFilteredTexRI->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(1), DirectMoment1Tex->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(2), DirectMoment2Tex->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(5), DirectlastM1->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(6), DirectlastM2->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getCBVTableIndex(0), lastvpmatRI->getCBVGPU());
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(3), TextureResourceItemTable["DirectHistorytex"+std::to_string((nFrame-1)%6)].get()->getUAVGPU(0));
                    pIcmdlistpost->SetPipelineState(computePSOITable["accumulationPass"].get()->PSO);
                    pIcmdlistpost->Dispatch(64, 48, 1);//累计当前帧到momentTex和historyTex
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), IndirectLRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(1), IndirectMoment1Tex->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(2), IndirectMoment2Tex->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(5), IndirectlastM1->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(6), IndirectlastM2->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(3), TextureResourceItemTable["IndirectHistorytex" + std::to_string((nFrame - 1) % 6)].get()->getUAVGPU(0));
                    pIcmdlistpost->Dispatch(64, 48, 1);//累计当前帧到momentTex和historyTex
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(4), IndirectVarianceTexRI->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(3), offsetArray[0].get()->getSRVGPU());
                    pIcmdlistpost->SetPipelineState(computePSOITable["variancePass"].get()->PSO);
                    pIcmdlistpost->Dispatch(64, 48, 1);//计算当前帧的variance作为filter的输入
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(1), DirectMoment1Tex->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(2), DirectMoment2Tex->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(4), DirectVarianceTexRI->getUAVGPU(0));
                    pIcmdlistpost->Dispatch(64, 48, 1);//计算当前帧的variance作为filter的输入
                    ResourceBarrierTrans(DirectFilteredTex1RI->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), DirectLRI->getSRVGPU(0));
                    pIcmdlistpost->SetPipelineState(computePSOITable["denoise"].get()->PSO);
                    for (int i = 0;i < 5;i++) {
                        pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(3), offsetArray[i].get()->getSRVGPU());
                        pIcmdlistpost->Dispatch(64, 48, 1);//降噪！
                        if (i == 4)
                            break;
                        if (i % 2 == 0) {
                            ResourceBarrierTrans(DirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                            ResourceBarrierTrans(DirectFilteredTex1RI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                            pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), DirectFilteredTexRI->getSRVGPU(0));
                            pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(0), DirectFilteredTex1RI->getUAVGPU(0));
                        }
                        else {
                            ResourceBarrierTrans(DirectFilteredTex1RI->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                            ResourceBarrierTrans(DirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                            pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), DirectFilteredTex1RI->getSRVGPU(0));
                            pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(0), DirectFilteredTexRI->getUAVGPU(0));
                        }
                    }
                    ResourceBarrierTrans(DirectFilteredTex1RI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());

                    ResourceBarrierTrans(IndirectFilteredTex1RI->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), IndirectLRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(4), IndirectVarianceTexRI->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(0), IndirectFilteredTexRI->getUAVGPU(0));
                    for (int i = 0;i < 5;i++) {
                        pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(3), offsetArray[i].get()->getSRVGPU());
                        pIcmdlistpost->Dispatch(64, 48, 1);//降噪！
                        if (i == 4)
                            break;
                        if (i % 2 == 0) {
                            ResourceBarrierTrans(IndirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                            ResourceBarrierTrans(IndirectFilteredTex1RI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                            pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), IndirectFilteredTexRI->getSRVGPU(0));
                            pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(0), IndirectFilteredTex1RI->getUAVGPU(0));
                        }
                        else {
                            ResourceBarrierTrans(IndirectFilteredTex1RI->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                            ResourceBarrierTrans(IndirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                            pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), IndirectFilteredTex1RI->getSRVGPU(0));
                            pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(0), IndirectFilteredTexRI->getUAVGPU(0));
                        }
                    }
                    ResourceBarrierTrans(IndirectFilteredTex1RI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());

                    pIcmdlistpost->SetPipelineState(computePSOITable["mixPass"].get()->PSO);
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), DirectLastFrameBufferRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(0), DirectFilteredTexRI->getUAVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getCBVTableIndex(0), lastvpmatRI->getCBVGPU());
                    pIcmdlistpost->Dispatch(64, 48, 1);//混合！
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getSRVTableIndex(0), IndirectLastFrameBufferRI->getSRVGPU(0));
                    pIcmdlistpost->SetComputeRootDescriptorTable(RSI->getUAVTableIndex(0), IndirectFilteredTexRI->getUAVGPU(0));
                    pIcmdlistpost->Dispatch(64, 48, 1);//混合！
                    ResourceBarrierTrans(pIARenderTargets[nFrameIndex].Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(lastnormalRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(normalTexRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectLastFrameBufferRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectLastFrameBufferRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectMoment1Tex->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectMoment2Tex->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectMoment1Tex->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectMoment2Tex->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectLRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectLRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectlastM1->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectlastM2->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectlastM1->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectlastM2->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(TextureResourceItemTable["DirectHistorytex" + std::to_string((nFrame - 1) % 6)].get()->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    ResourceBarrierTrans(TextureResourceItemTable["IndirectHistorytex" + std::to_string((nFrame - 1) % 6)].get()->getResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST, pIcmdlistpost.Get());
                    pIcmdlistpost->CopyResource(IndirectLastFrameBufferRI->getResource(), IndirectFilteredTexRI->getResource());
                    pIcmdlistpost->CopyResource(DirectLastFrameBufferRI->getResource(), DirectFilteredTexRI->getResource());
                    pIcmdlistpost->CopyResource(DirectlastM1->getResource(), DirectMoment1Tex->getResource());
                    pIcmdlistpost->CopyResource(DirectlastM2->getResource(), DirectMoment2Tex->getResource());
                    pIcmdlistpost->CopyResource(IndirectlastM1->getResource(), IndirectMoment1Tex->getResource());
                    pIcmdlistpost->CopyResource(IndirectlastM2->getResource(), IndirectMoment2Tex->getResource());
                    pIcmdlistpost->CopyResource(lastnormalRI->getResource(), normalTexRI->getResource());
                    pIcmdlistpost->CopyResource(TextureResourceItemTable["DirectHistorytex" + std::to_string((nFrame - 1) % 6)].get()->getResource(), DirectLRI->getResource());
                    pIcmdlistpost->CopyResource(TextureResourceItemTable["IndirectHistorytex" + std::to_string((nFrame - 1) % 6)].get()->getResource(), IndirectLRI->getResource());
                    ResourceBarrierTrans(lastnormalRI->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    ResourceBarrierTrans(normalTexRI->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectMoment1Tex->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectMoment2Tex->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectMoment1Tex->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectMoment2Tex->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectLRI->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectLRI->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectlastM1->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectlastM2->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectlastM1->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectlastM2->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(TextureResourceItemTable["DirectHistorytex" + std::to_string((nFrame - 1) % 6)].get()->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(TextureResourceItemTable["IndirectHistorytex" + std::to_string((nFrame - 1) % 6)].get()->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());

                    ResourceBarrierTrans(DirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectLastFrameBufferRI->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectLastFrameBufferRI->getResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, pIcmdlistpost.Get());
                    RSI = RootSignatureItemTable["default"].get();
                    TextureResourceItem* firstTRI = TextureResourceItemTable["grass"].get(); 
                    auto modulateTex = TextureResourceItemTable["modulatetex"].get();
                    auto DSV = TextureResourceItemTable["DS"].get()->getDSVCPU();
                    auto passRI = BufferResourceItemTable["pass"].get();
                    pIcmdlistpost->RSSetViewports(1, &stViewPort);
                    pIcmdlistpost->RSSetScissorRects(1, &stScissorRect);
                    pIcmdlistpost->SetPipelineState(PSOITable["modulatePass"].get()->PSO);
                    pIcmdlistpost->SetGraphicsRootSignature(pIRootSignature.Get());
                    pIcmdlistpost->SetDescriptorHeaps(2, heaps);
                    pIcmdlistpost->ClearRenderTargetView(modulateTex->getRTVCPU(), Colors::AliceBlue, 0, nullptr);
                    pIcmdlistpost->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                    pIcmdlistpost->OMSetRenderTargets(1, &modulateTex->getRTVCPU(), true, &DSV);
                    pIcmdlistpost->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(7), DirectFilteredTexRI->getSRVGPU(0));
                    pIcmdlistpost->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(8), IndirectFilteredTexRI->getSRVGPU(0));
                    pIcmdlistpost->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(0), firstTRI->getSRVGPU(0));
                    pIcmdlistpost->SetGraphicsRootDescriptorTable(RSI->getSamplerTableIndex(0), samplerRI->getSampler());
                    pIcmdlistpost->SetGraphicsRootDescriptorTable(RSI->getCBVTableIndex(1), passRI->getCBVGPU());
                    drawRenderItems(&RenderItemTable, pIcmdlistpost.Get(), RSI->getCBVTableIndex(0),RenderItemTable["light"].get());
                    pIcmdlistpost->SetPipelineState(PSOITable["color"].get()->PSO);
                    drawRenderItem(RenderItemTable["light"].get(), pIcmdlistpost.Get(), RSI->getCBVTableIndex(0));
                    ResourceBarrierTrans(modulateTex->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE, pIcmdlistpost.Get());
                    pIcmdlistpost->CopyResource(pIARenderTargets[nFrameIndex].Get(), modulateTex->getResource());//将modulate好的图复制到交换链及lastFrameBuffer上
                    ResourceBarrierTrans(modulateTex->getResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, pIcmdlistpost.Get());
                    ResourceBarrierTrans(pIARenderTargets[nFrameIndex].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectLRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectLRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET, pIcmdlistpost.Get());
                    ResourceBarrierTrans(DirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    ResourceBarrierTrans(IndirectFilteredTexRI->getResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, pIcmdlistpost.Get());
                    pIcmdlistpost->Close();
                    cmdlists.clear();
                    cmdlists.push_back(pIcmdlistpre.Get());
                    for (int i = 0;i < ThreadNum;i++)
                        cmdlists.push_back(paras[existThreadList[i]].cmdlist);
                    cmdlists.push_back(pIcmdlistpost.Get());
                    pICommandQueue->ExecuteCommandLists(cmdlists.size(), cmdlists.Getdata());
                    ThrowIfFailed(pISwapChain3->Present(1, 0));
                    n64fence = n64FenceValue;
                    n64FenceValue++;
                    pICommandQueue->Signal(pIFence.Get(), n64fence);
                    pIFence->SetEventOnCompletion(n64fence, hFenceEvent);
                    nStates = 1;
                    HandleMessage->clear();
                    HandleMessage->push_back(hFenceEvent);
                    lastvpmatRI->updateCB(&lastvpmat);
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
            TextureResourceItem* firstTRI = (*(para->TextureResourceItemTable))["grass"].get();//将连续纹理SRV中的第一个set到跟参数上，跟签名会自动将所有纹理都绑定上去
            StructureBufferResourceItem<AABBbox>* boxSBRI = (*para->boxTable)["model"].get();
            StructureBufferResourceItem<triangle>* triangleSBRI = (*para->triangleTable)["model"].get();
            StructureBufferResourceItem<material>* materialSBRI = para->materials->get();
            auto DirectLRI= (*para->TextureResourceItemTable)["DirectL"].get();
            auto IndirectLRI= (*para->TextureResourceItemTable)["IndirectL"].get();
            auto DirectLPSO = (*para->PSOITable)["raytraceDirect"].get()->PSO;
            auto IndirectLPSO = (*para->PSOITable)["raytraceIndirect"].get()->PSO;
            auto DSV = &(*para->TextureResourceItemTable)["DS"]->getDSVCPU();
            auto DSV1 = &(*para->TextureResourceItemTable)["DS1"]->getDSVCPU();
            para->cmdalloc->Reset();
            para->cmdlist->Reset(para->cmdalloc,DirectLPSO);
            //必须每个子线程都setRT，视口以及裁剪矩形
            para->cmdlist->OMSetRenderTargets(1, &DirectLRI->getRTVCPU(), true, DSV);
            para->cmdlist->RSSetViewports(1, para->stViewPort);
            para->cmdlist->RSSetScissorRects(1, para->stScissorRect);
            para->cmdlist->SetGraphicsRootSignature(RSI->rs);
            para->cmdlist->SetPipelineState(DirectLPSO);
            ID3D12DescriptorHeap* heaps[2] = {BRI->CBVHeap,SRI->samplerHeap};
            para->cmdlist->SetDescriptorHeaps(2, heaps);
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(0), firstTRI->getSRVGPU(0));
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSamplerTableIndex(0), SRI->getSampler());
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getCBVTableIndex(1), BRI->getCBVGPU());
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(1), boxSBRI->getSRVGPU());
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(2), triangleSBRI->getSRVGPU());
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(3), materialSBRI->getSRVGPU());
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(4), para->randomNumbers->get()->getSRVGPU());
            para->cmdlist->SetGraphicsRootDescriptorTable(RSI->getSRVTableIndex(5), para->lights->get()->getSRVGPU());
            drawRenderItem(RI, para->cmdlist, RSI->getCBVTableIndex(0));//渲染项中包含objectconstant有纹理索引，可以在shader中找到资源项对应的纹理索引。
            para->cmdlist->OMSetRenderTargets(1, &IndirectLRI->getRTVCPU(), true, DSV1);
            para->cmdlist->SetPipelineState(IndirectLPSO);
            drawRenderItem(RI, para->cmdlist, RSI->getCBVTableIndex(0));
            para->cmdlist->Close();
            SetEvent(para->hEventRenderOver);
        }
        break;
        }
    }
    return 0;
}
