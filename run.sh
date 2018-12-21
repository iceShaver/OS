IMG_DIR=image_dir/
IMG_FILE=disk.img
IMG_SIZE=100M
./build_kernel.sh $IMG_DIR && ./build_bootloader.sh $IMG_FILE $IMG_SIZE $IMG_DIR && ./create_img.sh $IMG_FILE $IMG_SIZE $IMG_DIR && ./qemu.sh disk.img