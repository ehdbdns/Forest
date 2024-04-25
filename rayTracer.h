#pragma once
#include"d3dUtil.h"
#include<cmath>
#include<process.h>
#include <atlcoll.h>
#include<windowsx.h>
#define GRS_UPPER(A,B) ((UINT)(((A)+((B)-1))&~(B - 1)))
HANDLE g_hOutput = 0;
std::unordered_map<ID3D12DescriptorHeap*, UINT>HeapOffsetTable;


namespace EST {

    struct Block {
        Block* next = nullptr;
    };

    class allocator {
    public:
        allocator() = default;
        allocator(UINT pagesize, UINT dividecount) {
            pageSize = pagesize;
            divideCount = dividecount;
            ptr = operator new (pageSize);//申请一大块内存
            BlockSize = pageSize / pow(2, divideCount);//一个block占多大
            freeList = reinterpret_cast<Block*>(ptr);//让freeList存储大块内存的首地址，并将第一个block的开头一小部分用于存储next指针
            std::uint8_t* p = reinterpret_cast<std::uint8_t*>(ptr);
            for (int j = 1;j < pow(2, divideCount);j++) {//将所有block加入freelist
                p += BlockSize;
                Block* b;
                b = reinterpret_cast<Block*>(p);
                b->next = freeList;
                freeList = b;
                freeNum++;
            }
        }
        void* allocate() {
            if (freeNum > 0) {
                Block* b = freeList;
                freeList = b->next;
                freeNum--;
                return (void*)b;
            }
            return nullptr;
        }
        void free(void* freeBlock) {
            Block* b = reinterpret_cast<Block*>(freeBlock);
            b->next = freeList;
            freeList = b;
            freeNum++;
        }
        void freeAll() {

        }
        void* getPTR() {//获得大块内存的首地址
            return ptr;

        }
        UINT divideCount;
    private:
        Block* freeList;
        int freeNum;

        UINT BlockSize;
        UINT pageSize;
        void* ptr = nullptr;
    };
    struct Page {
    public:
        Page* next = nullptr;
        allocator* alloc = nullptr;
    };
    class MemoryManager {
    public:
        MemoryManager() = default;
        MemoryManager(UINT pageSizeInKB, UINT maxdividenum) {
            PageSize = pageSizeInKB * 1000;
            MaxDivideNum = maxdividenum;
            PageList = nullptr;
            for (int i = 0;i < maxdividenum;i++)
                allocPage(i);
        }
        Page* allocPage(int PageDivideNum) {
            allocator* alloc = new allocator(PageSize, PageDivideNum);
            Page* p = reinterpret_cast<Page*>(reinterpret_cast<std::uint8_t*>(alloc->getPTR()) + sizeof(Block));
            p->alloc = alloc;
            p->next = PageList;
            PageList = p;
            PageNum++;
            return p;
        }
        void freePage(void* freepage, UINT size) {
            Page* p = reinterpret_cast<Page*>(freepage);
            Page* it = PageList;
            Page* preit = nullptr;
            for (int i = 0;i < PageNum;i++) {
                if ((void*)it == freepage) {
                    preit->next = it->next;
                    it->alloc->freeAll();
                    break;
                }
                preit = it;
                it = it->next;
            }
        }
        void* alloc(UINT MSize,allocator**alloc) {
            int level = 0;
            int currentSize = PageSize / pow(2, MaxDivideNum);
            while (MSize > currentSize) {
                level++;
                currentSize *= 2;
            }
            Page* p = PageList;
            void* m;
            while (1) {
                if (p->next == nullptr) {
                   p=allocPage(MaxDivideNum - level - 1);
                   m = p->alloc->allocate();
                    break;
                }
                if (p->alloc->divideCount == MaxDivideNum - level - 1) {
                    m= p->alloc->allocate();
                    if (m == nullptr) {
                        p = p->next;
                        continue;
                    }
                    break;
                }
                p = p->next;
            }
            *alloc= p->alloc;
            return m;
        }

    private:
        UINT MaxDivideNum;
        Page* PageList = nullptr;
        UINT PageSize;
        UINT PageNum = 0;
    };
    EST::MemoryManager* memorymanager = new EST::MemoryManager(1024, 5);
    template<class T>
    class vector {
    public:
        vector() {
            init();
        }

        vector(T* startInit, int size) {
            init();
            resize(size);
            for (int i = 0;i < Size;i++) {
                data[i] = *startInit;
                startInit++;
            }
        }
        void init() {
            StyleSizeInBytes = sizeof(T);
            MM = memorymanager;
        }
        void reserve(int cap) {
            int MemorySize = cap * StyleSizeInBytes;
            T* preBegin = begin;
            T* preit =begin;
           begin= reinterpret_cast<T*>( MM->alloc(MemorySize,&alloc));
           data = new(begin)T[cap];
           if (capacity > 0) {
               for (int i = 0;i < capacity;i++) {
                   data[i] = *preit;
                   preit++;
               }
               alloc->free(preBegin);
           }
           capacity = cap;
        }
        void resize(int size) {
            if (size > capacity)
                reserve(2*size);
            end = begin + size-1;
            Size = size;
        }
        void memset(T* start, int size,T obj) {
            for (int i = 0;i < size;i++)
                data[start-begin+i] = obj;
        }
        void push_back(T obj) {
            Size += 1;
            if (Size > capacity)
                resize(Size);
            T* push_pos =end;
            *push_pos = obj;
            end++;
        }
        T& operator[](int index) {
            if (index > Size - 1)
                abort();
            return data[index];
      }
        int size() {
            return Size;
        }
        bool empty() {
            if (Size == 0)
                return true;
            return false;
        }
        T back() {
            return *(end-1);
        }
        void pop_back() {
            Size--;
            end--;
            //if (Size < (capacity / 2))
            //    reserve(capacity / 2);
        }
        void erase(T* erasebegin, T* eraseend, T obj) {
            for (int i = 0;i < eraseend - erasebegin;i++) {
                if (obj == *(erasebegin + i)) {
                    for (int j = 0;j < Size - 1-(erasebegin+i-begin);j++)
                        *(erasebegin + i+j) = *(erasebegin + i + 1+j);
                    Size--;
                    end--;
                    return;
                }
            }
        }
        void clear() {
            Size = 0;
            end = begin;
        }
        T* getbegin() {
            return begin;
        }
        T* getend() {
            return end;
        }
        T* Getdata() {
            return data;
        }
        void insert(int index, T obj) {
            
        }
    private:
        T* data;
        allocator* alloc;
        MemoryManager* MM;
        int StyleSizeInBytes;
        int Size=0;
        int capacity=0;
        T* begin;
        T* end;
    };
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
struct ResourceID {
    int HeapIndex;
    int HeapOffset;
};
struct buddyID {
    int level;
    int index;
};
class buddySystem {
public:
    buddySystem() = default;
    buddySystem(UINT totalSizeInKB, UINT hierarchy, ID3D12Device4* device) {
        init(totalSizeInKB, hierarchy, device);
    }
   virtual void init(UINT totalSizeInKB, UINT hierarchy, ID3D12Device4* device) {
        totalSize = totalSizeInKB * 1024;
        Hierarchy = hierarchy;
        minSize = totalSize / pow(2, hierarchy - 1);
        stateList = new EST::vector<int>[hierarchy];
        for (int i = 0;i < hierarchy;i++) {
            int num = pow(2, hierarchy - i + 1);
            stateList[i].resize(num);
            stateList[i].memset(stateList[i].Getdata(), num, 0);
        }
    }
   buddyID createPlacedBufferResourceInBS(ID3D12Resource** BufferResource, ID3D12Device4* device, UINT BufferSize) {
       buddyID ID = allocate(BufferSize);
       if (ID.index != -1) {
           ThrowIfFailed(device->CreatePlacedResource(
               Heap.Get()
               , ID.index * minSize * pow(2, ID.level)
               , &CD3DX12_RESOURCE_DESC::Buffer(BufferSize)
               , D3D12_RESOURCE_STATE_GENERIC_READ
               , nullptr
               , IID_PPV_ARGS(BufferResource)));
       }
       return ID;
   }
    buddyID allocate(UINT size) {
        buddyID ID = { -1,-1 };
        UINT currentSize = minSize;
        int level = 0;
        while (size > currentSize) {
            level++;
            currentSize *= 2;
            if (level > Hierarchy - 1)
                return ID;
        }
        for (int i = 0;i < stateList[level].size();i++) {
            if (stateList[level][i] == 0) {
                stateList[level][i] = 1;
                for (int j = level + 1;j < Hierarchy - 1;j++)
                    stateList[j][i / pow(2, j - level)] = 1;
                for (int j = level - 1;j >= 0;j--) {
                    for (int k = 0;k < pow(2, level - j);k++)
                        stateList[j][i * pow(2, level - j) + k] = 1;
                }
                ID.level = level;
                ID.index = i;
                return ID;
            }
        }
        return ID;
    }
    void free(buddyID ID) {
        stateList[ID.level][ID.index] = 0;
        for (int j = ID.level;j < Hierarchy - 1;j++) {
            int cindex = ID.index / pow(2, j - ID.level);
            stateList[j][cindex] = 0;
            if (ID.index % 2 == 0) {
                if (stateList[j][cindex + 1] == 0)
                    continue;
                break;
            }
            else {
                if (stateList[j][cindex - 1] == 0)
                    continue;
                break;
            }
        }
        for (int j = ID.level - 1;j >= 0;j--) {
            for (int k = 0;k < pow(2, ID.level - j);k++)
                stateList[j][ID.index * pow(2, ID.level - j) + k] = 0;
        }
    }
    EST::vector<int>* stateList;
    UINT totalSize;
    UINT minSize;
    UINT Hierarchy;
    ComPtr<ID3D12Heap> Heap;
};
class uploadBuddySystem :buddySystem {
public:
    uploadBuddySystem() = default;
    uploadBuddySystem(UINT totalSizeInKB, UINT hierarchy, ID3D12Device4* device) {
        init(totalSizeInKB, hierarchy, device);
    }
     void init(UINT totalSizeInKB, UINT hierarchy, ID3D12Device4* device)override {
        buddySystem::init(totalSizeInKB, hierarchy, device);
        D3D12_HEAP_DESC stDefaultHeapDesc = {  };
        stDefaultHeapDesc.Alignment = 0;
        stDefaultHeapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;		//上传堆类型
        stDefaultHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        stDefaultHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        stDefaultHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        stDefaultHeapDesc.SizeInBytes = totalSize;
        ThrowIfFailed(device->CreateHeap(&stDefaultHeapDesc, IID_PPV_ARGS(&Heap)));
    }
     buddyID createPlacedBufferResourceInBS(ID3D12Resource** BufferResource, ID3D12Device4* device,UINT BufferSize) {
         return  buddySystem::createPlacedBufferResourceInBS(BufferResource, device, BufferSize);
     }
     buddyID allocate(UINT size) {
         return  buddySystem::allocate(size);
     }
     void free(buddyID ID) {
         buddySystem::free(ID);
     }
};
class defaultBuddySystem :buddySystem {
public:
    defaultBuddySystem() = default;
    defaultBuddySystem(UINT totalSizeInKB, UINT hierarchy, ID3D12Device4* device) {
        init(totalSizeInKB, hierarchy, device);
    }
     void init(UINT totalSizeInKB, UINT hierarchy, ID3D12Device4* device)override {
        buddySystem::init(totalSizeInKB, hierarchy, device);
        D3D12_HEAP_DESC stDefaultHeapDesc = {  };
        stDefaultHeapDesc.Alignment = 0;
        stDefaultHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;		
        stDefaultHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        stDefaultHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        stDefaultHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        stDefaultHeapDesc.SizeInBytes = totalSize;
        ThrowIfFailed(device->CreateHeap(&stDefaultHeapDesc, IID_PPV_ARGS(&Heap)));
    }
     buddyID createPlacedBufferResourceInBS(ID3D12Resource** BufferResource, ID3D12Device4* device,UINT BufferSize) {
        return buddySystem::createPlacedBufferResourceInBS(BufferResource, device, BufferSize);
     }
     buddyID allocate(UINT size) {
        return buddySystem::allocate(size);
     }
     void free(buddyID ID) {
         buddySystem::free(ID);
     }
};
class SegregatedFreeLists {

};
class RT_DS_TextureSegregatedFreeLists:SegregatedFreeLists {
public:
    RT_DS_TextureSegregatedFreeLists() = default;
    RT_DS_TextureSegregatedFreeLists(UINT minsizeinKB, UINT listnum, ID3D12Device4* device) {
        init(minsizeinKB, listnum, device);
    }
    ~RT_DS_TextureSegregatedFreeLists() {
        delete[]DefaultHeapState;
    }
    void freeDefaultResource(ResourceID ID) {
        DefaultHeapDeadLists[ID.HeapIndex].push_back(ID.HeapOffset);
        DefaultHeapState[ID.HeapIndex][1]--;
    }
    void init(UINT minsizeinKB, UINT listnum, ID3D12Device4* device) {
        minSizeInKB = minsizeinKB;
        listNum = listnum;
        Device = device;
        DefaultHeaps.resize(listNum);
        DefaultHeapDeadLists = new EST::vector<int>[listnum];
        DefaultHeapState = new EST::vector<int>[listnum];
        for (int i = 0;i < listnum;i++) {
            DefaultHeapState[i].resize(2);
            DefaultHeapState[i].memset(DefaultHeapState[i].Getdata(), 2, 0);
        }
    }
    ResourceID createPlacedResourceInDefaultSFL(ID3D12Resource** Tex, D3D12_RESOURCE_DESC* TextureDesc, D3D12_HEAP_FLAGS flag, D3D12_CLEAR_VALUE* dsclear) {
        UINT currentSize = minSizeInKB*1000;
        int level = 0;
        ResourceID ID = { -1,-1 };
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT  TexLayouts = {};
        UINT                                nNumRows = {};
        UINT64                              n64RowSizeInBytes = {};
        UINT64                              n64TotalBytes = 0;
        Device->GetCopyableFootprints(TextureDesc, 0, 1, 0, &TexLayouts, &nNumRows, &n64RowSizeInBytes, &n64TotalBytes);
        while (currentSize < n64TotalBytes) {
            currentSize *= 2;
            level++;
        }
        if (level > listNum - 1)
            return ID;
        int NumResource = pow(2, 3 - min(3, level));
        int SizeResource = pow(2, level) * GRS_UPPER(minSizeInKB, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)*1000;
        if (DefaultHeapState[level][0] == 0)
        {
            DefaultHeapState[level][0] = 1;
            D3D12_HEAP_DESC stDefaultHeapDesc = {  };
            stDefaultHeapDesc.Alignment = 0;
            stDefaultHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;		//上传堆类型
            stDefaultHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            stDefaultHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            stDefaultHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
            stDefaultHeapDesc.SizeInBytes = SizeResource * NumResource;
            ThrowIfFailed(Device->CreateHeap(&stDefaultHeapDesc, IID_PPV_ARGS(&DefaultHeaps[level])));
        }
        if (DefaultHeapState[level][1] < NumResource)
        {
            int offset;
            if (!DefaultHeapDeadLists[level].empty())
            {
                offset = DefaultHeapDeadLists[level].back();
                DefaultHeapDeadLists[level].pop_back();
            }
            else
                offset = DefaultHeapState[level][1];
            ThrowIfFailed(Device->CreatePlacedResource(
                DefaultHeaps[level].Get()
                , offset * SizeResource
                , TextureDesc
                , D3D12_RESOURCE_STATE_GENERIC_READ
                , dsclear
                , IID_PPV_ARGS(Tex)));
            DefaultHeapState[level][1]++;
            ID.HeapIndex = level;
            ID.HeapOffset = offset;
            return ID;
        }
        return ID;
    }
private:
    ID3D12Device4* Device;
    EST::vector<ComPtr<ID3D12Heap>>DefaultHeaps;
    UINT minSizeInKB;
    UINT listNum;
    EST::vector<int>(* DefaultHeapDeadLists);
    EST::vector<int>(* DefaultHeapState);
};
class NON_RT_DS_TextureSegregatedFreeLists :SegregatedFreeLists {
public:
    NON_RT_DS_TextureSegregatedFreeLists() = default;
    NON_RT_DS_TextureSegregatedFreeLists(UINT minsizeinKB, UINT listnum, ID3D12Device4* device) {
        init(minsizeinKB,listnum,device);
    }
    ~NON_RT_DS_TextureSegregatedFreeLists() {
        delete[]UploadHeapState;
        delete[]DefaultHeapState;
        delete[]UploadHeapDeadLists;
        delete[]DefaultHeapDeadLists;
    }
    void freeUploadResource(ResourceID ID) {
        UploadHeapDeadLists[ID.HeapIndex].push_back(ID.HeapOffset);
        UploadHeapState[ID.HeapIndex][1]--;
    }
    void freeDefaultResource(ResourceID ID) {
        DefaultHeapDeadLists[ID.HeapIndex].push_back(ID.HeapOffset);
        DefaultHeapState[ID.HeapIndex][1]--;
    }
    void init(UINT minsizeinKB, UINT listnum, ID3D12Device4* device) {
        minSizeInKB = minsizeinKB;
        listNum = listnum;
        Device = device;
        UploadHeaps.resize(listNum);
        DefaultHeaps.resize(listNum);
        UploadHeapState = new EST::vector<int>[listnum];
        DefaultHeapState = new EST::vector<int>[listnum];
        UploadHeapDeadLists = new EST::vector<int>[listnum];
        DefaultHeapDeadLists = new EST::vector<int>[listnum];
        for (int i = 0;i < listnum;i++) {
            UploadHeapState[i].resize(2);
            DefaultHeapState[i].resize(2);
            UploadHeapState[i].memset(UploadHeapState[i].Getdata(), 2, 0);
            DefaultHeapState[i].memset(DefaultHeapState[i].Getdata(), 2, 0);
        }
    }
    ResourceID createPlacedResourceInUploadTexSFLHeap(ID3D12Resource*Tex,UINT BufferSize) {
        UINT currentSize = minSizeInKB*1000;
        int level = 0;
        ResourceID ID = {-1,-1};
        while (currentSize < BufferSize) {
            currentSize *= 2;
            level++;
        }
        if (level > listNum-1)
            return ID;
        int NumResource = pow(2, 3 - min(3, level));
        int SizeResource = pow(2, level) * GRS_UPPER(minSizeInKB, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)*1000;
        if (UploadHeapState[level][0] == 0)
        {
            UploadHeapState[level][0] = 1;
            D3D12_HEAP_DESC stUploadHeapDesc = {  };
                stUploadHeapDesc.Alignment = 0;
                stUploadHeapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;		//上传堆类型
                stUploadHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                stUploadHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                stUploadHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
                stUploadHeapDesc.SizeInBytes = SizeResource *NumResource;
                ThrowIfFailed(Device->CreateHeap(&stUploadHeapDesc, IID_PPV_ARGS(&UploadHeaps[level])));
        }
        if (UploadHeapState[level][1] < NumResource)
        {
            int offset;
            if (!UploadHeapDeadLists[level].empty())
            {
                offset = UploadHeapDeadLists[level].back();
                UploadHeapDeadLists[level].pop_back();
            }
            else
                offset = UploadHeapState[level][1];
            ThrowIfFailed(Device->CreatePlacedResource(
                UploadHeaps[level].Get()
                , offset* SizeResource
                , &CD3DX12_RESOURCE_DESC::Buffer(BufferSize)
                , D3D12_RESOURCE_STATE_GENERIC_READ
                , nullptr
                , IID_PPV_ARGS(&Tex)));
            UploadHeapState[level][1]++;//这个Heap存Resource的数量
            ID.HeapIndex = level;
            ID.HeapOffset = offset;
            return ID;
        }
        return ID;
    }
    ResourceID createPlacedResourceInDefaultSFL(ID3D12Resource* Tex, D3D12_RESOURCE_DESC* TextureDesc) {
        UINT currentSize = minSizeInKB*1000;
        int level = 0;
        ResourceID ID = { -1,-1 };
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT  TexLayouts = {};
        UINT                                nNumRows = {};
        UINT64                              n64RowSizeInBytes = {};
        UINT64                              n64TotalBytes = 0;
        Device->GetCopyableFootprints(TextureDesc, 0, 1, 0, &TexLayouts, &nNumRows, &n64RowSizeInBytes, &n64TotalBytes);
        while (currentSize < n64TotalBytes) {
            currentSize *= 2;
            level++;
        }
        if (level > listNum - 1)
            return ID;
        int NumResource = pow(2, 3 - min(3, level));
        int SizeResource = pow(2, level) * GRS_UPPER(minSizeInKB, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)*1000;
        if (DefaultHeapState[level][0] == 0)
        {
            DefaultHeapState[level][0] = 1;
            D3D12_HEAP_DESC stDefaultHeapDesc = {  };
            stDefaultHeapDesc.Alignment = 0;
            stDefaultHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;		//上传堆类型
            stDefaultHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            stDefaultHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            stDefaultHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
            stDefaultHeapDesc.SizeInBytes = SizeResource * NumResource;
            ThrowIfFailed(Device->CreateHeap(&stDefaultHeapDesc, IID_PPV_ARGS(&DefaultHeaps[level])));
        }
        if (DefaultHeapState[level][1] < NumResource)
        {
            int offset;
            if (!DefaultHeapDeadLists[level].empty())
            {
                offset = DefaultHeapDeadLists[level].back();
                DefaultHeapDeadLists[level].pop_back();
            }
            else
                offset = DefaultHeapState[level][1];
            ThrowIfFailed(Device->CreatePlacedResource(
                DefaultHeaps[level].Get()
                , offset * SizeResource
                , TextureDesc
                , D3D12_RESOURCE_STATE_GENERIC_READ
                , nullptr
                , IID_PPV_ARGS(&Tex)));
            DefaultHeapState[level][1]++;
            ID.HeapIndex = level;
            ID.HeapOffset = offset;
            return ID;
        }
        return ID;
    }
private:
    ID3D12Device4* Device;
    EST::vector<ComPtr<ID3D12Heap>>UploadHeaps;
    EST::vector<ComPtr<ID3D12Heap>>DefaultHeaps;
    EST::vector<int>* UploadHeapDeadLists;
    EST::vector<int>* DefaultHeapDeadLists;
    UINT minSizeInKB;
    UINT listNum;
    EST::vector<int>*UploadHeapState;
    EST::vector<int>*DefaultHeapState;
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
        CreateDDSTextureFromFile12(device, cmdlist, fileName, Texture, TextureUpload);//这个库函数是用提交方式创建的，所以我们就让系统自己管理贴图资源,函数过程：loadDDS（）获得上传资源大小和已经创建好的默认堆资源，以及subresource，之后创建上传堆资源，之后updatesubresource
    }
    void createNON_RT_DS_WritableTex(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, D3D12_RESOURCE_DESC* TextureDesc, D3D12_HEAP_FLAGS flag, D3D12_CLEAR_VALUE* dsclear,NON_RT_DS_TextureSegregatedFreeLists*sfl) {
        if (isSticker)
            return;
        NON_RT_DS_SFL = sfl;
       ID= NON_RT_DS_SFL->createPlacedResourceInDefaultSFL(Texture.Get(), TextureDesc);
    }
    void createRT_DS_WritableTex(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, D3D12_RESOURCE_DESC* TextureDesc, D3D12_HEAP_FLAGS flag, D3D12_CLEAR_VALUE* dsclear, RT_DS_TextureSegregatedFreeLists* sfl) {
        if (isSticker)
            return;
        RT_DS_SFL = sfl;
        ID = RT_DS_SFL->createPlacedResourceInDefaultSFL(&Texture, TextureDesc,flag,dsclear);
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
    NON_RT_DS_TextureSegregatedFreeLists* NON_RT_DS_SFL = nullptr;
    RT_DS_TextureSegregatedFreeLists* RT_DS_SFL = nullptr;
    ResourceID ID;
    int RTVoffset;
    int DSVoffset;
    EST::vector<int>SRVOffsetList;
    EST::vector<int>UAVOffsetList;
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
    BufferResourceItem(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, T* strP, bool isstatic, ID3D12DescriptorHeap* cbvheap,uploadBuddySystem*upBS,defaultBuddySystem*defBS) {
        init(device, cmdlist, strP, isstatic, cbvheap,upBS,defBS);
    }
    void init(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, T* strP, bool isstatic, ID3D12DescriptorHeap* cbvheap, uploadBuddySystem* upbs, defaultBuddySystem* defbs) {
        this->isStatic = isstatic;
        this->CBVHeap = cbvheap;
        this->upBS = upbs;
        this->defBS = defbs;
        CBVoffset = HeapOffsetTable[CBVHeap];
        SRVCBVUAVincrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        this->str = *strP;
        D3D12_RANGE range = { 0,0 };
        strSize = sizeof(T) + 255 & ~255;
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.SizeInBytes = strSize;
        if (isStatic) {
           defID= defBS->createPlacedBufferResourceInBS(&constantBufferDefault, device,strSize);
           upID= upBS->createPlacedBufferResourceInBS(&constantBufferUpload, device,strSize);
            D3D12_SUBRESOURCE_DATA subResourceData;
            subResourceData.pData = &str;
            subResourceData.RowPitch = strSize;
            subResourceData.SlicePitch = subResourceData.RowPitch;
            UpdateSubresources<1>(cmdlist, constantBufferDefault.Get(), constantBufferUpload.Get(), 0, 0, 1, &subResourceData);//这个函数只需先创建两个资源，然后他会帮你完成Map memcpy、unmap copyregion等操作
            cbvDesc.BufferLocation = constantBufferDefault->GetGPUVirtualAddress();
            //upBS->free(upID);//不要释放上传堆，他还有用
        }
        else {
            upID=upBS->createPlacedBufferResourceInBS(&constantBufferUpload, device, strSize);
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
    uploadBuddySystem* upBS = nullptr;
    defaultBuddySystem* defBS = nullptr;
    buddyID upID;
    buddyID defID;
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
    void createDynamicGeo(ID3D12Device4* device, bool isStatic, EST::vector<Vertex>* vertices, EST::vector<std::uint16_t>* indices, uploadBuddySystem* upBS) {
        this->vertices = vertices;
        this->indices = indices;
        vbsize = (UINT)sizeof(Vertex) * (UINT)vertices->size();
        ibsize = (UINT)sizeof(std::uint16_t) * (UINT)indices->size();
        upBS->createPlacedBufferResourceInBS(&VertexBufferUpload, device, vbsize);//顶点缓冲及索引缓冲初始状态为common效率最佳

        upBS->createPlacedBufferResourceInBS(&IndexBufferUpload, device, ibsize);

        UINT8* vmapped = nullptr;UINT8* imapped = nullptr;
        D3D12_RANGE range = { 0,0 };
        VertexBufferUpload->Map(0, &range, reinterpret_cast<void**>(&vmapped));
        IndexBufferUpload->Map(0, &range, reinterpret_cast<void**>(&imapped));

        memcpy(vmapped, vertices->Getdata(), vbsize);
        memcpy(imapped, indices->Getdata(), ibsize);
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
    void createStaticGeo(ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, EST::vector<Vertex>* vertices, EST::vector<std::uint16_t>* indices, uploadBuddySystem* upBS, defaultBuddySystem* defBS) {//这之后必须excute
        createDynamicGeo(device, true, vertices, indices,upBS);
        defBS->createPlacedBufferResourceInBS(&VertexBufferDefault, device, vbsize);

        defBS->createPlacedBufferResourceInBS(&IndexBufferDefault, device, ibsize);
        D3D12_SUBRESOURCE_DATA subResourceData;
        subResourceData.pData = vertices->Getdata();
        subResourceData.RowPitch = vbsize;
        subResourceData.SlicePitch = subResourceData.RowPitch;
        UpdateSubresources<1>(cmdlist, VertexBufferDefault.Get(), VertexBufferUpload.Get(), 0, 0, 1, &subResourceData);
        subResourceData.pData = indices->Getdata();
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
    EST::vector<Vertex>* vertices;
    EST::vector<std::uint16_t>* indices;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
    ComPtr< ID3D12Resource> VertexBufferDefault;
    ComPtr< ID3D12Resource> VertexBufferUpload;
    ComPtr< ID3D12Resource> IndexBufferDefault;
    ComPtr< ID3D12Resource> IndexBufferUpload;

};
struct RenderItem {
    RenderItem() = default;
    RenderItem(GeometryItem* geo, int instancenum, int basevertex, int startindex, int indexnum, int startinstance, D3D12_PRIMITIVE_TOPOLOGY primitive, objectconstant* objc, ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, ID3D12DescriptorHeap* CBVHeap, uploadBuddySystem* upBS, defaultBuddySystem* defBS) {
        init(geo, instancenum, basevertex, startindex, indexnum, startinstance, primitive, objc, device, cmdlist, CBVHeap,upBS,defBS);
    }
    void init(GeometryItem* geo, int instancenum, int basevertex, int startindex, int indexnum, int startinstance, D3D12_PRIMITIVE_TOPOLOGY primitive, objectconstant* objc, ID3D12Device4* device, ID3D12GraphicsCommandList* cmdlist, ID3D12DescriptorHeap* CBVHeap, uploadBuddySystem* upBS, defaultBuddySystem* defBS) {
        Geo = geo;
        InstanceNum = instancenum;
        baseVertex = basevertex;
        startIndex = startindex;
        indexNum = indexnum;
        startInstance = startinstance;
        Primitive = primitive;
        auto ptr = std::make_unique< BufferResourceItem<objectconstant>>(device, cmdlist, objc, true, CBVHeap,upBS,defBS);//让渲染项管理一个资源项
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
    EST::vector<int>SRVDescriptorTableIndex;
    EST::vector<int>CBVDescriptorTableIndex;
    EST::vector<int>UAVDescriptorTableIndex;
    EST::vector<int>SamplerDescriptorTableIndex;
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