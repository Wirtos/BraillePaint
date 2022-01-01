#include "bilevel_cvt.h"
#include <stdlib.h>


/*
 * calculates contrast value, assumes rgba32
 * https://24ways.org/2010/calculating-color-contrast
 */
#define L(rgb) ((int32_t) (rgb)[0] * 299 + (int32_t) (rgb)[1] * 587 + (int32_t) (rgb)[2] * 114)

/* bounds long value to uint8 */
#define CLIP8(v)          \
    (                     \
        ((v) <= 0)        \
            ? 0           \
            : ((v) < 256) \
                ? (v)     \
                : 255     \
   )

int bilevel_cvt(uint32_t *imOut, const uint32_t *imIn, int xsize, int ysize) {
    int *errors;

    errors = calloc(xsize + 1, sizeof(*errors));
    if (!errors) return 0;
    /* map each pixel to black or white, using error diffusion */
    for (int y = 0; y < ysize; y++) {
        int l, l0, l1, l2, d2;
        const uint32_t *in_row = &imIn[y * xsize];
        uint32_t *out_row = &imOut[y * xsize];

        l = l0 = l1 = 0;

        for (int x = 0; x < xsize; x++) {
            int palette_color;
            uint8_t *out_rgba = (uint8_t *) &out_row[x];
            /* pick closest colour */
            /* errors should be filled with previous iteration's error data */
            l = CLIP8((L(((uint8_t *) &in_row[x])) / 1000) + (l + errors[x + 1]) / 16);
            palette_color = (l > 128) ? 255 : 0;

            out_rgba[0] = out_rgba[1] = out_rgba[2] = (uint8_t) palette_color;
            /* propagate errors */
            l -= palette_color;
            l2 = l;
            d2 = l + l;
            l += d2;
            errors[x] = l + l0;
            l += d2;
            l0 = l + l1;
            l1 = l2;
            l += d2;
        }

        errors[xsize] = l0;
    }

    free(errors);
    return 1;
}
