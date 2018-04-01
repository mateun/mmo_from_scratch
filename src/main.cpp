#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

bool game_running = true;

int main(int argc, char** args) {

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("error while initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	Uint64 ticks_per_second = SDL_GetPerformanceFrequency();
	printf("ticks per second: %llu\n", ticks_per_second);

	SDL_Window* window;
	SDL_Renderer* renderer;

	if (TTF_Init() < 0) {
		printf("error while initializing SDL_TTF: %s\n", TTF_GetError());
		return 1;
	}

	TTF_Font* font_default = TTF_OpenFont("D:/Projects/C++/mmo_game/build/data-latin.ttf", 50);
	if (!font_default) {
		printf("error while loading font: %s\n", TTF_GetError());
		return 1;
	}

	if (SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_RESIZABLE, 
													&window, &renderer)) {
		printf("error while creating our window, exiting%s\n", SDL_GetError());				
		return 1;
	}

	SDL_SetWindowTitle(window, "MMO_GAME");

	SDL_Color color= {240, 240, 240};
	SDL_Surface* text_surface = TTF_RenderText_Solid(font_default, "hello mmo game!", color);
	if (text_surface == NULL) {
		printf("error while rendering text to surface: %s\n", TTF_GetError());
		return 1;
	}
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	SDL_FreeSurface(text_surface);

	SDL_Event event;

	char frame_time_text_buffer[50];
	unsigned int frame_time_micro_secs = 0;
	float millis = 0;

	// Frame loop
	while (game_running) {

		Uint64 frame_start_tick = SDL_GetPerformanceCounter();
		//printf("frame start tick: %u\n", frame_start_tick);
			
		// Checking the inputs
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			break;
		}

		// Manipulate our model
		

		// Render our world (all our models; the complete state of the game)
		SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0x00);
		SDL_RenderClear(renderer);
		SDL_Rect rect_target;
		rect_target.x = 5;
		rect_target.y = 5;
		rect_target.w = 150;
		rect_target.h = 40;

		millis = (float)frame_time_micro_secs/(float)1000;

		sprintf(frame_time_text_buffer,  "FrameTime: %f (ms)", millis);
		SDL_Surface* text_surface = TTF_RenderText_Solid(font_default, frame_time_text_buffer, color);
		if (text_surface == NULL) {
			printf("error while rendering text to surface: %s\n", TTF_GetError());
			return 1;
		}
		SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
		SDL_FreeSurface(text_surface);
		
		SDL_RenderCopy(renderer, text_texture, NULL, &rect_target);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(text_texture);
	
		Uint64 frame_end_tick = SDL_GetPerformanceCounter();	
		//printf("frame end tick: %u\n", frame_end_tick);
		Uint64 frame_ticks = frame_end_tick - frame_start_tick;
		frame_time_micro_secs = frame_ticks * 0.293;
		printf("frame tick time: %u (microseconds)  \n", frame_time_micro_secs);

	}

	printf("all good!\n");
	return 0;
}
