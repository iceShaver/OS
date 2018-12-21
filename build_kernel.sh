# gcc -ffreestanding -c kernel.cc -o kernel.o -fPIC
# ld -o kernel.bin -Ttext 0x0 --oformat binary kernel.o

# Display binary 
# objdump -b binary -D kernel.bin -m i386:x86-64