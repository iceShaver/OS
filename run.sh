#!/bin/bash
BOOT_FILE=$1
if [ -z "$BOOT_FILE" ]
then
	echo "Empty arg"
	exit 1
fi
EFI_FILE=bios.bin
#VM_NAME="OS"
#sync -f disk.img
#sleep 1
#VBoxManage controlvm $VM_NAME poweroff
#if ps -x | grep -E "virtualbox.*--comment OS" | grep -v "grep" > /dev/null; then
#	echo "Waiting for VM to poweroff"
#fi
#while  ps -x | grep -E "virtualbox.*--comment OS" | grep -v "grep" > /dev/null
#do
#  sleep 0.1
#done;
#rm boot.vdi
# VBoxManage storageattach $VM_NAME --storagectl "SATA" --port 0 --device 0 --type fdd --medium "fat.img"
#VBoxManage convertfromraw --format VDI disk.img boot.vdi --uuid d7df84ed-f561-4303-b677-d321c9c3564e
#VBoxManage startvm "OS"
#qemu-system-x86_64 -pflash bios.bin -hda fat:rw:hda-contents -net none
qemu-system-x86_64 -pflash $EFI_FILE -drive format=raw,file=$BOOT_FILE -net none
