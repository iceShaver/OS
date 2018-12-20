#!/usr/bin/env bash
OUTPUT_DIR=hda_contents
IMG_NAME=$1
IMG_SIZE=100M
if [ -z "$IMG_NAME" ]
then
	echo "Bad arguments"
	exit 1
fi
for src in *.cc; do
	g++ ${src} -c -fno-stack-protector -fpic -fshort-wchar -fpermissive -mno-red-zone -DEFI_FUNCTION_WRAPPER -o "$src".o -I /usr/include/efi -I /usr/include/efi/x86_64
done

ld *.o /usr/lib/crt0-efi-x86_64.o -nostdlib -znocombreloc -T /usr/lib/elf_x86_64_efi.lds -shared -Bsymbolic -L /usr/lib -l:libgnuefi.a -l:libefi.a -o main.so

objcopy -j.text -j.sdata -j.data -j.dynamic -j.dynsym -j.rel -j.rela -j.reloc --target=efi-app-x86_64 main.so main.efi

mkdir -p $OUTPUT_DIR/EFI/BOOT && cp -f main.efi $OUTPUT_DIR/EFI/BOOT/BOOTX64.EFI

rm *.o
./create_img.sh $IMG_NAME $IMG_SIZE $OUTPUT_DIR