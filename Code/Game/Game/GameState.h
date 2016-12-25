#pragma once

#include "Player.h"

#include <array>

namespace eae6320
{

struct GameState
{
	Player ** const players;
	const size_t max_players;
	uint16_t local_player_id = ~0;

	GameState(size_t max_players);
	~GameState();

	void init_player(void (*update_callback)());

	Player * local_player() const
	{
		return local_player_id < max_players ? players[local_player_id] : NULL;
	}
	bool active() const
	{
		return local_player() != NULL;
	}
};

}
