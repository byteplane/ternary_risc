#include <iostream>
#include <SDL2/SDL.h>

#include "char_rom.h"

#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (768)

#define TERNARY_SCREEN_PIXELS (729)
#define TERNARY_SCREEN_START_COL (276)
#define TERNARY_SCREEN_END_COL (TERNARY_SCREEN_START_COL + TERNARY_SCREEN_PIXELS)
#define TERNARY_SCREEN_START_ROW (19)
#define TERNARY_SCREEN_END_ROW (TERNARY_SCREEN_START_ROW + TERNARY_SCREEN_PIXELS)
#define TERNARY_TILE_WIDTH (18)
#define TERNARY_TILE_HEIGHT (27)
#define TERNARY_SCREEN_SCALE (3)

void draw_char(Uint32 *buf, int char_row, int char_col, int char_idx, Uint32 background, Uint32 color1, Uint32 color2) {
	for (int row = 0; row < TERNARY_TILE_HEIGHT; row++) {
		Uint8 char_line = char_rom[char_idx] * 9 + row / TERNARY_SCREEN_SCALE;

		for (int col = 0; col < TERNARY_TILE_WIDTH; col++) {
			int screen_row = (TERNARY_SCREEN_START_ROW + char_row * TERNARY_TILE_HEIGHT + row);
			int screen_col = (TERNARY_SCREEN_START_COL + char_col * TERNARY_TILE_WIDTH + col);
			int screen_loc = screen_row * SCREEN_WIDTH + screen_col;
			bool char_bit = char_line >> (col / TERNARY_SCREEN_SCALE);
			
			buf[screen_loc] = background;
		}
	}
}

void init() {
}

int main(int argc, char **argv) {
	init();

	bool quit = false;
	bool leftMouseButtonDown = false;
	SDL_Event event;
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Error in SDL_Init:\n");
		printf("SDL_Error: %s\n", SDL_GetError());
	}

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
	if (!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")) {
		printf("Warning: SDL cannot disable compositor bypass\n");
	}
#endif

	SDL_Window *window = SDL_CreateWindow("Test SDL Window",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		printf("Error in SDL_CreateWindow:\n");
		printf("SDL_Error: %s\n", SDL_GetError());
		return 0;
	}

	SDL_Surface *screen = SDL_GetWindowSurface(window);
	SDL_Surface *pixels = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_RGBX8888);

	SDL_FillRect(pixels, NULL, 0xFFFFFFFF);

	bool leftMouseButtonPressed = false;

	int mouseX, mouseY;
	SDL_Keysym pressedKey;
	bool keyPressed = false;

	for (;;) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT) return 0;
			if (ev.type == SDL_KEYDOWN) { printf( "Scancode: 0x%02X", ev.key.keysym.scancode ); }
			if (ev.type == SDL_KEYUP) { printf("Released.\n"); }
			if (ev.type == SDL_MOUSEBUTTONDOWN) leftMouseButtonPressed = true;
			if (ev.type == SDL_MOUSEBUTTONUP) leftMouseButtonPressed = false;
			if (ev.type == SDL_MOUSEMOTION) { 
				if (leftMouseButtonPressed) {
					SDL_GetMouseState(&mouseX, &mouseY);
				}
			}
		}

		if (SDL_MUSTLOCK(pixels)) SDL_LockSurface(pixels);
		{
			Uint32 *buf = (Uint32*)pixels->pixels;
			static int i = 0;
			for (int row = 0; row < TERNARY_SCREEN_PIXELS / TERNARY_TILE_HEIGHT; row++) {
				for (int col = 0; col < TERNARY_SCREEN_PIXELS / TERNARY_TILE_WIDTH; col++) {
					if ((row + col) % 2 == 0) {
						// Draw a black box
						draw_char(buf, row, col, 1, i, 0, 0);
					}
				}
			}
		}				
		if (SDL_MUSTLOCK(pixels)) SDL_UnlockSurface(pixels);

		SDL_BlitSurface(pixels, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);
	}

	
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
