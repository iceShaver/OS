IMG_DIR=$1
KERNEL_FILE=kernel.bin
if [ -z "$IMG_DIR" ]
then
	echo "Bad args"
fi
gcc -ffreestanding -c kernel/kernel.cc -o kernel.o -fpic
ld -pie -o $KERNEL_FILE --oformat binary kernel.o
mkdir -p $IMG_DIR && cp $KERNEL_FILE $IMG_DIR/ && rm $KERNEL_FILE
rm -f kernel.o $KERNEL_FILE
# Display binary 
# objdump -b binary -D kernel.bin -m i386:x86-64