#ifndef CONFIG_H
#define CONFIG_H

#include <string.h>

/* todo: everything should be configurable from within a separate microui-SDL window */

#define WIN_W 512
#define WIN_H 512

#define BSUR_W  (64)
#define BSUR_H  (64)

#define BRAILLE_W_PX 2
#define BRAILLE_H_PX 4

#if BSUR_W % BRAILLE_W_PX != 0
    #error "Surface width must be a multiple of 2"
#endif

#if BSUR_H % BRAILLE_H_PX != 0
    #error "Surface height must be a multiple of 2"
#endif


#define BRUSH_COLOR 0xFFu, 0xFFu, 0xFFu, 0xFFu

static inline uint32_t toRGBA32(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    uint8_t arr[4] = {r, g, b, a};
    uint32_t res;
    memcpy(&res, arr, sizeof(arr));
    return res;
}

#endif /* CONFIG_H */
