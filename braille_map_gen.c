#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static uint8_t b2br(uint8_t i) {
    /* shuffle bit to align them as 7 3 6 5 4 2 1 0 */
    return (i & 0x87u)
        | ((i & 0x08u) << 3)
        | ((i & 0x70u) >> 1);
}

#define fw(s, fp) fwrite(s, 1, sizeof(s) - 1, fp)

int main(int argc, char *argv[]) {
    FILE *fp;
    fp = (argc != 1) ? fopen(argv[1], "wb") : stdout;
    if (!fp) return EXIT_FAILURE;
    fw("static const char braille_map[256][3] = {\n    ", fp);
    for (uint8_t i = 0;; i++) {
        fw("\"", fp);
        {
            uint8_t offset;
            uint8_t glyph[4]; /* one extra byte for fw's sizeof - 1 */
            offset = b2br(i);
            uint32_t cp = 0x2800 + offset;
            /* codepoint to 3 utf-8 bytes */
            glyph[0] = 0xE0u | ((cp >> 12) & 0x0Fu);
            glyph[1] = 0x80u | ((cp >> 6 ) & 0x3Fu);
            glyph[2] = 0x80u | ((cp >> 0 ) & 0x3Fu);
            fw(glyph, fp);
        }
        fw("\"", fp);

        if (i == 0xFF) break;

        fw(", ", fp);

        if (i % 17 == 0) fw("\n    ", fp);
    }
    fw("\n};", fp);

    if (argc != 1) fclose(fp);
    return EXIT_SUCCESS;
}
