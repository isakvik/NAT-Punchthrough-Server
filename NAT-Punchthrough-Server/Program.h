#pragma once

#include "Utils.h"
#include "Constants.h"

struct GameServer {
	sf::Uint32 id;

	sf::Int32 address;
	sf::Uint16 port;
	//bool online = true;

	int lastReceivedPacketTime = now_millis();
	int timeSinceLastResponse()
	{
		return now_millis() - lastReceivedPacketTime;
	}	
};

std::vector<GameServer> serverList;
std::vector<sf::Uint32> idList;
std::map<int, int> indexMap;

sf::UdpSocket serverSocket;

int main();

void registerGameServer(sf::IpAddress address, unsigned short port);
sf::Uint32 addGameServerToPacket(sf::Packet &inPacket, sf::Packet &outPacket, sf::IpAddress address, unsigned short port);
void notifyGameServer(sf::IpAddress clientAddress, unsigned short clientPort, sf::Uint32 serverID);