#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <map>
#include <string>
#include "player_structures.h"

// Gamestate
std::map<std::string, Player*> players_map;

Uint8 latest_global_player_id = 0;

Player* login_player(UDPpacket* packet) {
	Uint8* data = packet->data;
	Player* player = new Player();
	player->address = packet->address;	
	player->player_id = latest_global_player_id++;
	// In a "real" mmo/persistent game, 
	// we would very probably look up the position 
	// from a database of the given player.
	player->pos_x = 80; 
	player->pos_y = 100;
	player->hp = 100;
	SDL_Log("player address: %d __pos_x: %f", player, player->pos_x);
	return player;
	
}

// This procedure should is part of the "network" layer. 
void send_data_to_client(IPaddress client_address, UDPsocket server_socket, Uint8* data) {
	SDL_Log("in send_initial_data_to_client");
	UDPpacket* out_packet = SDLNet_AllocPacket(50);
	//Uint8 data_out[50];
	// 0 = x coordinate of the player itself
	// 1 = y coordinate of the player itself
	// 2 = number of players
	// 3 = x coordinate of "other player" 1
	// 4 ... 
	// 5 ... 
	//
	//data_out[0] = 50;
	//data_out[1] = 100;
	//data_out[2] = 2;
	//data_out[3] = 55;
	//data_out[4] = 100;
	//data_out[5] = 135;
	//data_out[6] = 155;
	out_packet->data = data;
	out_packet->len = 7;
	out_packet->address = client_address;
	SDL_Log("we just waited for 5 seconds, now sending the package to the client really");
	SDLNet_UDP_Send(server_socket, -1, out_packet);
}

std::string get_player_key_by_ip_address(IPaddress address) {
	return std::to_string(address.host) + "_" 
												+ std::to_string(address.port);
}

int main(int argc, char** args) {

	SDL_Log("server game starting up");

	if (SDL_Init(0) < 0) {
		SDL_Log("error initializing SDL %s", SDL_GetError());
		return 1;
	}

	if (SDLNet_Init() < 0) {
		SDL_Log("error initializing SDLNet %s", SDLNet_GetError());
		return 1;
	}

	UDPsocket server_socket = SDLNet_UDP_Open(6666);
	if (!server_socket)  {
		SDL_Log("error setting up our socket %s", SDLNet_GetError());
		return 1;
	}

	// Lets receive a packet
	UDPpacket* packet_in = SDLNet_AllocPacket(1024);
	if (!packet_in) {
		SDL_Log("error creating packet%s", SDLNet_GetError());
		return 1;
	}
	// Lets prepare a packet for our outgoing information
	UDPpacket* packet_out = SDLNet_AllocPacket(100);
	Uint8 data_out[10];
	for (int i=0; i<10;++i) {
		data_out[i] = 0;
	}	
	data_out[0] = 0x01;
	data_out[1] = 0xff;

	int recv_ok = 0;
	SDL_Log("number of players connected: %d", players_map.size());
	SDL_Log("server game is wating for packets...");
	while (true) {
		recv_ok = SDLNet_UDP_Recv(server_socket, packet_in);
		if (recv_ok) {
			//SDL_Log("the game server received a packet");
			Uint8* data_in = packet_in->data;
			int len = packet_in->len;
			//SDL_Log("data received: %u len: %d", data_in[0], len);
			// In a real implementation we would now do something with the data.
			// Interpret and process it; align it to the respective client;
			// Update the world/game model -> e.g. move player one two meters to the right
			// Then we would send an update to all the connected clients. 
			//
			int message_type = data_in[0];
			switch (message_type) {
				case 0: SDL_Log("logout message"); 
								// remove the player from the game (current session)
								// all kind of bookkeeping and storing the state to the database.
								break;
				case 7: {
								SDL_Log("login message");
								Player* player = login_player(packet_in);
								SDL_Log("player address: %d pos_x: %f", player, player->pos_x);

								std::string unique_player_network_key = get_player_key_by_ip_address(player->address);
								players_map[unique_player_network_key] = player;
								player = players_map[unique_player_network_key];
								SDL_Log("player address: %d pos_x: %f", player, player->pos_x);

								SDL_Log("size of players map: %d", players_map.size());
								Uint8 initial_game_data[50];
								SDL_Log("player pos_x: %f", player->pos_x);
								initial_game_data[0] = player->pos_x;
								initial_game_data[1] = player->pos_y;
								initial_game_data[2] = players_map.size();
								send_data_to_client(packet_in->address, server_socket, initial_game_data);
								break;
								}
				case 8: {
												SDL_Log("input message"); 
								std::string unique_player_network_key = get_player_key_by_ip_address(packet_in->address);
								Player* player = players_map[unique_player_network_key];
								//SDL_Log("nr. of players: %d. player->pos_x: %d", players_map.size(), player->pos_x);
								// Lets now examine/parse the input data from the client
								// and then update the model accordingly.
								Uint8 fwd = packet_in->data[1];
								Uint8 bwd = packet_in->data[2];
								Uint8 right = packet_in->data[3];
								Uint8 left = packet_in->data[4];
								if (fwd) player->pos_y -= 0.01f;
								if (bwd) player->pos_y += 0.01f;
								if (right) player->pos_x += 0.01f;
								if (left) player->pos_x -= 0.01f;
								SDL_Log("player pos_x: %d pos_y: %d", player->pos_x, player->pos_y);
								Uint8 game_data[3];
								game_data[0] = 9;
								game_data[1] = player->pos_x;
								game_data[2] = player->pos_y;
								send_data_to_client(packet_in->address, server_socket, game_data);
								break;
								}
			}

		}
	}
	



	SDLNet_UDP_Close(server_socket);
	SDLNet_Quit();

	SDL_Log("all good, server closing down regularly");
	return 0;
}

