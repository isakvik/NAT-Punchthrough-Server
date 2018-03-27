#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

#include <SFML/Network.hpp>
#include <SFML/System.hpp>

#include "Program.h"
#include "Utils.h"
#include "Constants.h"

int main()
{
	if (serverSocket.bind(LISTEN_PORT) != sf::Socket::Done) {
		return -1;
	}
	serverSocket.setBlocking(false);
	std::cout << "listening on UDP port " << serverSocket.getLocalPort() << std::endl;

	while (true) {

		sf::Packet inPacket;
		sf::IpAddress address;
		sf::Uint16 port;

		if (serverSocket.receive(inPacket, address, port) != sf::Socket::Done) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		if (verifyPacket(inPacket)) {
			unsigned char who;
			inPacket >> who;

			if (who == 's') {
				// TODO: verify that server exists?

				// in case NAT changes the running port. 
				// we want clients to be able to look up servers
				sf::Uint16 serverLocalPort;
				if (!(inPacket >> serverLocalPort))
					serverLocalPort = port;

				sf::Packet outPacket;
				outPacket << PROTOCOL_VERSION << true;	// connection accepted

				registerGameServer(address, port, serverLocalPort);

				if (serverSocket.send(outPacket, address, port) == sf::Socket::Done) {
					std::cout << "sent response to server." << std::endl;
				}
			}
			else if (who == 'c') {
				// client
				// TODO: verify that client exists?

				sf::Packet outPacket;
				outPacket << PROTOCOL_VERSION;

				int serverID = addGameServerToPacket(inPacket, outPacket, address, port);

				if (serverID != 0) {
					if (notifyGameServer(address, port, serverID)) {
						if (serverSocket.send(outPacket, address, port) == sf::Socket::Done) {
							std::cout << "sent response to client." << std::endl;
						}
					}
					else {
						std::cout << "could not ping server!" << std::endl;
					}
				}

			}
		}
		else {
			std::cout << "discarded packet from " << address << ":" << port << ": failed verification." << std::endl;
		}

		// TODO: time out game servers
	}

	return 0;
}

void registerGameServer(sf::IpAddress address, unsigned short port, sf::Uint16 serverLocalPort) {
	int serverID = generateServerID(address, serverLocalPort);

	if (std::find(idList.begin(), idList.end(), serverID) == idList.end()) {
		// add server to list. serverLocalPort is used for lookup
		GameServer gameServer;
		gameServer.address = address.toInteger();
		gameServer.port = sf::Uint16(port);
		gameServer.id = serverID;

		serverList.push_back(gameServer);
		idList.push_back(serverID);
		indexMap.emplace(serverID, serverList.size());

		std::cout << "added " << address << ":" << port << " to server list." << std::endl;
		if(port != serverLocalPort)
			std::cout << "- local port reported as " << serverLocalPort << ", using that for lookup" << std::endl;
	}
	else {
		// update last received packet time (keep alive packet)
		std::map<int, int>::iterator it;
		int index = -1;

		it = indexMap.find(serverID);
		if (it != indexMap.end())
			index = it->second;

		if (index > -1) {
			serverList[index].lastReceivedPacketTime = now_millis();
			std::cout << "update from " << address << ":" << port << " received." << std::endl;
		}
	}
}

sf::Uint32 addGameServerToPacket(sf::Packet & inPacket, sf::Packet & outPacket, sf::IpAddress address, unsigned short port) {
	sf::Uint32 requestAddress;
	sf::Uint16 requestPort;

	if (!(inPacket >> requestAddress >> requestPort)) {
		std::cout << "could not extract address:port from request packet." << std::endl;
		return false;
	}

	sf::IpAddress parsedAddress = sf::IpAddress(requestAddress);
	sf::Uint32 serverID = generateServerID(parsedAddress, requestPort);

	/*
	std::cout << "- requested: " << parsedAddress.toString() << ":" << requestPort << " (id: " << serverID << ")" << std::endl;
	std::cout << "- found: " << std::endl;
	for (int i = 0; i < serverList.size(); i++) {
		std::cout << "- -- " << sf::IpAddress(serverList.at(i).address) << ":" << serverList.at(i).port << " (id: " << serverList.at(i).id << ")" << std::endl;
	}
	*/

	for (GameServer& server : serverList) {
		if (server.id == serverID) {
			outPacket << true;
			outPacket << server.address << server.port;
			return serverID;
		}
	}

	outPacket << false;
	return 0;
}
	
bool notifyGameServer(sf::IpAddress clientAddress, unsigned short clientPort, sf::Uint32 serverID) {
	sf::Packet outPacket;
	outPacket << PROTOCOL_VERSION << sf::Uint8(0xFF) << sf::Uint32(clientAddress.toInteger()) << sf::Uint16(clientPort);

	for (GameServer& server : serverList) {
		if (server.id == serverID) {
			sf::IpAddress serverAddress = sf::IpAddress(server.address);
			unsigned short serverPort = server.port;

			if (serverSocket.send(outPacket, serverAddress, serverPort) != sf::Socket::Done) {
				std::cout << "failed to notify game server!" << std::endl;
				return false;
			}

			return true;
		}
	}

	std::cout << "did not find server with ID " << serverID << "!" << std::endl;
	return false;
}