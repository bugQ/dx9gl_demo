#pragma once

#include "../../External/RakNet/RakNetTypes.h"
#include "GameState.h"

namespace eae6320
{

class Neterface
{
	static Neterface * Instance;
	
	RakNet::RakPeerInterface * const peer;
	RakNet::SystemAddress address;

	GameState & game_state;
public:
	const bool is_server;

	// if host is NULL, starts server instead
	Neterface(const char *host, unsigned short port, GameState & game_state);
	~Neterface();

	void Update();
	void SendPlayerUpdate() const;

	static void PlayerUpdateCallback() { Instance->SendPlayerUpdate(); }
};

}