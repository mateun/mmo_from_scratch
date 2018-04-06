#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>

#define MSG_TYPE_GAMESTATE_UPDATE 9

bool game_running = true;

bool initSDL() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("error while initializing SDL: %s\n", SDL_GetError());
		return false;
	}

	// TTF stuff
	if (TTF_Init() < 0) {
		printf("error while initializing SDL_TTF: %s\n", TTF_GetError());
		return false;
	}

	// NET stuff
	if (SDLNet_Init() < 0) {
		SDL_Log("error while initializing SDLNet %s", SDLNet_GetError());
		return false;
	}	

}

int main(int argc, char** args) {
	Uint64 ticks_per_second = SDL_GetPerformanceFrequency();
	initSDL();

	SDL_Window* window;
	SDL_Renderer* renderer;

	TTF_Font* font_default = TTF_OpenFont("D:/Projects/C++/mmo_game/build/data-latin.ttf", 50);
	if (!font_default) {
		printf("error while loading font: %s\n", TTF_GetError());
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
	SDL_Delay(2500);
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
	
	// Client sprite rendering stuff
	SDL_Surface* surf = SDL_LoadBMP("d:/Projects/C++/mmo_game/build/local_player.bmp");
	if (!surf) {
		SDL_Log("no surface! error! %s", SDL_GetError());
	}
	SDL_SetColorKey(surf, SDL_TRUE, 0);
	SDL_Texture* local_player_texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);

	// Local player position
	float local_pos_x;
	float local_pos_y;

	Uint32 last_message_update = 0;	
	// Frame loop
	while (game_running) {

		Uint64 frame_start_tick = SDL_GetPerformanceCounter();
		//printf("frame start tick: %u\n", frame_start_tick);
			
		// Checking the inputs
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			break; } // Grabing local keyboard input
		// Manipulate our model based
		const Uint8* kb_state = SDL_GetKeyboardState(NULL); 
		Uint8 input_data[5];
		input_data[0] = 8; // message type = INPUT_MSG
		input_data[1] = 0; // forward key state
		input_data[2] = 0; // backward key state
		input_data[3] = 0; // right key state
		input_data[4] = 0; // left key state

		if (kb_state[SDL_SCANCODE_W]) {
			//local_pos_y -= 0.01f;	
			input_data[1] = 1;
		}
		if (kb_state[SDL_SCANCODE_S]) {
			//local_pos_y += 0.01f;	
			input_data[2] = 1;
		} 
		if (kb_state[SDL_SCANCODE_D]) {
			//local_pos_x += 0.01f;	
			input_data[3] = 1;
		}
		if (kb_state[SDL_SCANCODE_A]) {
			//local_pos_x -= 0.01f;	
			input_data[4] = 1;
		}

		// Send the input data to the server
		// Only ten times per second for now.
		// todo(rongo): compress the information
		// that we gathered in the last 100ms
		// into the packets sent to the server
		Uint32 current_ticks = SDL_GetTicks();
		if (current_ticks - last_message_update > 50) {
			// todo(rongo): we should be able to use that function from the client as well,
			// because we are doing UDP, so that should not make a difference
			//send_data_to_client(packet_in->address, client_socket, game_data, 9);
			packet_out->data = input_data; 
			packet_out->len = 5;
			SDLNet_UDP_Send(client_socket, -1, packet_out);
			last_message_update = current_ticks;
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
		int recv_ok = SDLNet_UDP_Recv(client_socket, packet_in);
		if (recv_ok) {
				// Reading out the information for the local player
				Uint8* game_state = packet_in->data;
				Uint8 message_type = game_state[0];
				if (message_type == MSG_TYPE_GAMESTATE_UPDATE) {
					local_pos_x = SDLNet_Read32(&game_state[1]); 
					local_pos_y = SDLNet_Read32(&game_state[5]);
				}
				SDL_Log("x_coord_local_player updated: %f/%f", local_pos_x, local_pos_y);

				// Read out the information for all "other" players
				int number_of_other_players = game_state[9];	
				SDL_Log("number_of_other_players: %d", number_of_other_players);
				for (int i=0;i<number_of_other_players;i++) {
					Uint32 other_x = SDLNet_Read32(&game_state[10 + (i*8)]);
					Uint32 other_y = SDLNet_Read32(&game_state[14 + (i*8)]);
					SDL_Log("other player number: %d other_x: %u other_y: %u", i, other_x, other_y);
				}
		} else {
			//SDL_Log("nothing received from the server!");
		}
		

		// Render our world (all our models; the complete state of the game)
		SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0x00);
		SDL_RenderClear(renderer);
		
		millis = (float)frame_time_micro_secs/(float)1000;

		sprintf(frame_time_text_buffer,  "FrameTime: %f (ms)", millis);
		SDL_Surface* text_surface = TTF_RenderText_Solid(font_default, frame_time_text_buffer, color);
		if (text_surface == NULL) {
			printf("error while rendering text to surface: %s\n", TTF_GetError());
			return 1;
		}
		SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
		SDL_FreeSurface(text_surface);
		SDL_Rect rect_target_local_player;	
		rect_target_local_player.x = local_pos_x;
		rect_target_local_player.y = local_pos_y;
		rect_target_local_player.w = 64;
		rect_target_local_player.h = 64;

		SDL_RenderCopy(renderer, local_player_texture, NULL, &rect_target_local_player);
		// Render our frametime
		SDL_Rect rect_target;
		rect_target.x = 5;
		rect_target.y = 5;
		rect_target.w = 150;
		rect_target.h = 40;
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
