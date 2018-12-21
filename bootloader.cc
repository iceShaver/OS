#include "bootloader.hh"


Bootloader::Bootloader(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) 
    : imageHandle(imageHandle), systemTable(systemTable) {
    InitializeLib(this->imageHandle, this->systemTable);

    // check protocol
    status = uefi_call_wrapper(BS->HandleProtocol, 3,
                        this->imageHandle,
                        &LoadedImageProtocol,
                        &this->loadedImage);
    if (error()) return;

    // Disable watchdog timer
    status = uefi_call_wrapper(BS->SetWatchdogTimer, 4,
                        0, 0, 0, NULL);
    if(error()) return;
};


EFI_STATUS Bootloader::main() {
    auto kernelMemoryLocation = loadKernel();
    if(error()) return status;
    exitBootServices();
    // invoke kernel code
    typedef void (*fptr)(void);
    ((fptr)kernelMemoryLocation)();

    return EFI_SUCCESS;
}


EFI_STATUS Bootloader::exitBootServices(){
    // Allocate memory for memory map
    UINTN memoryMapSize = 4096;
    EFI_MEMORY_DESCRIPTOR memoryMap[memoryMapSize];
    UINTN mapKey;
    UINTN descriptorSize;
    UINT32 descriptorVersion;
    Print(L"Getting memory map and exiting boot services\n");
    status = uefi_call_wrapper(
        BS->GetMemoryMap, 5,
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

    // Exit boot services (can't do anything memory related between GetMemoryMap and exitBootServices)
    status = uefi_call_wrapper(
        BS->ExitBootServices, 2,
        imageHandle,
        mapKey
    );
    if(error()) return status;
    return EFI_SUCCESS;
}

EFI_STATUS Bootloader::setTextMode(){

}

void* Bootloader::loadKernel() {
    Print(L"Loading kernel\n");
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = nullptr;
    EFI_FILE_PROTOCOL *root = nullptr;
    
    // Get protocol for sfs
    Print(L"Getting protocol for sfs\n");
    status = uefi_call_wrapper(
        BS->LocateProtocol, 3,
        &gEfiSimpleFileSystemProtocolGuid,
        nullptr,
        &sfs
    );
    if (error()) return;

    // Open volume 
    Print(L"Opening volume\n");
    status = uefi_call_wrapper(
        sfs->OpenVolume, 2,
        sfs,
        &root
    );
    if (error()) return;
    
    // Open kernel file
    Print(L"Opening kernel file\n");
    EFI_FILE_PROTOCOL *kernelFile = nullptr;
    status = uefi_call_wrapper(
        root->Open, 5,
        root,
        &kernelFile,
        KERNEL_FILENAME,
        EFI_FILE_MODE_READ,
        nullptr
    );
    
    // get GetInfo size
    EFI_FILE_INFO *kernelFileInfo;
    UINTN buffSize = 0;
    status = uefi_call_wrapper(
        kernelFile->GetInfo, 4,
        kernelFile,
        &gEfiFileInfoGuid,
        &buffSize,
        nullptr
    );

    // Alloc mem for kernel file info
    Print(L"Allocating memory for kernel file info\n");
    status = uefi_call_wrapper(
        BS->AllocatePool, 3,
        EfiBootServicesCode,
        buffSize, 
        &kernelFileInfo
    );
    if(error()) return;

    // load kernel info
    Print(L"Loading kernel info\n");
    status = uefi_call_wrapper(
        kernelFile->GetInfo, 4,
        kernelFile,
        &gEfiFileInfoGuid,
        &buffSize,
        kernelFileInfo
    );
    if (error()) return;

    // Alloc mem for kernel
    auto kernelSize = kernelFileInfo->Size;
    Print(L"Allocating memory for kernel\n");
    void* kernelMemoryLocation = nullptr;
    status = uefi_call_wrapper(
        BS->AllocatePool, 3,
        EfiBootServicesCode,
        kernelSize,
        &kernelMemoryLocation
    );
    if (error()) return;

    // load kernel
    Print(L"Loading kernel\n");
    status = uefi_call_wrapper(
        kernelFile->Read, 3,
        kernelFile,
        &kernelSize,
        kernelMemoryLocation
    );
    if(error()) return;

    // Close kernel file and root
    Print(L"Closing kernel file\n");
    status = uefi_call_wrapper(
        kernelFile->Close, 1,
        kernelFile
    );
    if (error()) return;
    Print(L"Closing volume\n");
    status = uefi_call_wrapper(
        root->Close, 1,
        root
    );
    if (error()) return;

    Print(L"Successfully loaded kernel of size %u byte(s) at location: 0x%X\n",
        kernelSize,
        kernelMemoryLocation
     );

    return kernelMemoryLocation;
}


bool Bootloader::error(){
    if(EFI_ERROR(this->status)){
        Print(L"Error: %d\n", this->status);
        return true;
    }
    return false;
}




