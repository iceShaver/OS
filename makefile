SHELL := /bin/bash
ARCH            = $(shell uname -m | sed s,i[3456789]86,ia32,)

OBJS            = main.o
TARGET          = hello.efi
OUTPUT_DIR	= hda_contents
IMG_NAME	= img.img
IMG_SIZE 	= 100M

EFIINC          = /usr/include/efi
EFIINCS         = -I$(EFIINC) -I$(EFIINC)/$(ARCH) #-I$(EFIINC)/protocol
LIB             = /usr/lib
EFILIB          = /usr/lib
EFI_CRT_OBJS    = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS         = $(EFILIB)/elf_$(ARCH)_efi.lds

CFLAGS          = $(EFIINCS)  -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall 
CXXFLAGS          = $(EFIINCS)  -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall -fpermissive
ifeq ($(ARCH),x86_64)
  CFLAGS += -DEFI_FUNCTION_WRAPPER
  CXXFLAGS += -DEFI_FUNCTION_WRAPPER
endif

LDFLAGS         = -nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS)


qemu: build
	qemu-system-x86_64 -pflash bios.bin -drive format=raw,file=$(OUTPUT_DIR) -net none


img: build
	sh create_img.sh $(IMG_NAME) $(IMG_SIZE) $(OUTPUT_DIR)

all: build

build: $(TARGET)
	mkdir -p $(OUTPUT_DIR) && cp $^ $(OUTPUT_DIR)

%.efi: %.so
	objcopy -j .text -j .sdata -j .data -j .dynamic \
		-j .dynsym  -j .rel -j .rela -j .reloc \
		--target=efi-app-$(ARCH) $^ $@

hello.so: $(OBJS)
	ld $(LDFLAGS) $(OBJS) -o $@ -lefi -lgnuefi


clean:
	rm hello.efi hello.so main.o