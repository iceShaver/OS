#!/bin/bash
while mountpoint -q disk_dir; do
	sudo umount -f disk_dir
	if [ $? -ne 0 ]; then
		break
	fi
done

loops="$(losetup -l | grep disk.img | awk '{print $1}')"
for loop in $loops
do
	sudo losetup -d "$loop"
done
