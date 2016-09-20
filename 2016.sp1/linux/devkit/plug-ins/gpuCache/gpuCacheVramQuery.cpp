//-
//**************************************************************************/
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

#include "gpuCacheVramQuery.h"

#include "gpuCacheGLFT.h"

#include <maya/MGlobal.h>
#include <maya/MHardwareRenderer.h>
#include <maya/MString.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#if defined(_WIN32)
    #define INITGUID
    #include <windows.h>
    #include <oleauto.h>
    #include <initguid.h>
    #include <wbemidl.h>
    #include <dxgi.h>
#elif defined(__APPLE__) || defined(__MACH__)
    #include <ApplicationServices/ApplicationServices.h>
#else
    #include <iostream>
    #include <fstream>
#endif

#include <maya/MGL.h>

using namespace GPUCache;


//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

namespace {

#if defined(_WIN32)

    typedef BOOL (WINAPI *PfnCoSetProxyBlanket)(
        IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc,
        OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel,
        RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities);

    class CoInitializeHelper
    {
    public:
        CoInitializeHelper() {
            fResult = CoInitialize(0);
        }
        ~CoInitializeHelper() {
            CoUninitialize();
        }
        operator bool() {
            return (SUCCEEDED(fResult));
        }
    private:
        HRESULT fResult;
    };

    class Win32LibraryHelper
    {
    public:
        Win32LibraryHelper(const wchar_t* library) {
            fModule = LoadLibraryW(library);
        }
        ~Win32LibraryHelper() {
            if (fModule) {
                FreeLibrary(fModule);
            }
        }
        operator HINSTANCE() const {
            return fModule;
        }
    private:
        HINSTANCE fModule;
    };

    template<class CoObjectType>
    class CoObjectCreator
    {
    public:
        virtual ~CoObjectCreator() {}
        virtual CoObjectType* operator() () const = NULL;
    };

    template<class CoObjectType>
    class CoObjectHelper
    {
    public:
        CoObjectHelper(const CoObjectCreator<CoObjectType>& creator) {
            fObject = creator();
        }
        CoObjectHelper(CoObjectType* object) {
            fObject = object;
        }
        ~CoObjectHelper() {
            if (fObject) {
                fObject->Release();
            }
        }
        CoObjectType* operator-> () const {
            return fObject;
        }
        operator bool() const {
            return (fObject != NULL);
        }
        operator CoObjectType*() const {
            return fObject;
        }
    private:
        CoObjectType* fObject;
    };

    class CoStringHelper
    {
    public:
        CoStringHelper(const wchar_t* str) {
            fString = SysAllocString(str);
        }
        ~CoStringHelper() {
            if (fString) {
                SysFreeString(fString);
            }
        }
        operator BSTR() const {
            return fString;
        }
    private:
        BSTR fString;
    };

    class WbemLocatorHelper : public CoObjectCreator<IWbemLocator>
    {
    public:
        virtual IWbemLocator* operator() () const {
            IWbemLocator* wbemLocator = NULL;
            HRESULT hres = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                IID_IWbemLocator, (LPVOID*)&wbemLocator);
            return SUCCEEDED(hres) ? wbemLocator : NULL;
        }
    };

    class WbemServicesHelper : public CoObjectCreator<IWbemServices>
    {
    public:
        WbemServicesHelper(IWbemLocator* wbemLocator) : fWbemLocator(wbemLocator) {}
        virtual IWbemServices* operator() () const {
            CoStringHelper ns(L"\\\\.\\root\\cimv2");
            IWbemServices* wbemServices = NULL;
            HRESULT hres = fWbemLocator->ConnectServer(ns, NULL, NULL, 0L, 0L, NULL, NULL, &wbemServices);
            return SUCCEEDED(hres) ? wbemServices : NULL;
        }
    private:
        IWbemLocator* fWbemLocator;
    };

    class EnumVideoCtrlHelper : public CoObjectCreator<IEnumWbemClassObject>
    {
    public:
        EnumVideoCtrlHelper(IWbemServices* wbemServices) : fWbemServices(wbemServices) {}
        virtual IEnumWbemClassObject* operator() () const {
            CoStringHelper className(L"Win32_VideoController");
            IEnumWbemClassObject* enumVideoControllers = NULL;
            HRESULT hres = fWbemServices->CreateInstanceEnum(className, 0, NULL, &enumVideoControllers);
            return SUCCEEDED(hres) ? enumVideoControllers : NULL;
        }
    private:
        IWbemServices* fWbemServices;
    };

    typedef HRESULT ( WINAPI* LPCREATEDXGIFACTORY )( REFIID, void** );

    class DXGIFactoryHelper : public CoObjectCreator<IDXGIFactory>
    {
    public:
        DXGIFactoryHelper(HINSTANCE dxgiModule) : fDXGIModule(dxgiModule) {}
        virtual IDXGIFactory* operator() () const {
            LPCREATEDXGIFACTORY createDXGIFactory = (LPCREATEDXGIFACTORY)
                GetProcAddress(fDXGIModule, "CreateDXGIFactory");
            if (!createDXGIFactory) {
                return NULL;
            }

            IDXGIFactory* dxgiFactory = NULL;
            createDXGIFactory(__uuidof(IDXGIFactory), (LPVOID*)&dxgiFactory);
            return dxgiFactory;
        }
    private:
        HINSTANCE fDXGIModule;
    };

    class DXGIAdapterHelper : public CoObjectCreator<IDXGIAdapter>
    {
    public:
        DXGIAdapterHelper(IDXGIFactory* dxgiFactory, UINT index)
            : fDXGIFactory(dxgiFactory), fIndex(index) {}
        virtual IDXGIAdapter* operator() () const {
            IDXGIAdapter* dxgiAdapter = NULL;
            HRESULT result = fDXGIFactory->EnumAdapters(fIndex, &dxgiAdapter);

            // End of enumeration
            if (FAILED(result)) {
                return NULL;
            }
            return dxgiAdapter;
        }

    private:
        IDXGIFactory* fDXGIFactory;
        UINT          fIndex;
    };

#endif

}

namespace GPUCache {

//==============================================================================
// CLASS VramQuery
//==============================================================================

//------------------------------------------------------------------------------
//
MUint64 VramQuery::queryVram()
{
    // Initialized in constructor
    return VramQuery::getInstance().fVram;
}


//------------------------------------------------------------------------------
//
bool VramQuery::isGeforce()
{
    // Initialized in constructor
    return VramQuery::getInstance().fIsGeforce;
}

//------------------------------------------------------------------------------
//
bool VramQuery::isQuadro()
{
    // Initialized in constructor
    return VramQuery::getInstance().fIsQuadro;
}

//------------------------------------------------------------------------------
//
const MString& VramQuery::manufacturer()
{
    // Initialized in constructor
    return VramQuery::getInstance().fManufacturer;
}

//------------------------------------------------------------------------------
//
const MString& VramQuery::model()
{
    // Initialized in constructor
    return VramQuery::getInstance().fModel;
}

//------------------------------------------------------------------------------
//
void VramQuery::driverVersion(int version[3])
{
    // Initialized in constructor
    const VramQuery& query = VramQuery::getInstance();
    version[0] = query.fDriverVersion[0];
    version[1] = query.fDriverVersion[1];
    version[2] = query.fDriverVersion[2];

    //assert(version[0] != 0 || version[1] != 0 || version[2] != 0);
}

//------------------------------------------------------------------------------
//
#if defined(_WIN32)
void VramQuery::queryVramAndDriverWMI(MUint64& vram, int driverVersion[3], MString& manufacturer, MString& model)
{
    vram = driverVersion[0] = driverVersion[1] = driverVersion[2] = 0;

    // Initialize COM
    CoInitializeHelper coInit;
    if (!coInit) {
        return;
    }

    // create WMI COM instance
    WbemLocatorHelper wbemLocatorCreator;
    CoObjectHelper<IWbemLocator> wbemLocator(wbemLocatorCreator);
    if (!wbemLocator) {
        return;
    }

    // connect to WMI
    WbemServicesHelper wbemServiceCreator(wbemLocator);
    CoObjectHelper<IWbemServices> wbemServices(wbemServiceCreator);
    if (!wbemServices) {
        return;
    }

    // switch security level to IMPERSONATE 
    Win32LibraryHelper ole32Library(L"ole32.dll");
    if (ole32Library) {
        PfnCoSetProxyBlanket pfnCoSetProxyBlanket = 
            (PfnCoSetProxyBlanket)GetProcAddress(ole32Library, "CoSetProxyBlanket");
        if (pfnCoSetProxyBlanket) {
            pfnCoSetProxyBlanket(wbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0);
        }
    }

    // create video controller enum
    EnumVideoCtrlHelper enumVideoCtrlCreator(wbemServices);
    CoObjectHelper<IEnumWbemClassObject> enumVideoCtrls(enumVideoCtrlCreator);
    if (!enumVideoCtrls) {
        return;
    }

    // get the first 10 video controllers
    IWbemClassObject* videoCtrls[10] = {0};
    DWORD returned = 0;
    enumVideoCtrls->Reset();
    HRESULT hres = enumVideoCtrls->Next(5000, 10, videoCtrls, &returned);
    if (FAILED(hres) || returned == 0) {
        return;
    }

    // query the video memory
    VARIANT var;
    VariantClear(&var);
    CoStringHelper vramPropName(L"AdapterRAM");
    CoStringHelper compatPropName(L"AdapterCompatibility");
    CoStringHelper driverVersionPropName(L"DriverVersion");
    CoStringHelper modelPropName(L"Name");
    MUint64 maxVidMem = 0;
    MString driverVersionStr;

    for (UINT ctrlIndex = 0; ctrlIndex < returned; ctrlIndex++) {
        CoObjectHelper<IWbemClassObject> videoCtrl(videoCtrls[ctrlIndex]);
        hres = videoCtrl->Get(vramPropName, 0L, &var, NULL, NULL);
        if (SUCCEEDED(hres)) {
            if (var.ulVal > maxVidMem) {
                maxVidMem = var.ulVal;

                VariantClear(&var);
                videoCtrl->Get(compatPropName, 0L, &var, NULL, NULL);
                manufacturer = var.bstrVal;

                VariantClear(&var);
                videoCtrl->Get(driverVersionPropName, 0L, &var, NULL, NULL);
                driverVersionStr = var.bstrVal;

                VariantClear(&var);
                videoCtrl->Get(modelPropName, 0L, &var, NULL, NULL);
                model = var.bstrVal;

            }
        }
        VariantClear(&var);
    }

    vram = maxVidMem;
    if (manufacturer == "NVIDIA"
        || manufacturer == "NVIDIA " // beta drivers
        ) {
        // e.g. 8.17.12.8026 = 280.26
        MStringArray versions;
        driverVersionStr.split('.', versions);
        if (versions.length() == 4) {
            unsigned int numChars2 = versions[2].numChars();
            unsigned int numChars3 = versions[3].numChars();
            if (numChars2 >= 1 && numChars3 >= 2) {
                while (numChars3 < 4) {
                    // If it has less than 4 digits, patch it with leading zeros.
                    // e.g. 9.18.13.529 = 305.29
                    versions[3] = MString("0") + versions[3];
                    numChars3++;
                }

                MString major1 = versions[2].substringW(numChars2-1, numChars2-1);
                MString major2 = versions[3].substringW(0, 1);
                MString major = major1 + major2;
                MString minor = versions[3].substringW(2, numChars3-1);
                if (major.isUnsigned() && minor.isUnsigned()) {
                    driverVersion[0] = major.asUnsigned();
                    driverVersion[1] = minor.asUnsigned();
                }
            }
        }
    }
    else if (manufacturer == "ATI Technologies Inc." || 
        manufacturer == "Advanced Micro Devices, Inc.") {
        // e.g. 8.861.0.0 = 8.861
        int version[4], ret;
        ret = sscanf_s(driverVersionStr.asChar(), "%d.%d.%d.%d",
                &version[0], &version[1], &version[2], &version[3]);
        if (ret == 4) {
            driverVersion[0] = version[0];
            driverVersion[1] = version[1];
        }
    }
}

MUint64 VramQuery::queryVramDXGI()
{
    // Initialize COM
    CoInitializeHelper coInit;
    if (!coInit) {
        return 0;
    }

    // Load dxgi.dll (need Vista or later)
    Win32LibraryHelper dxgiLibrary(L"dxgi.dll");
    if (!dxgiLibrary) {
        return 0;
    }

    // Create DXGI Factory
    DXGIFactoryHelper dxgiFactoryCreator(dxgiLibrary);
    CoObjectHelper<IDXGIFactory> dxgiFactory(dxgiFactoryCreator);
    if (!dxgiFactory) {
        return 0;
    }
    
    MUint64 maxVidMem = 0;
    for (UINT index = 0; ; ++index) {
        DXGIAdapterHelper dxgiAdapterCreator(dxgiFactory, index);
        CoObjectHelper<IDXGIAdapter> dxgiAdapter(dxgiAdapterCreator);
        if (!dxgiAdapter) {
            break;
        }

        DXGI_ADAPTER_DESC dxgiAdapterDesc;
        ZeroMemory(&dxgiAdapterDesc, sizeof(DXGI_ADAPTER_DESC));
        
        HRESULT result = dxgiAdapter->GetDesc(&dxgiAdapterDesc);
        if (SUCCEEDED(result)) {
            SIZE_T vidMem = dxgiAdapterDesc.DedicatedVideoMemory;
            maxVidMem = (vidMem > maxVidMem) ? vidMem : maxVidMem;
        }
    }

    return maxVidMem;
}

#elif defined(__APPLE__) || defined(__MACH__)
void VramQuery::queryVramAndDriverMAC(MUint64& vram, int driverVersion[3], MString& manufacturer, MString& model)
{
    vram = 0;
    driverVersion[0] = driverVersion[1] = driverVersion[2] = 0;

    CGError res = CGDisplayNoErr;

    // query active displays
    CGDisplayCount dspCount = 0;
    res = CGGetActiveDisplayList(0, NULL, &dspCount);
    if (res || dspCount == 0) {
        return;
    }

    // use boost here
    CGDirectDisplayID* displays = (CGDirectDisplayID*)calloc((size_t)dspCount, sizeof(CGDirectDisplayID));
    res = CGGetActiveDisplayList(dspCount, displays, &dspCount);
    if (res || dspCount == 0) {
        return;
    }

    SInt64 maxVramTotal = 0;

    for (int i = 0; i < dspCount; i++) {
        // get the service port for the display
        io_service_t dspPort = CGDisplayIOServicePort(displays[i]);

        // ask IOKit for the VRAM size property
        /* HD 2600: IOFBMemorySize = 256MB. VRAM,totalsize = 256MB
           HD 5770: IOFBMemorySize = 512MB. VRAM,totalsize = 1024MB
           Apple's QA page is not correct. We should search for IOPCIDevice's VRAM,totalsize property.
        CFTypeRef typeCode = IORegistryEntryCreateCFProperty(dspPort,
            CFSTR(kIOFBMemorySizeKey),
            kCFAllocatorDefault,
            kNilOptions);
        */
        SInt64 vramScale = 1;
        CFTypeRef typeCode = IORegistryEntrySearchCFProperty(dspPort,
            kIOServicePlane,
            CFSTR("VRAM,totalsize"),
            kCFAllocatorDefault,
            kIORegistryIterateRecursively | kIORegistryIterateParents);

        if (!typeCode) {
            // On the new Mac Pro, we have VRAM,totalMB instead.
            typeCode = IORegistryEntrySearchCFProperty(dspPort,
                kIOServicePlane,
                CFSTR("VRAM,totalMB"),
                kCFAllocatorDefault,
                kIORegistryIterateRecursively | kIORegistryIterateParents);
            if (typeCode) {
                vramScale = 1024 * 1024;
            }
        }

        // ensure we have valid data from IOKit
        if (typeCode) {
            SInt64 vramTotal = 0;
            if (CFGetTypeID(typeCode) == CFNumberGetTypeID()) {
                // AMD, VRAM,totalsize is CFNumber
                CFNumberGetValue((const __CFNumber*)typeCode, kCFNumberSInt64Type, &vramTotal);
            }
            else if (CFGetTypeID(typeCode) == CFDataGetTypeID()) {
                // NVIDIA, VRAM,totalsize is CFData
                CFIndex      length = CFDataGetLength((const __CFData*)typeCode);
                const UInt8* data   = CFDataGetBytePtr((const __CFData*)typeCode);
                if (length == 4) {
                    vramTotal = *(const unsigned int*)data;
                }
                else if (length == 8) {
                    vramTotal = *(const SInt64*)data;
                }
            }
            vramTotal *= vramScale;
            CFRelease(typeCode);
            
            if (vramTotal > maxVramTotal) {
                maxVramTotal = vramTotal;

                typeCode = IORegistryEntrySearchCFProperty(dspPort,
                            kIOServicePlane,
                            CFSTR("NVDA,Features"),
                            kCFAllocatorDefault,
                            kIORegistryIterateRecursively | kIORegistryIterateParents);
                if (typeCode) {
                    manufacturer = MString("NVIDIA");
                    CFRelease(typeCode);
                }

                typeCode = IORegistryEntrySearchCFProperty(dspPort,
                            kIOServicePlane,
                            CFSTR("ATY,Copyright"),
                            kCFAllocatorDefault,
                            kIORegistryIterateRecursively | kIORegistryIterateParents);
                if (typeCode) {
                    manufacturer = MString("Advanced Micro Devices, Inc.");
                    CFRelease(typeCode);
                }

                // GPU model
                typeCode = IORegistryEntrySearchCFProperty(dspPort,
                            kIOServicePlane,
                            CFSTR("model"),
                            kCFAllocatorDefault,
                            kIORegistryIterateRecursively | kIORegistryIterateParents);
                if (typeCode) {
                    if (CFGetTypeID(typeCode) == CFDataGetTypeID()) {
                        model = MString((const char*)CFDataGetBytePtr((const __CFData*)typeCode));
                    }
                    CFRelease(typeCode);
                }
            }
        }
    }

    vram = (MUint64)maxVramTotal;

    // Query the display driver version.
    // e.g. 2.1 NVIDIA-7.2.9
    // e.g. 1.5 ATI-1.4.18
    const char* glVersion = (const char*)gGLFT->glGetString(MGL_VERSION);
    if (glVersion) {
        const char* implVersion = strstr(glVersion, "-");
        if (implVersion) {
            int version[3], ret;
            ret = sscanf(implVersion+1, "%d.%d.%d",
                &version[0], &version[1], &version[2]);
            if (ret == 3) {
                driverVersion[0] = version[0];
                driverVersion[1] = version[1];
                driverVersion[2] = version[2];
            }
        }
    }
}

#else
void VramQuery::queryVramAndDriverXORG(MUint64& vram, int driverVersion[3], MString& manufacturer, MString& model)
{
    vram = 0;
    driverVersion[0] = driverVersion[1] = driverVersion[2] = 0;

    // open the Xorg.0.log
    std::string   line;
    std::ifstream xorgLog("/var/log/Xorg.0.log");
    if (!xorgLog.is_open()) {
        return;
    }

    int maxVidMemKb = 0;
    int version[3] = {0, 0, 0}, versionSize = 0;
    while (xorgLog.good()) {
        // read a line
        std::getline(xorgLog, line);

        // Try to detect card model in the pattern of the following line:
        // "(--) PCI:*(0:15:0:0) 10de:061a:10de:055f nVidia Corporation G92 [Quadro FX 3700] rev 162, Mem @ 0xfa000000/16777216, 0xe0000000/268435456, 0xf8000000/33554432, I/O @ 0x0000d000/128"
        size_t initPos = line.find("(--) PCI:");
        if (model.length() == 0 &&
            initPos != std::string::npos &&
            line.find("Mem @") != std::string::npos &&
            line.find("I/O @") != std::string::npos
            ) {
            size_t start = line.find("[", initPos);

            if (start != std::string::npos) {
                // NVIDIA
                size_t end = line.find("]", start);
                model = MString(line.substr(start + 1, end - start - 1).c_str());
                manufacturer = MString("NVIDIA");
            }
            else {
                start = line.find("ATI Technologies Inc", initPos);
                if (start != std::string::npos) {
                    // AMD
                    size_t end0 = line.find(" (", start);
                    size_t end1 = line.find(", Mem @", start);
                    size_t end = (end0 != std::string::npos && end0 < end1) ? end0 : end1; 
                    model = MString(line.substr(start + 20, end - start - 20).c_str());
                    manufacturer = MString("Advanced Micro Devices, Inc.");
                }
            }
        }

        // Try to detect card model in the pattern of the following line:
        // "(II) NVIDIA(0): NVIDIA GPU Quadro 4000 (GF100GL) at PCI:1:1:0 (GPU-0)"
        if (model.length() == 0) {
            size_t start = line.find("NVIDIA GPU ");
            if (start != std::string::npos &&
                line.find("NVIDIA(") != std::string::npos) {
                size_t end0 = line.find(" (", start);
                size_t end1 = line.find(" at", start);
                size_t end = (end0 != std::string::npos && end0 < end1) ? end0 : end1; 
                model = MString(line.substr(start + 11, end - start - 11).c_str());
                manufacturer = MString("NVIDIA");
            }
        }

        // find VRAM
        size_t startOffset = std::string::npos;
        size_t endOffset   = std::string::npos;
        if (line.find("NVIDIA") != std::string::npos) {
            // found NVIDIA
            startOffset = line.find("Memory:") + 7;
            endOffset   = line.find("kBytes");
        }

        if (startOffset == std::string::npos ||
                endOffset == std::string::npos) {
            // possible AMD
            startOffset = line.find("Video RAM:") + 10;
            endOffset   = line.find("kByte");
        }

        if (startOffset != std::string::npos &&
                endOffset != std::string::npos) {
            // parse the number between start and end
            std::string strVidMem = line.substr(startOffset, endOffset - startOffset);
            int vidMemKb = atoi(strVidMem.c_str());
            maxVidMemKb  = (vidMemKb > maxVidMemKb) ? vidMemKb : maxVidMemKb;
        }

        // find driver version
        const char* driver = NULL;
        if ((driver = strstr(line.c_str(), "NVIDIA dlloader X Driver"))) {
            versionSize = sscanf(driver+24, "%d.%d.%d",
                &version[0], &version[1], &version[2]);
            manufacturer = MString("NVIDIA");
        }
        else if ((driver = strstr(line.c_str(), "ATI Proprietary Linux Driver Release Identifier:"))) {
            versionSize = sscanf(driver+48, "%d.%d",
                &version[0], &version[1]);
            manufacturer = MString("Advanced Micro Devices, Inc.");
        }
    }

    vram = MUint64(maxVidMemKb) * 1024;
    for (int i = 0; i < versionSize; i++) {
        driverVersion[i] = version[i];
    }
}

#endif


//------------------------------------------------------------------------------
//
MUint64 VramQuery::queryVramOGL()
{
    // Query Vram by OpenGL extensions.
    // This function needs an OpenGL context.
    if (gGLFT->extensionExists(kMGLext_NVX_gpu_memory_info)) {
        // NVIDIA GL_NVX_gpu_memory_info exists
        MGLint dedicatedVidMem = 0;
        gGLFT->glGetIntegerv(MGL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &dedicatedVidMem);
        return MUint64(dedicatedVidMem) * 1024;
    }
    else if (gGLFT->extensionExists(kMGLext_ATI_meminfo)) {
        // AMD GL_ATI_meminfo exists
        MGLint freeVBOMem[4] = {0, 0, 0, 0};
        gGLFT->glGetIntegerv(MGL_VBO_FREE_MEMORY_ATI, freeVBOMem);
        return MUint64(freeVBOMem[0]) * 1024;
    }

    return 0;
}


//------------------------------------------------------------------------------
//
bool VramQuery::isGeforceOGL()
{
    // Query the renderer by glGetString.
    // This function needs an OpenGL context
    const char* renderer = (const char*)gGLFT->glGetString(MGL_RENDERER);
    return (renderer && strstr(renderer, "GeForce"));
}


//------------------------------------------------------------------------------
//
bool VramQuery::isQuadroOGL()
{
    // Query the renderer by glGetString.
    // This function needs an OpenGL context
    const char* renderer = (const char*)gGLFT->glGetString(MGL_RENDERER);
    return (renderer && strstr(renderer, "Quadro"));
}


//------------------------------------------------------------------------------
//
const VramQuery& VramQuery::getInstance()
{
    // singleton, init on function call needs an OpenGL context
    static VramQuery query;
    return query;
}


//------------------------------------------------------------------------------
//
VramQuery::VramQuery()
    : fVram(0),
      fIsGeforce(false),
      fIsQuadro(false)
{
    fDriverVersion[0] = fDriverVersion[1] = fDriverVersion[2] = 0;

    if (MGlobal::kInteractive == MGlobal::mayaState()) {
        InitializeGLFT();
        MUint64 vram = 0;
        int     driverVersion[3] = {0, 0, 0};
        MString manufacturer;
        MString model;

#if defined(_WIN32)
        // Windows, let's query VRAM via WMI
        // Please see the Video Memory sample in DirectX SDK
        // http://msdn.microsoft.com/en-us/library/ee419018%28v=vs.85%29.aspx
        MUint64 vramDXGI = VramQuery::queryVramDXGI();
        VramQuery::queryVramAndDriverWMI(vram, driverVersion, manufacturer, model);
        if (vramDXGI != 0) {
            vram = vramDXGI;  // DXGI can detect VRAM over 4G
        }
#elif defined(__APPLE__) || defined(__MACH__)
        // Mac OSX, let's query VRAM via Core Graphics and IOKit
        // http://developer.apple.com/library/mac/#qa/qa1168/_index.html 
        VramQuery::queryVramAndDriverMAC(vram, driverVersion, manufacturer, model);
#else
        // Linux, let's parse Xorg.0.log
        VramQuery::queryVramAndDriverXORG(vram, driverVersion, manufacturer, model);
#endif

        // the platform specific query failed
        // http://www.opengl.org/registry/specs/ATI/meminfo.txt 
        // http://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt 
        if (vram == 0) {
            vram = VramQuery::queryVramOGL();
        }
    
        // everything failed.. use a predefined value: 1G
        if (vram == 0) {
            vram = 1 << 30;
        }

        fVram      = vram;
        fDriverVersion[0] = driverVersion[0];
        fDriverVersion[1] = driverVersion[1];
        fDriverVersion[2] = driverVersion[2];
        fIsGeforce = VramQuery::isGeforceOGL();
        fIsQuadro  = VramQuery::isQuadroOGL();
        fManufacturer = manufacturer;
        fModel = model;
    }
}

}

