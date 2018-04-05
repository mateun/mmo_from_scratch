#include <SDL2/SDL_net.h>

struct Player {
	// technical, "infrastructural information"
	const char* name;
	IPaddress address;
	Uint8 player_id;

	// "pure" gameplay information
	float pos_x;
	float pos_y;
	int hp;
};
