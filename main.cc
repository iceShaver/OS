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
#include "efi.hh"
EFI_STATUS efi_cpp_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table);

extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
    return efi_cpp_main(image_handle, system_table);
}

EFI_STATUS efi_cpp_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table){
    //uefi_call_wrapper(ST-> ConOut -> OutputString, 2, ST -> ConOut, L"Hello world!\n" );
    return EFI(image_handle, system_table).main();
}