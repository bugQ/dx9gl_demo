#include "GameState.h"

#include <cassert>


namespace eae6320
{

GameState::GameState(size_t max_players)
	: max_players(max_players)
	, players(new Player *[max_players]())
{
}


GameState::~GameState()
{
	for (size_t i = 0; i < max_players; ++i)
		delete players[i];
	delete[] players;
}

void GameState::init_player(void (*update_callback)())
{
	assert(local_player_id < max_players);

	float yaw = local_player_id * 6.283185307f / max_players;
	Vector3 position = Versor::rotation_y(yaw).rotate(-Vector3::K * 2);
	Graphics::Color color = team_color(local_player_id);

	players[local_player_id] = new Player(color, position, yaw);
	players[local_player_id]->update_callback = update_callback;
}

Graphics::Color GameState::team_color(uint16_t player_id)
{
	return Graphics::Color::fromHSV(360.0f * player_id / max_players, 0.25f, 0.93f);
}

}
