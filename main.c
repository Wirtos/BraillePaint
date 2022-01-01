#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <sfd.h>

typedef struct braille_ctx {
    SDL_Texture *tex;
    SDL_Window *win;
    SDL_Renderer *ren;
    SDL_Point pnt;
    short brush_rad;
} braille_ctx;

#include "config.h"
#include <braille_map.h>
#include <bilevel_cvt.h>

#define sz(arr) (sizeof(arr) / sizeof(*(arr)))

static char generated_text[(BSUR_W * BSUR_H) / (BRAILLE_H_PX * BRAILLE_W_PX) * sizeof(*braille_map) + BSUR_H + 1] = {0};
#define IS_LMB(x) ((x) & SDL_BUTTON_LMASK)

static uint32_t pixeldata[BSUR_H][BSUR_W];

static void critical_error(void) {
    puts(SDL_GetError());
    abort();
}

void clear_texture(const braille_ctx *ctx) {
    SDL_SetRenderTarget(ctx->ren, ctx->tex);
    SDL_SetRenderDrawColor(ctx->ren, 0x0u, 0x0u, 0x0u, 0xFFu);
    SDL_RenderClear(ctx->ren);
}

int load_into_texture(const char *path, SDL_Renderer *ren, SDL_Texture *tex) {
    SDL_Texture *itex = IMG_LoadTexture(ren, path);
    if (itex == NULL) {
        puts(IMG_GetError());
        return 0;
    }
    SDL_SetRenderTarget(ren, tex);
    SDL_RenderCopy(ren, itex, NULL, NULL);
    SDL_DestroyTexture(itex);
    return 1;
}

#define pow2(x) ((x) * (x))

uint32_t find_closest_b_and_w(Uint8 r, Uint8 g, Uint8 b) {
    if ((sqrt(pow2(r) * .241 + pow2(g) * .691 + pow2(b) * .068) > 128)) {
        return toRGBA32(0xFFu, 0xFFu, 0xFFu, 0xFFu);
    } else {
        return toRGBA32(0x0u, 0x0u, 0x0u, 0xFFu);
    }
}

int main(int argc, char *argv[]) {
    braille_ctx ctx = {.brush_rad = 1};
    (void) argc, (void) argv;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        critical_error();
    }
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != 0) {
        critical_error();
    }
    ctx.win = SDL_CreateWindow("BraillePaint",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIN_W, WIN_H,
        SDL_WINDOW_SHOWN);
    if (ctx.win == NULL) {
        critical_error();
    }

    ctx.ren = SDL_CreateRenderer(ctx.win, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED);
    if (ctx.ren == NULL) {
        critical_error();
    }

    ctx.tex = SDL_CreateTexture(ctx.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, BSUR_W, BSUR_H);
    if (ctx.tex == NULL) {
        critical_error();
    }
    clear_texture(&ctx);

    while (1) {
        SDL_Event evt;
        /* todo: not very efficient gui-wise, create different control units */
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
                case SDL_QUIT:
                    goto quit;
                case SDL_DROPFILE: {
                    char *dropped_file = evt.drop.file;
                    puts(dropped_file);
                    load_into_texture(dropped_file, ctx.ren, ctx.tex);
                    SDL_free(dropped_file);
                    break;
                }
                case SDL_KEYDOWN:
                    switch (evt.key.keysym.sym) {
                        case SDLK_EQUALS:
                            ctx.brush_rad++;
                            break;

                        case SDLK_MINUS:
                            if (ctx.brush_rad > 0) ctx.brush_rad--;
                            break;
                        case SDLK_c:
                            clear_texture(&ctx);
                            break;
                        case SDLK_SPACE:
                            ctx.pnt = (SDL_Point) {WIN_W / 2, WIN_H / 2};
                            SDL_WarpMouseInWindow(ctx.win, ctx.pnt.x, ctx.pnt.y);
                            break;

                        case SDLK_i: {
                            SDL_SetRenderTarget(ctx.ren, ctx.tex);

                            SDL_RenderReadPixels(ctx.ren, NULL, SDL_PIXELFORMAT_RGBA32,
                                pixeldata, BSUR_W * sizeof(**pixeldata));

                            for (int y = 0; y < BSUR_H; y++) {
                                for (int x = 0; x < BSUR_W; x++) {

                                    uint8_t *col = (uint8_t *) &pixeldata[y][x];

                                    SDL_SetRenderDrawColor(ctx.ren, ~col[0], ~col[1], ~col[2], col[3]);
                                    SDL_RenderDrawPoint(ctx.ren, x, y);
                                }
                            }
                            SDL_RenderPresent(ctx.ren);
                            break;
                        }
                        case SDLK_g: {
                            SDL_SetRenderTarget(ctx.ren, ctx.tex);
                            SDL_RenderReadPixels(ctx.ren, NULL, SDL_PIXELFORMAT_RGBA32,
                                pixeldata, BSUR_W * sizeof(**pixeldata));

                            tobilevel((uint32_t *) pixeldata, (const uint32_t *) pixeldata, BSUR_W, BSUR_H);

                            SDL_Texture *tex = SDL_CreateTextureFromSurface(ctx
                                .ren, SDL_CreateRGBSurfaceWithFormatFrom(pixeldata, BSUR_W, BSUR_H, 32,
                                BSUR_W * sizeof(**pixeldata), SDL_PIXELFORMAT_RGBA32)
                            );
                            SDL_SetRenderTarget(ctx.ren, ctx.tex);
                            SDL_RenderCopy(ctx.ren, tex, NULL, NULL);
                            SDL_DestroyTexture(tex);
                            SDL_RenderPresent(ctx.ren);
                            break;
                        }
                        case SDLK_b:
                            SDL_SetRenderTarget(ctx.ren, ctx.tex);

                            SDL_RenderReadPixels(ctx.ren, NULL, SDL_PIXELFORMAT_RGBA32,
                                pixeldata, BSUR_W * sizeof(**pixeldata));

                            for (int y = 0; y < BSUR_H; y++) {
                                for (int x = 0; x < BSUR_W; x++) {
                                    uint32_t res;
                                    uint8_t *col = (uint8_t *) &pixeldata[y][x];
                                    uint8_t *newcol = (uint8_t *) &res;
                                    res = find_closest_b_and_w(col[0], col[1], col[2]);

                                    SDL_SetRenderDrawColor(ctx.ren, newcol[0], newcol[1], newcol[2], newcol[3]);
                                    SDL_RenderDrawPoint(ctx.ren, x, y);
                                }
                            }
                            SDL_RenderPresent(ctx.ren);
                            break;

                        case SDLK_l: {
                            const char *path = sfd_open_dialog(&(sfd_Options) {
                                .title        = "Import Image File",
                                .filter_name  = "Image File",
                                .filter       = "*.bmp|*.png|*.jpg",
                            });
                            if (path == NULL) {
                                puts(sfd_get_error());
                                break;
                            }
                            load_into_texture(path, ctx.ren, ctx.tex);
                            break;
                        }

                        case SDLK_s: {
                            FILE *fp;
                            const char *path = sfd_save_dialog(&(sfd_Options) {
                                .title        = "Save As Text File",
                                .filter_name  = "Normal text file (*.txt)",
                                .filter       = "*.txt",
                                .extension    = "txt"
                            });
                            if (path == NULL) {
                                puts(sfd_get_error());
                                break;
                            }
                            fp = fopen(path, "w");
                            if (fp == NULL) {
                                perror("Can't open file for writing");
                                break;
                            }
                            SDL_SetRenderTarget(ctx.ren, ctx.tex);

                            SDL_RenderReadPixels(ctx.ren, NULL, SDL_PIXELFORMAT_RGBA32,
                                pixeldata, BSUR_W * sizeof(**pixeldata));
                            char *it = generated_text;
                            for (int y = 0; y < BSUR_H; y += BRAILLE_H_PX) {
                                for (int x = 0; x < BSUR_W; x += BRAILLE_W_PX) {
                                    #define p(pd) (uint8_t)((pd) == toRGBA32(0x0u, 0x0u, 0x0u, 0xFFu))
                                    Uint8 braille_byte = 0;
                                    braille_byte |= p(pixeldata[y + 0][x + 0]) << 0;
                                    braille_byte |= p(pixeldata[y + 1][x + 0]) << 1;
                                    braille_byte |= p(pixeldata[y + 2][x + 0]) << 2;
                                    braille_byte |= p(pixeldata[y + 3][x + 0]) << 3;
                                    braille_byte |= p(pixeldata[y + 0][x + 1]) << 4;
                                    braille_byte |= p(pixeldata[y + 1][x + 1]) << 5;
                                    braille_byte |= p(pixeldata[y + 2][x + 1]) << 6;
                                    braille_byte |= p(pixeldata[y + 3][x + 1]) << 7;

                                    memcpy(it, braille_map[sz(braille_map) - braille_byte - 1], sizeof(*braille_map));
                                    it += sizeof(*braille_map);
                                }
                                *it++ = '\n';
                            }
                            *it = '\0';
                            // SDL_SetClipboardText(generated_text);
                            fputs(generated_text, fp);
                            fclose(fp);
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
                        (Sint16) (ctx.pnt.x / ((double) WIN_W / BSUR_W)),
                        (Sint16) (ctx.pnt.y / ((double) WIN_H / BSUR_H)),
                        ctx.brush_rad,
                        BRUSH_COLOR);
                    SDL_RenderPresent(ctx.ren);
                    break;
                default:
                    continue;
            } /* switch evt.type */
        } /* while SDL_PollEvent */

        {
            #define KB_SPEED 1
            #define KB_FASTSPEED 8
            const Uint8 *st = SDL_GetKeyboardState(NULL);
            SDL_GetMouseState(&ctx.pnt.x, &ctx.pnt.y);
            int speed = KB_SPEED;
            if (!st[SDL_SCANCODE_LCTRL] && !st[SDL_SCANCODE_RCTRL]) speed = KB_FASTSPEED;
            if (st[SDL_SCANCODE_UP]) ctx.pnt.y -= speed;
            if (st[SDL_SCANCODE_DOWN]) ctx.pnt.y += speed;
            if (st[SDL_SCANCODE_RIGHT]) ctx.pnt.x += speed;
            if (st[SDL_SCANCODE_LEFT]) ctx.pnt.x -= speed;

            if (st[SDL_SCANCODE_UP] | st[SDL_SCANCODE_DOWN] | st[SDL_SCANCODE_RIGHT] | st[SDL_SCANCODE_LEFT]) {
                SDL_WarpMouseInWindow(ctx.win, ctx.pnt.x, ctx.pnt.y);
            }
        }

        SDL_SetRenderTarget(ctx.ren, NULL);
        SDL_RenderClear(ctx.ren);
        SDL_RenderCopy(ctx.ren, ctx.tex, NULL, NULL);
        SDL_RenderPresent(ctx.ren);

        /* todo: don't draw if nothing changed */
        SDL_Delay(0);
    }
    quit:
    return EXIT_SUCCESS;
}
