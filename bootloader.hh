#ifndef BOOTLOADER_HH
#define BOOTLOADER_HH
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



class Bootloader
{
  public:
    Bootloader(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable);
    ~Bootloader(){ while (true); }
    EFI_STATUS main();
    void* loadKernel();
    EFI_STATUS exitBootServices();
    EFI_STATUS setTextMode();
    bool error();

  private:
    EFI_HANDLE imageHandle;
    EFI_SYSTEM_TABLE *systemTable;
    EFI_LOADED_IMAGE *loadedImage;
    EFI_STATUS status;
    
};
#endif // !BOOTLOADER_HH