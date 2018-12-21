#!/bin/bash
# sudo losetup --offset 1048576 --sizelimit 103792128 /dev/loop3 disk.img
IMG_FILE=$1
IMG_SIZE=$2
IMG_DIR=$3
if [ -z "$IMG_FILE" ] || [ -z "$IMG_SIZE" ] || [ -z "$IMG_DIR" ]
then
      echo "Bad arguments"
      exit 1
fi
TMP_DISK_DIR="tmp_disk_dir"
echo "Creating image file with name: $IMG_FILE Size: $IMG_SIZE from dir: $IMG_DIR"
# create GPT disk image and create partition
rm -f $IMG_FILE;
truncate -s $IMG_SIZE $IMG_FILE;
sgdisk $IMG_FILE -o -n 0:0:0 -t 0:ef00;
# set loop device to created img file
loop_dev_path="$(sudo losetup --partscan --find --show $IMG_FILE)";
# format partition
sudo mkfs.fat -F32 "$loop_dev_path"p1;
# mount partition
mkdir -p $TMP_DISK_DIR;
sudo mount -o uid=$UID,gid=$UID "$loop_dev_path"p1 $TMP_DISK_DIR;
echo $loop_dev_path;
# copy files from $FROM_DIR to partition
cp -r $IMG_DIR/* $TMP_DISK_DIR
sync
# unmount partition and delete tmp dir
while mountpoint -q $TMP_DISK_DIR; do
	sudo umount -f $TMP_DISK_DIR
	if [ $? -ne 0 ]; then
		break
	fi
done
loops="$(losetup -l | grep $IMG_FILE | awk '{print $1}')"
for loop in $loops
do
	sudo losetup -d "$loop"
done
rm -rf $TMP_DISK_DIR