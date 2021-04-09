#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

typedef struct braille_ctx {
    SDL_Texture *tex;
    SDL_Window *win;
    SDL_Renderer *ren;
    SDL_Point pnt;
} braille_ctx;

static const char *braille_map[256] = {
    "⠀",
    "⠁", "⠂", "⠃", "⠄", "⠅", "⠆", "⠇", "⠈", "⠉", "⠊", "⠋", "⠌", "⠍", "⠎", "⠏", "⠐", "⠑",
    "⠒", "⠓", "⠔", "⠕", "⠖", "⠗", "⠘", "⠙", "⠚", "⠛", "⠜", "⠝", "⠞", "⠟", "⠠", "⠡", "⠢",
    "⠣", "⠤", "⠥", "⠦", "⠧", "⠨", "⠩", "⠪", "⠫", "⠬", "⠭", "⠮", "⠯", "⠰", "⠱", "⠲", "⠳",
    "⠴", "⠵", "⠶", "⠷", "⠸", "⠹", "⠺", "⠻", "⠼", "⠽", "⠾", "⠿", "⡀", "⡁", "⡂", "⡃", "⡄",
    "⡅", "⡆", "⡇", "⡈", "⡉", "⡊", "⡋", "⡌", "⡍", "⡎", "⡏", "⡐", "⡑", "⡒", "⡓", "⡔", "⡕",
    "⡖", "⡗", "⡘", "⡙", "⡚", "⡛", "⡜", "⡝", "⡞", "⡟", "⡠", "⡡", "⡢", "⡣", "⡤", "⡥", "⡦",
    "⡧", "⡨", "⡩", "⡪", "⡫", "⡬", "⡭", "⡮", "⡯", "⡰", "⡱", "⡲", "⡳", "⡴", "⡵", "⡶", "⡷",
    "⡸", "⡹", "⡺", "⡻", "⡼", "⡽", "⡾", "⡿", "⢀", "⢁", "⢂", "⢃", "⢄", "⢅", "⢆", "⢇", "⢈",
    "⢉", "⢊", "⢋", "⢌", "⢍", "⢎", "⢏", "⢐", "⢑", "⢒", "⢓", "⢔", "⢕", "⢖", "⢗", "⢘", "⢙",
    "⢚", "⢛", "⢜", "⢝", "⢞", "⢟", "⢠", "⢡", "⢢", "⢣", "⢤", "⢥", "⢦", "⢧", "⢨", "⢩", "⢪",
    "⢫", "⢬", "⢭", "⢮", "⢯", "⢰", "⢱", "⢲", "⢳", "⢴", "⢵", "⢶", "⢷", "⢸", "⢹", "⢺", "⢻",
    "⢼", "⢽", "⢾", "⢿", "⣀", "⣁", "⣂", "⣃", "⣄", "⣅", "⣆", "⣇", "⣈", "⣉", "⣊", "⣋", "⣌",
    "⣍", "⣎", "⣏", "⣐", "⣑", "⣒", "⣓", "⣔", "⣕", "⣖", "⣗", "⣘", "⣙", "⣚", "⣛", "⣜", "⣝",
    "⣞", "⣟", "⣠", "⣡", "⣢", "⣣", "⣤", "⣥", "⣦", "⣧", "⣨", "⣩", "⣪", "⣫", "⣬", "⣭", "⣮",
    "⣯", "⣰", "⣱", "⣲", "⣳", "⣴", "⣵", "⣶", "⣷", "⣸", "⣹", "⣺", "⣻", "⣼", "⣽", "⣾", "⣿"
};

#define sz(arr) (sizeof(arr) / sizeof(*(arr)))

#define BRAILLE_HEIGHT_PX 4
#define BRAILLE_WIDTH_PX  2
#define BSUR_WIDTH  64
#define BSUR_HEIGHT 128

#define WIN_WIDTH 512
#define WIN_HEIGHT 512

static char text[(BSUR_WIDTH * BSUR_HEIGHT) / (BRAILLE_HEIGHT_PX * BRAILLE_WIDTH_PX) * 3 + BSUR_HEIGHT + 1] = {0};
#define IS_LMB(x) ((x) & SDL_BUTTON_LMASK)

static Uint32 pixeldata[BSUR_HEIGHT][BSUR_WIDTH];

int main(int argc, char *argv[]) {
    braille_ctx ctx = {0};
    (void) argc, (void) argv;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    ctx.win = SDL_CreateWindow("BraillePaint", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN);
    ctx.ren = SDL_CreateRenderer(ctx.win, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED);
    ctx.tex = SDL_CreateTexture(ctx.ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, BSUR_WIDTH, BSUR_HEIGHT);
    goto clear_tex;
    while (1) {
        SDL_Event evt;
        SDL_WaitEvent(&evt);
        switch (evt.type) {
            case SDL_QUIT:
                goto quit;
            case SDL_KEYDOWN:
                switch (evt.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_DOWN:
                    case SDLK_RIGHT:
                    case SDLK_LEFT: {
                        #define KB_SPEED 5
                        const Uint8 *st = SDL_GetKeyboardState(NULL);
                        SDL_GetMouseState(&ctx.pnt.x, &ctx.pnt.y);
                        if (st[SDL_SCANCODE_UP]) ctx.pnt.y -= KB_SPEED;
                        if (st[SDL_SCANCODE_DOWN]) ctx.pnt.y += KB_SPEED;
                        if (st[SDL_SCANCODE_RIGHT]) ctx.pnt.x += KB_SPEED;
                        if (st[SDL_SCANCODE_LEFT]) ctx.pnt.x -= KB_SPEED;
                        SDL_WarpMouseInWindow(ctx.win, ctx.pnt.x, ctx.pnt.y);
                        break;
                    }
                    case SDLK_l: {
                        SDL_Surface *sur = SDL_LoadBMP("file.bmp");
                        if (!sur) break;
                        SDL_Texture *tex = SDL_CreateTextureFromSurface(ctx.ren, sur);

                        SDL_SetRenderTarget(ctx.ren, ctx.tex);
                        SDL_RenderCopy(ctx.ren, tex, NULL, NULL);
                        SDL_SetRenderTarget(ctx.ren, NULL);

                        SDL_FreeSurface(sur);
                        SDL_DestroyTexture(tex);
                        break;
                    }
                    case SDLK_c:
                    clear_tex:
                        SDL_SetRenderTarget(ctx.ren, ctx.tex);

                        SDL_SetRenderDrawColor(ctx.ren, 0x0, 0x0, 0x0, 0xFF);
                        SDL_RenderClear(ctx.ren);
                        SDL_RenderPresent(ctx.ren);

                        SDL_SetRenderTarget(ctx.ren, NULL);
                        break;
                    case SDLK_s: {
                        SDL_SetRenderTarget(ctx.ren, ctx.tex);

                        SDL_RenderReadPixels(ctx.ren, NULL, SDL_PIXELFORMAT_RGBA8888, pixeldata, BSUR_WIDTH * 4);
                        char *it = text;
                        for (int y = 0; y < BSUR_HEIGHT; y += BRAILLE_HEIGHT_PX) {
                            for (int x = 0; x < BSUR_WIDTH; x += BRAILLE_WIDTH_PX) {
                                Uint8 braille_byte = 0;
                                for (int i = x; i < x + BRAILLE_WIDTH_PX; ++i) {
                                    for (int j = y; j < y + BRAILLE_HEIGHT_PX; ++j) {
                                        int offset = (
                                            BRAILLE_HEIGHT_PX * (i % BRAILLE_WIDTH_PX)
                                            + (j % BRAILLE_HEIGHT_PX)
                                        );
                                        braille_byte |= (pixeldata[j][i] != 0x000000FF) << offset;
                                    }
                                }
                                memcpy(it, braille_map[sz(braille_map) - braille_byte - 1], 3);
                                it += 3;
                            }
                            *it++ = '\n';
                        }
                        *it = '\0';
                        SDL_SetClipboardText(text);

                        SDL_SetRenderTarget(ctx.ren, NULL);
                        break;
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (!IS_LMB(evt.motion.state)) break;
                ctx.pnt = (SDL_Point) {evt.button.x, evt.button.y};
                break;
            case SDL_MOUSEBUTTONUP:
                if (!IS_LMB(evt.motion.state)) break;
                ctx.pnt = (SDL_Point) {0, 0};
                break;
            case SDL_MOUSEMOTION:
                if (!IS_LMB(evt.motion.state)) break;
                SDL_SetRenderTarget(ctx.ren, ctx.tex);

                SDL_GetMouseState(&ctx.pnt.x, &ctx.pnt.y);
                filledCircleRGBA(ctx.ren,
                    (Sint16) (ctx.pnt.x / ((double) WIN_WIDTH / BSUR_WIDTH)),
                    (Sint16) (ctx.pnt.y / ((double) WIN_HEIGHT / BSUR_HEIGHT)),
                    1,
                    0x0, 0xFF, 0x0, 0xFF);
                SDL_RenderPresent(ctx.ren);

                SDL_SetRenderTarget(ctx.ren, NULL);
                break;
        }
        SDL_RenderCopy(ctx.ren, ctx.tex, NULL, NULL);
        SDL_RenderPresent(ctx.ren);
    }
    quit:
    return 0;
}
