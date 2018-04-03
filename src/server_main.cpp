#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>



void send_initial_data_to_client(IPaddress client_address, UDPsocket server_socket) {
	SDL_Log("in send_initial_data_to_client");
	UDPpacket* out_packet = SDLNet_AllocPacket(50);
	Uint8 data_out[50];
	// 0 = x coordinate of the player itself
	// 1 = y coordinate of the player itself
	// 2 = number of players
	// 3 = x coordinate of "other player" 1
	// 4 ... 
	// 5 ... 
	//
	data_out[0] = 50;
	data_out[1] = 100;
	data_out[2] = 2;
	data_out[3] = 55;
	data_out[4] = 100;
	data_out[5] = 135;
	data_out[6] = 155;
	out_packet->data = data_out;
	out_packet->len = 7;
	out_packet->address = client_address;
	SDL_Delay(5000);
	SDL_Log("we just waited for 5 seconds, now sending the package to the client really");
	SDLNet_UDP_Send(server_socket, -1, out_packet);
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
	SDL_Log("server game is wating for packets...");
	while (true) {
		recv_ok = SDLNet_UDP_Recv(server_socket, packet_in);
		if (recv_ok) {
			SDL_Log("the game server received a packet");
			Uint8* data_in = packet_in->data;
			int len = packet_in->len;
			SDL_Log("data received: %u len: %d", data_in[0], len);
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
				case 7: SDL_Log("login message");
								send_initial_data_to_client(packet_in->address, server_socket);
								break;
			}


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

