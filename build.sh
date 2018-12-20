#!/usr/bin/env bash
for src in *.c; do
	gcc ${src} -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPPER -o "$src".o -I /usr/include/efi -I /usr/include/efi/x86_64
done

ld *.o /usr/lib/crt0-efi-x86_64.o -nostdlib -znocombreloc -T /usr/lib/elf_x86_64_efi.lds -shared -Bsymbolic -L /usr/lib -l:libgnuefi.a -l:libefi.a -o main.so

objcopy -j.text -j.sdata -j.data -j.dynamic -j.dynsym -j.rel -j.rela -j.reloc --target=efi-app-x86_64 main.so main.efi
cp -f main.efi ./disk_dir/EFI/BOOT/BOOTX64.EFI