#!/bin/bash
# sudo losetup --offset 1048576 --sizelimit 103792128 /dev/loop3 disk.img
loop_dev_path="$(sudo losetup --partscan --find --show disk.img)"
if [ $? -eq 0 ]; then
	sudo mount -o uid=$UID,gid=$UID "$loop_dev_path"p1 disk_dir
	echo $loop_dev_path
fi
# sudo mount -o loop disk.img disk_dir