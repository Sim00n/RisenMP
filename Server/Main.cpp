#include "RakPeerInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include <stdio.h>

const unsigned MAX_CLIENTS = 10;
const unsigned PORT = 6666;

enum NETWORK_MESSAGE {
	ENTITY_POSITION = ID_USER_PACKET_ENUM + 1,
	ENTITY_ROTATION = ID_USER_PACKET_ENUM + 2
};

struct Vec3 {
	float x, y, z;
};

struct bCQuaternion {
	float x, y, z, theta;
};

int main()
{
	printf("Starting server on port %d ...\n", PORT);
	
	RakNet::Packet *packet = nullptr;

	RakNet::RakPeerInterface *peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::SocketDescriptor sd(PORT, 0);

	peer->Startup(MAX_CLIENTS, &sd, 1);
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);

	while (1)
	{
		while (packet = peer->Receive())
		{
			// Get the packet data into a BitStream
			RakNet::BitStream data(packet->data, packet->length, false);

			// Extract the first byte which tells us what kind of packet this is
			const unsigned char raknet_identifier = 0;
			data.Read(raknet_identifier);

			//switch ((unsigned) raknet_identifier)
			switch (packet->data[0])
			{
				case ID_NEW_INCOMING_CONNECTION: {
					printf("New incoming connection.\n");
				} break;
				case ID_DISCONNECTION_NOTIFICATION: {
					printf("A client has disconnected.\n");
				} break;
				case ID_CONNECTION_LOST: {
					printf("A client lost the connection.\n");
				} break;
				case ENTITY_POSITION: {
					Vec3 playerPosition;
					data.Read((char *)&playerPosition, sizeof(Vec3));
					printf("Players position: %f, %f, %f\n", playerPosition.x, playerPosition.y, playerPosition.z);
				} break;
				case ENTITY_ROTATION: {
					bCQuaternion playerRotation;
					data.Read((char *)&playerRotation, sizeof(bCQuaternion));
					printf("Players rotation: %f, %f, %f, %f\n", playerRotation.x, playerRotation.y, playerRotation.z, playerRotation.theta);
				} break;
				default: {
					printf("Received some other data. Here it is:\n");
					unsigned char *otherData = data.GetData();
					for (int i = 0; i < sizeof(otherData); i++) {
						printf("%d (%c) | ", *(otherData+i), *(otherData+i));
					}
					printf("-------------\n");
				} break;
			}

			peer->DeallocatePacket(packet);
		}

		Sleep(17); // freq = ~60Hz
	}

	return 0;
}