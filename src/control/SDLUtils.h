#ifndef LIFESIMULATIONC_SDLUTILS_H
#define LIFESIMULATIONC_SDLUTILS_H

#include <SDL2/SDL.h>

extern const SDL_Color C_BLACK_V;
extern const SDL_Color C_WHITE_V;
extern const SDL_Color C_RED_V;
extern const SDL_Color C_GREEN_V;
extern const SDL_Color C_BLUE_V;
extern const SDL_Color C_CYAN_V;
extern const SDL_Color C_MAGENTA_V;
extern const SDL_Color C_YELLOW_V;

extern const SDL_Color* C_BLACK;
extern const SDL_Color* C_WHITE;
extern const SDL_Color* C_RED;
extern const SDL_Color* C_GREEN;
extern const SDL_Color* C_BLUE;
extern const SDL_Color* C_CYAN;
extern const SDL_Color* C_MAGENTA;
extern const SDL_Color* C_YELLOW;

int SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color color);
int SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color* color);
int SetRenderDrawColor(SDL_Renderer* renderer, const SDL_Color* color);

#endif //LIFESIMULATIONC_SDLUTILS_H
