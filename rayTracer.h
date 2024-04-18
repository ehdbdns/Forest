#pragma once
#include"d3dUtil.h"
#include<cmath>
#include<process.h>
#include <atlcoll.h>
#include<windowsx.h>
HANDLE g_hOutput = 0;
std::unordered_map<ID3D12DescriptorHeap*, UINT>HeapOffsetTable;
meshdata subvide(meshdata mesh, int vsize, int isize) {//传入正十二面体顶点，创建球的顶点
    std::vector<XMFLOAT3> vertices;
    std::vector<std::uint16_t> indices;
    for (int meshindex = 1;meshindex <= (isize / 3);meshindex++) {
        int a = mesh.indices[(meshindex - 1) * 3];//每个面源的点索引
        int b = mesh.indices[(meshindex - 1) * 3 + 1];
        int c = mesh.indices[(meshindex - 1) * 3 + 2];
        XMFLOAT3 v1 = XMFLOAT3{ (mesh.vertices[a].x + mesh.vertices[b].x) / 2,(mesh.vertices[a].y + mesh.vertices[b].y) / 2,(mesh.vertices[a].z + mesh.vertices[b].z) / 2 };
        XMFLOAT3 v2 = XMFLOAT3{ (mesh.vertices[b].x + mesh.vertices[c].x) / 2,(mesh.vertices[b].y + mesh.vertices[c].y) / 2,(mesh.vertices[b].z + mesh.vertices[c].z) / 2 };
        XMFLOAT3 v3 = XMFLOAT3{ (mesh.vertices[a].x + mesh.vertices[c].x) / 2,(mesh.vertices[a].y + mesh.vertices[c].y) / 2,(mesh.vertices[a].z + mesh.vertices[c].z) / 2 };
        vertices.push_back(mesh.vertices[a]);//0
        vertices.push_back(mesh.vertices[b]);//1
        vertices.push_back(mesh.vertices[c]);//2
        vertices.push_back(v1);//3
        vertices.push_back(v2);//4
        vertices.push_back(v3);//5
        int k = (meshindex - 1) * 6;
        indices.push_back(3 + k);
        indices.push_back(1 + k);
        indices.push_back(4 + k);
        indices.push_back(0 + k);
        indices.push_back(3 + k);
        indices.push_back(5 + k);
        indices.push_back(3 + k);
        indices.push_back(4 + k);
        indices.push_back(5 + k);
        indices.push_back(5 + k);
        indices.push_back(4 + k);
        indices.push_back(2 + k);
    }
    meshdata ret;
    ret.vertices = vertices;
    ret.indices = indices;
    return ret;
}
struct Vertex
{
    XMFLOAT4 position;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
    XMFLOAT3 TangentU;
    float AOk = 0;
    XMFLOAT3 color;
};


class ResourceItem {

};
class SamplerResourceItem :ResourceItem {
public:
    SamplerResourceItem() = default;
    SamplerResourceItem(ID3D12Device4* device, ID3D12DescriptorHeap* samplerheap, D3D12_SAMPLER_DESC samplerDesc, int offsetInHeap) {
        init(device, samplerheap, samplerDesc, offsetInHeap);
    }
    void init(ID3D12Device4* device, ID3D12DescriptorHeap* samplerheap, D3D12_SAMPLER_DESC samplerDesc, int offsetInHeap) {
        samplerHeap = samplerheap;
        samplerOffset = offsetInHeap;
        CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandel(samplerheap->GetCPUDescriptorHandleForHeapStart());
        device->CreateSampler(&samplerDesc, samplerHandel);
        HeapOffsetTable[samplerHeap]++;
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE getSampler() {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(samplerHeap->GetGPUDescriptorHandleForHeapStart(), samplerOffset, SamplerSize);
    }
    ID3D12DescriptorHeap* samplerHeap = nullptr;
    int samplerOffset;
private:
    UINT SamplerSize;
};
class TextureResourceItem :ResourceItem {//先初始化，然后调用创建资源函数，然后调用创建SRV函数
public:
    TextureResourceItem() = default;
    TextureResourceItem(ID3D12Device4* device, ID3D12DescriptorHeap* srvuavheap, ID3D12DescriptorHeap* rtvheap, ID3D12DescriptorHeap* dsvheap, bool issticker) {
        init(device, srvuavheap, rtvheap, dsvheap, issticker);
    }
    void init(ID3D12Device4* device, ID3D12DescriptorHeap* srvuavheap, ID3D12DescriptorHeap* rtvheap, ID3D12DescriptorHeap* dsvheap, bool issticker) {
        SRVUAVincrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        DSVincrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        RTVincrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        RTVHeap = rtvheap;
        SRVUAVHeap = srvuavheap;
        DSVHeap = dsvheap;
        isSticker = issticker;
    }
    void createStickerTex(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, wchar_t* fileName) {
        if (!isSticker)
        {
            return;
        }
        CreateDDSTextureFromFile12(device, cmdlist, fileName, Texture, TextureUpload);
    }
    void createWritableTex(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, D3D12_RESOURCE_DESC* TextureDesc, D3D12_HEAP_FLAGS flag, D3D12_CLEAR_VALUE* dsclear) {
        if (isSticker)
            return;
        device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), flag, TextureDesc, D3D12_RESOURCE_STATE_GENERIC_READ, dsclear, IID_PPV_ARGS(&Texture));
    }
    void createSRVforResourceItem(D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc, ID3D12Device4* device, UINT SRVOffsetInHeap) {
        srvDesc->Format = Texture->GetDesc().Format;
        if (srvDesc->ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D) {
            srvDesc->Texture2D.MipLevels = Texture->GetDesc().MipLevels;
        }
        else if (srvDesc->ViewDimension == D3D12_SRV_DIMENSION_TEXTURE3D) {
            srvDesc->Texture3D.MipLevels = Texture->GetDesc().MipLevels;
        }
        else if (srvDesc->ViewDimension == D3D12_SRV_DIMENSION_TEXTURECUBE) {
            srvDesc->TextureCube.MipLevels = Texture->GetDesc().MipLevels;
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SRVUAVHeap->GetCPUDescriptorHandleForHeapStart(), SRVOffsetInHeap, SRVUAVincrementSize);
        device->CreateShaderResourceView(Texture.Get(), srvDesc, cpuHandle);
        SRVOffsetList.push_back(SRVOffsetInHeap);
        HeapOffsetTable[SRVUAVHeap]++;
    }
    void createUAVforResourceItem(D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc, ID3D12Device4* device, UINT UAVOffsetInHeap) {
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SRVUAVHeap->GetCPUDescriptorHandleForHeapStart(), UAVOffsetInHeap, SRVUAVincrementSize);
        device->CreateUnorderedAccessView(Texture.Get(), nullptr, uavDesc, cpuHandle);
        UAVOffsetList.push_back(UAVOffsetInHeap);
        HeapOffsetTable[SRVUAVHeap]++;
    }
    void createDSVforResourceItem(D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDesc, ID3D12Device4* device, UINT DSVOffsetInHeap, ID3D12GraphicsCommandList* cmdlist) {
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(DSVHeap->GetCPUDescriptorHandleForHeapStart(), DSVOffsetInHeap, DSVincrementSize);
        device->CreateDepthStencilView(Texture.Get(), dsvDesc, dsvHandle);
        DSVoffset = DSVOffsetInHeap;
        HeapOffsetTable[DSVHeap]++;
        cmdlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition
        (Texture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
    }
    void createRTVforResourceItem(D3D12_RENDER_TARGET_VIEW_DESC* rtvDesc, ID3D12Device4* device, UINT RTVOffsetInHeap, ID3D12GraphicsCommandList* cmdlist) {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVHeap->GetCPUDescriptorHandleForHeapStart(), RTVOffsetInHeap, RTVincrementSize);
        device->CreateRenderTargetView(Texture.Get(), rtvDesc, rtvHandle);
        RTVoffset = RTVOffsetInHeap;
        HeapOffsetTable[RTVHeap]++;
        cmdlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition
        (Texture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE getSRVGPU(int SRVnote) {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(SRVUAVHeap->GetGPUDescriptorHandleForHeapStart(), SRVOffsetList[SRVnote], SRVUAVincrementSize);
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE getUAVGPU(int UAVnote) {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(SRVUAVHeap->GetGPUDescriptorHandleForHeapStart(), UAVOffsetList[UAVnote], SRVUAVincrementSize);
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE getRTVGPU() {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(RTVHeap->GetGPUDescriptorHandleForHeapStart(), RTVoffset, RTVincrementSize);
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE getDSVCPU() {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(DSVHeap->GetCPUDescriptorHandleForHeapStart(), DSVoffset, DSVincrementSize);
    }
    ID3D12DescriptorHeap* RTVHeap = nullptr;
    ID3D12DescriptorHeap* DSVHeap = nullptr;
    ID3D12DescriptorHeap* SRVUAVHeap = nullptr;
    int RTVoffset;
    int DSVoffset;
    std::vector<int>SRVOffsetList;
    std::vector<int>UAVOffsetList;
private:
    bool isSticker = true;
    UINT SRVUAVincrementSize;
    UINT DSVincrementSize;
    UINT RTVincrementSize;
    ComPtr< ID3D12Resource> Texture;
    ComPtr< ID3D12Resource> TextureUpload;
};
template<class T>
class BufferResourceItem :ResourceItem {//管理cbv以及对应结构体以及资源本身,构造函数创建资源，并map，更新函数可以memcpy更新，但创建完资源项得自行创建CBV并赋值偏移量
public:
    BufferResourceItem() = default;
    BufferResourceItem(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, T* strP, bool isstatic, ID3D12DescriptorHeap* cbvheap) {
        init(device, cmdlist, strP, isstatic, cbvheap);
    }
    void init(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, T* strP, bool isstatic, ID3D12DescriptorHeap* cbvheap) {
        this->isStatic = isstatic;
        this->CBVHeap = cbvheap;
        CBVoffset = HeapOffsetTable[CBVHeap];
        SRVCBVUAVincrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        this->str = *strP;
        D3D12_RANGE range = { 0,0 };
        strSize = sizeof(T);
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.SizeInBytes = sizeof(T) + 255 & ~255;
        if (isStatic) {
            ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(T) + 255 & ~255), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBufferDefault)));
            constantBufferUpload->Map(0, &range, reinterpret_cast<void**>(&cbmapped));
            memcpy(cbmapped, &str, strSize);
            D3D12_SUBRESOURCE_DATA subResourceData;
            subResourceData.pData = &str;
            subResourceData.RowPitch = strSize;
            subResourceData.SlicePitch = subResourceData.RowPitch;
            UpdateSubresources<1>(cmdlist, constantBufferDefault.Get(), constantBufferUpload.Get(), 0, 0, 1, &subResourceData);
            cbvDesc.BufferLocation = constantBufferDefault->GetGPUVirtualAddress();

        }
        else {

            ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(T) + 255 & ~255), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBufferUpload)));
            constantBufferUpload->Map(0, &range, reinterpret_cast<void**>(&cbmapped));
            memcpy(cbmapped, &str, strSize);
            cbvDesc.BufferLocation = constantBufferUpload->GetGPUVirtualAddress();
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CBVHeap->GetCPUDescriptorHandleForHeapStart(), CBVoffset, SRVCBVUAVincrementSize);
        device->CreateConstantBufferView(&cbvDesc, cpuHandle);
        HeapOffsetTable[CBVHeap]++;
    }
    void updateCB(T* strP) {
        if (isStatic)
            return;
        str = *strP;
        memcpy(cbmapped, &str, strSize);
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE getCBVGPU() {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(CBVHeap->GetGPUDescriptorHandleForHeapStart(), CBVoffset, SRVCBVUAVincrementSize);
    }
    ID3D12DescriptorHeap* CBVHeap = nullptr;

    UINT strSize;
    int CBVoffset;
    T str;
    bool isStatic = true;
private:
    byte* cbmapped = nullptr;
    UINT SRVCBVUAVincrementSize;
    ComPtr< ID3D12Resource> constantBufferDefault;
    ComPtr< ID3D12Resource> constantBufferUpload;
};

struct GeometryItem {//构造函数传入顶点及其索引，然后调用创建静态or动态顶点，最后获取其vbvibv即可；
public:
    GeometryItem() = default;
    ~GeometryItem() {
        freeVertexAndIndex();
    }
    void createDynamicGeo(ID3D12Device4* device, bool isStatic, std::vector<Vertex>* vertices, std::vector<std::uint16_t>* indices) {
        this->vertices = vertices;
        this->indices = indices;
        vbsize = (UINT)sizeof(Vertex) * (UINT)vertices->size();
        ibsize = (UINT)sizeof(std::uint16_t) * (UINT)indices->size();
        device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vbsize), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&VertexBufferUpload));//顶点缓冲及索引缓冲初始状态为common效率最佳

        device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(ibsize), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&IndexBufferUpload));

        UINT8* vmapped = nullptr;UINT8* imapped = nullptr;
        D3D12_RANGE range = { 0,0 };
        VertexBufferUpload->Map(0, &range, reinterpret_cast<void**>(&vmapped));
        IndexBufferUpload->Map(0, &range, reinterpret_cast<void**>(&imapped));

        memcpy(vmapped, vertices->data(), vbsize);
        memcpy(imapped, indices->data(), ibsize);
        VertexBufferUpload->Unmap(0, nullptr);
        IndexBufferUpload->Unmap(0, nullptr);
        if (!isStatic) {
            vbv.BufferLocation = VertexBufferUpload->GetGPUVirtualAddress();
            vbv.SizeInBytes = vbsize;
            vbv.StrideInBytes = sizeof(Vertex);
            ibv.Format = DXGI_FORMAT_R16_UINT;
            ibv.SizeInBytes = ibsize;
            ibv.BufferLocation = IndexBufferUpload->GetGPUVirtualAddress();
        }
    }
    void createStaticGeo(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, std::vector<Vertex>* vertices, std::vector<std::uint16_t>* indices) {//这之后必须excute
        createDynamicGeo(device, true, vertices, indices);
        device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vbsize), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&VertexBufferDefault));

        device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(ibsize), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&IndexBufferDefault));
        D3D12_SUBRESOURCE_DATA subResourceData;
        subResourceData.pData = vertices->data();
        subResourceData.RowPitch = vbsize;
        subResourceData.SlicePitch = subResourceData.RowPitch;
        UpdateSubresources<1>(cmdlist, VertexBufferDefault.Get(), VertexBufferUpload.Get(), 0, 0, 1, &subResourceData);
        subResourceData.pData = indices->data();
        subResourceData.RowPitch = ibsize;
        subResourceData.SlicePitch = subResourceData.RowPitch;
        UpdateSubresources<1>(cmdlist, IndexBufferDefault.Get(), IndexBufferUpload.Get(), 0, 0, 1, &subResourceData);
        vbv.BufferLocation = VertexBufferDefault->GetGPUVirtualAddress();
        vbv.SizeInBytes = vbsize;
        vbv.StrideInBytes = sizeof(Vertex);
        ibv.Format = DXGI_FORMAT_R16_UINT;
        ibv.SizeInBytes = ibsize;
        ibv.BufferLocation = IndexBufferDefault->GetGPUVirtualAddress();
    }
    D3D12_VERTEX_BUFFER_VIEW* getVBV() {
        return &vbv;
    }
    D3D12_INDEX_BUFFER_VIEW* getIBV() {
        return &ibv;
    }
    void freeVertexAndIndex() {
        vertices->~vector();
        indices->~vector();
    }
private:
    UINT vbsize;
    UINT ibsize;
    UINT indexNum;
    std::vector<Vertex>* vertices;
    std::vector<std::uint16_t>* indices;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
    ComPtr< ID3D12Resource> VertexBufferDefault;
    ComPtr< ID3D12Resource> VertexBufferUpload;
    ComPtr< ID3D12Resource> IndexBufferDefault;
    ComPtr< ID3D12Resource> IndexBufferUpload;

};
struct RenderItem {
    RenderItem() = default;
    RenderItem(GeometryItem* geo, int instancenum, int basevertex, int startindex, int indexnum, int startinstance, D3D12_PRIMITIVE_TOPOLOGY primitive, objectconstant* objc, ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, ID3D12DescriptorHeap* CBVHeap) {
        init(geo, instancenum, basevertex, startindex, indexnum, startinstance, primitive, objc, device, cmdlist, CBVHeap);
    }
    void init(GeometryItem* geo, int instancenum, int basevertex, int startindex, int indexnum, int startinstance, D3D12_PRIMITIVE_TOPOLOGY primitive, objectconstant* objc, ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, ID3D12DescriptorHeap* CBVHeap) {
        Geo = geo;
        InstanceNum = instancenum;
        baseVertex = basevertex;
        startIndex = startindex;
        indexNum = indexnum;
        startInstance = startinstance;
        Primitive = primitive;
        auto ptr = std::make_unique< BufferResourceItem<objectconstant>>(device, cmdlist, objc, false, CBVHeap);//让渲染项管理一个资源项
        objconstantRI = std::move(ptr);
    }
    std::unique_ptr< BufferResourceItem<objectconstant>>objconstantRI;
    D3D12_PRIMITIVE_TOPOLOGY Primitive;
    GeometryItem* Geo;
    int InstanceNum;
    int baseVertex;
    int startIndex;
    int indexNum;
    int startInstance;
    int threadNote;
};
struct RootSignatureItem {
public:
    RootSignatureItem() = default;
    RootSignatureItem(ID3D12RootSignature* rs, int descriptorNum, int srvnum, int cbvnum, int uavnum, int samplernum, CD3DX12_DESCRIPTOR_RANGE* dt) {
        init(rs, descriptorNum, srvnum, cbvnum, uavnum, samplernum, dt);
    }
    void init(ID3D12RootSignature* rs, int descriptorNum, int srvnum, int cbvnum, int uavnum, int samplernum, CD3DX12_DESCRIPTOR_RANGE* dt) {
        this->rs = rs;
        SRVNum = srvnum;
        CBVNum = cbvnum;
        UAVNum = uavnum;
        SamplerNum = samplernum;
        for (int i = 0;i < descriptorNum;i++) {
            D3D12_DESCRIPTOR_RANGE_TYPE type = dt[i].RangeType;
            if (type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
                SRVDescriptorTableIndex.push_back(i);
            if (type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
                CBVDescriptorTableIndex.push_back(i);
            if (type == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
                UAVDescriptorTableIndex.push_back(i);
            if (type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                SamplerDescriptorTableIndex.push_back(i);
        }
    }
    int getSRVTableIndex(int srvIndex) {
        return SRVDescriptorTableIndex[srvIndex];
    }
    int getCBVTableIndex(int cbvIndex) {
        return CBVDescriptorTableIndex[cbvIndex];
    }
    int getUAVTableIndex(int uavIndex) {
        return UAVDescriptorTableIndex[uavIndex];
    }
    int getSamplerTableIndex(int samplerIndex) {
        return SamplerDescriptorTableIndex[samplerIndex];
    }
    ID3D12RootSignature* rs = nullptr;
private:
    int SRVNum;
    int CBVNum;
    int UAVNum;
    int SamplerNum;
    std::vector<int>SRVDescriptorTableIndex;
    std::vector<int>CBVDescriptorTableIndex;
    std::vector<int>UAVDescriptorTableIndex;
    std::vector<int>SamplerDescriptorTableIndex;
};
void drawRenderItem(RenderItem* ri, ID3D12GraphicsCommandList* cmdlist, int objcPara) {
    CD3DX12_GPU_DESCRIPTOR_HANDLE objcHandle = ri->objconstantRI->getCBVGPU();
    cmdlist->SetGraphicsRootDescriptorTable(objcPara, objcHandle);
    cmdlist->IASetVertexBuffers(0, 1, ri->Geo->getVBV());
    cmdlist->IASetIndexBuffer(ri->Geo->getIBV());
    cmdlist->IASetPrimitiveTopology(ri->Primitive);
    cmdlist->DrawIndexedInstanced(ri->indexNum, ri->InstanceNum, ri->startIndex, ri->baseVertex, ri->startInstance);
}
class APP {
public:
	APP() = default;
    bool initDX12(HINSTANCE hInstance, WNDPROC CALLBACK    WndProc, int       nCmdShow) {
        createWindow(hInstance, WndProc, nCmdShow);
        openDebug();
        createFactoryAndDevice();
        return true;
    }
    int iWidth = 1024;
    int iHeight = 768;
    UINT nDXGIFactoryFlags = 0U;
    HWND hWnd = nullptr;
    UINT ThreadNum = 0;
    ComPtr<IDXGIFactory5>                pIDXGIFactory5;
    ComPtr<IDXGIAdapter1>                pIAdapter;
    ComPtr<ID3D12Device4>                pID3DDevice;
private:

    bool createWindow(HINSTANCE hInstance, WNDPROC CALLBACK    WndProc, int       nCmdShow) {
        AllocConsole();
        g_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        {
            WNDCLASSEX wcex = {};
            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style = CS_GLOBALCLASS;
            wcex.lpfnWndProc = WndProc;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = hInstance;
            wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);		//防止无聊的背景重绘
            wcex.lpszClassName = GRS_WND_CLASS_NAME;
            RegisterClassEx(&wcex);

            DWORD dwWndStyle = WS_OVERLAPPED | WS_SYSMENU;
            RECT rtWnd = { 0, 0, iWidth, iHeight };
            AdjustWindowRect(&rtWnd, dwWndStyle, FALSE);

            // 计算窗口居中的屏幕坐标
            INT posX = (GetSystemMetrics(SM_CXSCREEN) - rtWnd.right - rtWnd.left) / 2;
            INT posY = (GetSystemMetrics(SM_CYSCREEN) - rtWnd.bottom - rtWnd.top) / 2;

            hWnd = CreateWindowW(GRS_WND_CLASS_NAME
                , GRS_WND_TITLE
                , dwWndStyle
                , posX
                , posY
                , rtWnd.right - rtWnd.left
                , rtWnd.bottom - rtWnd.top
                , nullptr
                , nullptr
                , hInstance
                , nullptr);

            if (!hWnd)
            {
                return FALSE;
            }

            ShowWindow(hWnd, nCmdShow);
            UpdateWindow(hWnd);
        }


    }
    void openDebug() {
        // 打开显示子系统的调试支持
#if defined(_DEBUG)
        ComPtr<ID3D12Debug> debugController;
        ComPtr<ID3D12Debug1>debugController1;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
        debugController.As(&debugController1);
        debugController1->SetEnableGPUBasedValidation(true);
        // 打开附加的调试支持
        nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

#endif 
    }
    void createFactoryAndDevice() {
        //创建factory
        {
            CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(&pIDXGIFactory5));
            ThrowIfFailed(pIDXGIFactory5->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
        }
        //创建device
        {
            DXGI_ADAPTER_DESC1 desc = {};
            for (UINT index = 0;DXGI_ERROR_NOT_FOUND != pIDXGIFactory5->EnumAdapters1(index, &pIAdapter);index++) {

                pIAdapter->GetDesc1(&desc);
                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    continue;
                }
                if (SUCCEEDED(D3D12CreateDevice(pIAdapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
                {

                    break;
                }
            }
            ThrowIfFailed(D3D12CreateDevice(pIAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pID3DDevice)));
            TCHAR pszWndTitle[MAX_PATH] = {};
            ThrowIfFailed(pIAdapter->GetDesc1(&desc));
            ::GetWindowText(hWnd, pszWndTitle, MAX_PATH);
            ::SetWindowText(hWnd, pszWndTitle);
        }
    }

};