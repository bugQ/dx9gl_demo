#include "Neterface.h"

#include "../../External/RakNet/RakPeerInterface.h"
#include "../../External/RakNet/MessageIdentifiers.h"
#include "../../External/RakNet/BitStream.h"

#include "../../Engine/Debug_Runtime/UserOutput.h"

#include <cassert>

namespace eae6320
{
Neterface * Neterface::Instance = NULL;

enum GameMessages
{
	ASSIGN_CLIENT_ID = ID_USER_PACKET_ENUM,
	PLAYER_UPDATE,
};

#define SEND(bits, broadcast) (peer)->Send((bits), HIGH_PRIORITY, RELIABLE_ORDERED, 0, is_server ? RakNet::AddressOrGUID() : address, (broadcast));

Neterface::Neterface(const char *host, unsigned short port, GameState & game_state)
	: is_server(host == NULL), peer(RakNet::RakPeerInterface::GetInstance())
	, game_state(game_state)
{
	assert(Instance == NULL);
	assert(peer != NULL);
	Instance = this;

	if (is_server) {
		RakNet::SocketDescriptor sock(port, NULL);
		peer->Startup(1, &sock, 1);
		peer->SetMaximumIncomingConnections(1);
		game_state.local_player_id = 0;
		game_state.init_player(PlayerUpdateCallback);
	}
	else {
		RakNet::SocketDescriptor sock;
		peer->Startup(1, &sock, 1);
		peer->Connect(host, port, NULL, 0);
		peer->SetMaximumIncomingConnections(1);
	}
}


Neterface::~Neterface()
{
	RakNet::RakPeerInterface::DestroyInstance(peer);
	assert(Instance != NULL);
	Instance = NULL;
}

void Neterface::Update()
{
	RakNet::Packet *packet;
	for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
	{
		RakNet::BitStream in_bits(packet->data, packet->length, false);
		RakNet::BitStream out_bits;
		RakNet::MessageID msg_id;
		in_bits.Read(msg_id);

		switch (msg_id)
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			printf("Another client has disconnected.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			printf("Another client has lost the connection.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			printf("Another client has connected.\n");
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			printf("Our connection request has been accepted.\n");
			address = packet->systemAddress;
			
			break;
		case ID_NEW_INCOMING_CONNECTION:
			printf("A connection is incoming.\n");
			address = packet->systemAddress;

			out_bits.Write((RakNet::MessageID) ASSIGN_CLIENT_ID);
			out_bits.Write((uint16_t)1);
			peer->Send(&out_bits, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);

			//m_totalClients++;
			//SendPlayerPosition();
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("The server is full.\n");
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			if (is_server) {
				printf("A client has disconnected.\n");
			}
			else {
				printf("We have been disconnected.\n");
			}
			break;

		case ASSIGN_CLIENT_ID:
			uint16_t local_player_id;

			in_bits.Read(local_player_id);

			game_state.local_player_id = local_player_id;
			game_state.init_player(PlayerUpdateCallback);
			assert(game_state.active());
			break;
		
		case PLAYER_UPDATE:
			uint16_t remote_player_id;
			Vector3 remote_player_pos;
			float remote_player_yaw;
			
			in_bits.Read(remote_player_id);
			in_bits.Read(remote_player_pos.x);
			in_bits.Read(remote_player_pos.y);
			in_bits.Read(remote_player_pos.z);
			in_bits.Read(remote_player_yaw);
			
			if (game_state.players[remote_player_id] == NULL)
				game_state.players[remote_player_id] = new Player(remote_player_pos, remote_player_yaw);
			else
				game_state.players[remote_player_id]->remote_update(remote_player_pos, remote_player_yaw);
			break;


		}

		out_bits.Reset();
	}
}

void Neterface::SendPlayerUpdate() const
{
	RakNet::BitStream out_bits;
	Player * player = game_state.local_player();

	out_bits.Write((RakNet::MessageID) PLAYER_UPDATE);
	out_bits.Write(game_state.local_player_id);
	out_bits.Write(player->position.x);
	out_bits.Write(player->position.y);
	out_bits.Write(player->position.z);
	out_bits.Write(player->yaw);
	peer->Send(&out_bits, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, is_server ? RakNet::UNASSIGNED_SYSTEM_ADDRESS : address, is_server);
}

}