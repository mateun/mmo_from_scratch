#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>

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

	// TTF stuff
	if (TTF_Init() < 0) {
		printf("error while initializing SDL_TTF: %s\n", TTF_GetError());
		return 1;
	}

	TTF_Font* font_default = TTF_OpenFont("D:/Projects/C++/mmo_game/build/data-latin.ttf", 50);
	if (!font_default) {
		printf("error while loading font: %s\n", TTF_GetError());
		return 1;
	}

	// NET stuff
	if (SDLNet_Init() < 0) {
		SDL_Log("error while initializing SDLNet %s", SDLNet_GetError());
		return 1;
	}

	UDPsocket client_socket = SDLNet_UDP_Open(0);
	UDPpacket* packet_out = SDLNet_AllocPacket(100);
	if (!packet_out) {
		SDL_Log("error while creating our outgoing package: %s", SDLNet_GetError());
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


	// "Connect" (send a packet) to the server to ask for initial data
	// Send a "magic" byte package to the server
	// Wait for the answer and store the initial information
	// we got and use it for rendering our world/game.
	IPaddress server_address;
	SDLNet_ResolveHost(&server_address, "127.0.0.1", 6666);
	packet_out->address = server_address;
	Uint8 data_out[10];
	data_out[0] = 0x07;
	packet_out->data = data_out; 
	packet_out->len = 4;
	SDLNet_UDP_Send(client_socket, -1, packet_out);	
	SDL_Log("status of our last send: %d", packet_out->status);	

	// Now, next we wait for an answer
	UDPpacket* packet_in = SDLNet_AllocPacket(50);
	SDL_Delay(7000);
	int recv_ok = SDLNet_UDP_Recv(client_socket, packet_in);
	if (recv_ok) {
		Uint8* initial_data = packet_in->data;
		// Now we examine the incoming data
  	Uint8 x_coord_local_player = initial_data[0];
		int y_coord_local_player = initial_data[1];	
		int number_of_online_players = initial_data[2];
		int x_coord_remote_player_1 = initial_data[3];	
		// ... 
		SDL_Log("x_coord_local_player: %u", x_coord_local_player);
	} else {
		SDL_Log("nothing received from the server!");
	}
	

	// Frame loop
	while (game_running) {

		Uint64 frame_start_tick = SDL_GetPerformanceCounter();
		//printf("frame start tick: %u\n", frame_start_tick);
			
		// Checking the inputs
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			break;
		}

		// Checking input from the server?
		// Listening to its UDPsocket
		// Reading in the information, interpreting it, 
		// applying it to its own model on this specific client. 
		// Recv();
		// packet->data();
		// if (data[0] == POSITION_UPDATE) 
		// apply the new position right away OR check if the new position
		// is close enough to make an immediate update. 
		// If its too far away that might look choppy, and we might need to interpolate.  

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
		//printf("frame tick time: %u (microseconds)  \n", frame_time_micro_secs);

	}

	printf("all good!\n");
	return 0;
}
