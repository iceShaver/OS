#ifndef EFI_HH
#define EFI_HH 
extern "C" {
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
class EFI {
public:

    EFI(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) : image_handle(image_handle), system_table(system_table){
        InitializeLib(image_handle, system_table);
    };
    EFI_STATUS main(){
        this->print_string(L"Hello cpp world!!!\n\r");
        this->load_system_file();
        __asm__("hlt");
        return EFI_SUCCESS;
    }
    void print_string(CHAR16 *str)
    {
        uefi_call_wrapper(ST-> ConOut -> OutputString, 2, ST -> ConOut, str);
    }
    EFI_STATUS load_system_file(){
        EFI_STATUS result = 0;
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs;
        EFI_FILE_PROTOCOL *root = nullptr;
        result = uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiSimpleFileSystemProtocolGuid, nullptr, (VOID **)&sfs);
        if(EFI_ERROR(result)) return result;
        result = uefi_call_wrapper(sfs->OpenVolume, 2, sfs, &root);
        for (;;)
        {
            UINTN buff_size = 0;
            result = uefi_call_wrapper(root->Read, 3, root, &buff_size, nullptr);
            if(result == EFI_BUFFER_TOO_SMALL) {
                Print(L"Allocating buffer\n");
                EFI_FILE_INFO *file_info;
                result = uefi_call_wrapper(gBS->AllocatePool, 3, EfiBootServicesCode, buff_size, (VOID **)&file_info);
                if(EFI_ERROR(result)) return error(result);
                result = uefi_call_wrapper(root->Read, 3, root, &buff_size, file_info);
                if(EFI_ERROR(result)) return error(result);
                Print(L"Size: %d\nFileSize: %d \nPhysicalSize: %d %s\n",
                    file_info->Size, file_info->FileSize, file_info->PhysicalSize, file_info->FileName);
                Print(L"%s\n", file_info->FileName);
                result = uefi_call_wrapper(gBS->FreePool, 1, file_info);
            
            }
            if(buff_size=0) return result;
        }
        return result;
    }
    EFI_STATUS error(EFI_STATUS s){
        Print(L"Error: %d\n", s);
        return s;
    }

  private:
    EFI_HANDLE image_handle;
    EFI_SYSTEM_TABLE *system_table;
};
#endif // !EFI_HH