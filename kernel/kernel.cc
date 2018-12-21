#include <cstdint>
#include <ctype.h>
//using Pixel = uint32_t;
struct Pixel {
    uint8_t R : 8, G : 8, B : 8, empty : 8 = 0;
};

constexpr auto const VIDEO_MEM_SIZE = 640 * 480; // <- changeable in uefi settings
constexpr Pixel const WHITE = {255, 255, 255};

//Pixel *const VIDEO_MEM_ADDR =
//    reinterpret_cast<Pixel *const>(0xA0'00'0); // <- doesn't work

Pixel *const VIDEO_MEM_ADDR_VBE =
    reinterpret_cast<Pixel *const>(0x80'00'00'00); // <- from uefi settings

int main() {
    
    for (int i = 0; i < 640 * 5; i++) {
        VIDEO_MEM_ADDR_VBE[i] = WHITE;
    }
    // __asm__("movq $0xA0200, %rax");
    // __asm__("movq $0xFFFFF, (%rax)");
    __asm__("hlt");

    return 0;
}
