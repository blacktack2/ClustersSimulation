#include "SDLUtils.h"

const SDL_Color C_BLACK_V   = {0x00, 0x00, 0x00, 0xFF};
const SDL_Color C_WHITE_V   = {0xFF, 0xFF, 0xFF, 0xFF};
const SDL_Color C_RED_V     = {0xFF, 0x00, 0x00, 0xFF};
const SDL_Color C_GREEN_V   = {0x00, 0xFF, 0x00, 0xFF};
const SDL_Color C_BLUE_V    = {0x00, 0x00, 0xFF, 0xFF};
const SDL_Color C_CYAN_V    = {0x00, 0xFF, 0xFF, 0xFF};
const SDL_Color C_MAGENTA_V = {0xFF, 0x00, 0xFF, 0xFF};
const SDL_Color C_YELLOW_V  = {0xFF, 0xFF, 0x00, 0xFF};

const SDL_Color* C_BLACK   = &C_BLACK_V;
const SDL_Color* C_WHITE   = &C_WHITE_V;
const SDL_Color* C_RED     = &C_RED_V;
const SDL_Color* C_GREEN   = &C_GREEN_V;
const SDL_Color* C_BLUE    = &C_BLUE_V;
const SDL_Color* C_CYAN    = &C_CYAN_V;
const SDL_Color* C_MAGENTA = &C_MAGENTA_V;
const SDL_Color* C_YELLOW  = &C_YELLOW_V;

int SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color color) {
    const SDL_Color* c = &color;
    return SetRenderDrawColor(renderer, c);
}
int SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color* color) {
    const SDL_Color* c = color;
    return SetRenderDrawColor(renderer, c);
}
int SetRenderDrawColor(SDL_Renderer* renderer, const SDL_Color* color) {
    return SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
}
