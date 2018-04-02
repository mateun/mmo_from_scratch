#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

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
	SDL_Log("server game is wating for packets...");
	while (true) {
		recv_ok = SDLNet_UDP_Recv(server_socket, packet_in);
		if (recv_ok) {
			SDL_Log("the game server received a packet");
			Uint8* data_in = packet_in->data;
			int len = packet_in->len;
			SDL_Log("data received: %u len: %d", data_in[1], len);
			// In a real implementation we would now do something with the data.
			// Interpret and process it; align it to the respective client;
			// Update the world/game model -> e.g. move player one two meters to the right
			// Then we would send an update to all the connected clients. 
			packet_out->data = data_out;
			packet_out->len = 10;
			packet_out-> maxlen = 10;
			packet_out->address = packet_in->address;
			SDLNet_UDP_Send(server_socket, -1, packet_out);


		}
	}
	



	SDLNet_UDP_Close(server_socket);
	SDLNet_Quit();

	SDL_Log("all good, server closing down regularly");
	return 0;
}

