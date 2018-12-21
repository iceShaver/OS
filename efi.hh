#ifndef EFI_HH
#define EFI_HH
extern "C"
{
#include <efi.h>
#include <efilib.h>
#include <efiprot.h>
#include <efibind.h>
#include <efidef.h>
#include <efidevp.h>
#include <eficon.h>
#include <efiapi.h>
#include <efierr.h>
}
#include <cstring>
constexpr auto const KERNEL_FILENAME = L".\\kernel.bin";

class EFI
{
  public:
    EFI(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable);
    ~EFI(){while (true);}
    EFI_STATUS main();
    EFI_STATUS listFiles();
    void* loadKernel();
    EFI_STATUS ExitBootServices();
    bool error();

  private:
    EFI_HANDLE imageHandle;
    EFI_SYSTEM_TABLE *systemTable;
    EFI_LOADED_IMAGE *loadedImage;
    EFI_STATUS status;
    
};

EFI::EFI(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) : imageHandle(imageHandle), systemTable(systemTable)
{
    InitializeLib(this->imageHandle, this->systemTable);

    // check protocol
    status = uefi_call_wrapper(this->systemTable->BootServices->HandleProtocol, 3,
                        this->imageHandle,
                        &LoadedImageProtocol,
                        &this->loadedImage);
    if (error()) return;

    // Disable watchdog timer
    status = uefi_call_wrapper(systemTable->BootServices->SetWatchdogTimer, 4,
                        0, 0, 0, NULL);
    if(error()) return;
    Print(L"Image base: 0x%lx\n", loadedImage->ImageBase);
};

EFI_STATUS EFI::main()
{
    auto kernelMemoryLocation = loadKernel();
    ExitBootServices();
    
    // invoke kernel code
    typedef void (*fptr)(void);
    ((fptr)kernelMemoryLocation)();
    return EFI_SUCCESS;
}


EFI_STATUS EFI::ExitBootServices(){
    Print(L"Exiting boot services\n");
    // Allocate memory for memory map
    UINTN memoryMapSize = 4096;
    EFI_MEMORY_DESCRIPTOR memoryMap[memoryMapSize];
    UINTN mapKey;
    UINTN descriptorSize;
    UINT32 descriptorVersion;
    Print(L"Getting memory map\n");
    status = uefi_call_wrapper(
        systemTable->BootServices->GetMemoryMap, 5,
        &memoryMapSize,
        memoryMap,
        &mapKey,
        &descriptorSize,
        &descriptorVersion
    );

    if(error()) return status;
    // Print(L"Memory map addr: %u\n", memoryMap);
    // Print(L"Memory map size: %u\n", memoryMapSize);
    // Print(L"Map key: %u\n", mapKey);
    // Print(L"Descriptor size: %u\n", descriptorSize);

    // Exit boot services (can't do anything memory related between GetMemoryMap and ExitBootServices)
    status = uefi_call_wrapper(
        systemTable->BootServices->ExitBootServices, 2,
        imageHandle,
        mapKey
    );
    if(error()) return status;
    return EFI_SUCCESS;
}

EFI_STATUS EFI::listFiles()
{
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs;
    EFI_FILE_PROTOCOL *root = nullptr;
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiSimpleFileSystemProtocolGuid, nullptr, (VOID **)&sfs);
    if (error())
        return status;
    status = uefi_call_wrapper(sfs->OpenVolume, 2, sfs, &root);
    for (;;)
    {
        UINTN buff_size = 0;
        status = uefi_call_wrapper(root->Read, 3, root, &buff_size, nullptr);
        if (status == EFI_BUFFER_TOO_SMALL)
        {
            Print(L"Allocating buffer\n");
            EFI_FILE_INFO *file_info;
            status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiBootServicesCode, buff_size, (VOID **)&file_info);
            if (error())
                return status;
            status = uefi_call_wrapper(root->Read, 3, root, &buff_size, file_info);
            if (error())
                return status;
            Print(L"Size: %d\nFileSize: %d \nPhysicalSize: %d %s\n",
                  file_info->Size, file_info->FileSize, file_info->PhysicalSize, file_info->FileName);
            Print(L"%s\n", file_info->FileName);
            status = uefi_call_wrapper(gBS->FreePool, 1, file_info);
        }
        if (buff_size = 0)
            return status;
    }
    return status;
}

void* EFI::loadKernel() {
    Print(L"Loading kernel\n");
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = nullptr;
    EFI_FILE_PROTOCOL *root = nullptr;
    Print(L"Getting protocol for sfs\n");
    // Get protocol for sfs
    status = uefi_call_wrapper(
        BS->LocateProtocol, 3,
        &gEfiSimpleFileSystemProtocolGuid,
        nullptr,
        &sfs
    );
    if (error()) return;

    Print(L"Opening volume\n");
    // Open volume 
    status = uefi_call_wrapper(
        sfs->OpenVolume, 2,
        sfs,
        &root
    );
    if (error()) return;
    
    Print(L"Opening kernel file\n");
    // Open kernel file
    EFI_FILE_PROTOCOL *kernelFile = nullptr;
    status = uefi_call_wrapper(
        root->Open, 5,
        root,
        &kernelFile,
        KERNEL_FILENAME,
        EFI_FILE_MODE_READ,
        nullptr
    );
    
    // GetInfo
    EFI_FILE_INFO *kernelFileInfo;
    UINTN buffSize = sizeof(kernelFileInfo);
    status = uefi_call_wrapper(
        kernelFile->GetInfo, 4,
        kernelFile,
        &gEfiFileInfoGuid,
        &buffSize,
        nullptr
    );
    if (status == EFI_BUFFER_TOO_SMALL)
    {
        Print(L"Buffer to small for kernel file info. Allocating...\n");
        status = uefi_call_wrapper(
            BS->AllocatePool, 3,
            EfiBootServicesCode,
            buffSize, 
            &kernelFileInfo
        );
        if(error()) return;
        Print(L"Loading kernel info\n");
        status = uefi_call_wrapper(
            kernelFile->GetInfo, 4,
            kernelFile,
            &gEfiFileInfoGuid,
            &buffSize,
            kernelFileInfo
        );
        if (error())
            return;
    };

    auto kernelSize = kernelFileInfo->Size;
    Print(L"Allocating memory for kernel file\n");
    void* kernelMemoryLocation = nullptr;
    status = uefi_call_wrapper(
        BS->AllocatePool, 3,
        EfiBootServicesCode,
        kernelSize,
        &kernelMemoryLocation
    );
    if (error()) return;

    Print(L"Loading kernel file\n");
    
    status = uefi_call_wrapper(
        kernelFile->Read, 3,
        kernelFile,
        &kernelSize,
        kernelMemoryLocation
    );
    if(error()) return;
    Print(L"Closing volume\n");
    // Close root
    status = uefi_call_wrapper(
        root->Close, 1,
        root
    );
    if (error()) return;
    Print(L"Successfully loaded kernel of size %u byte(s) at location: 0x%X\n",kernelSize, kernelMemoryLocation);
    return kernelMemoryLocation;
}

bool EFI::error()
{
    if(EFI_ERROR(this->status)){
        Print(L"Error: %d\n", this->status);
        return true;
    }

    return false;
}
#endif // !EFI_HH
