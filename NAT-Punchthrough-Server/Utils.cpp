#include <cstring>
#include <iostream>
#include <chrono>

#include <SFML/Network.hpp>

#include "Utils.h"
#include "Constants.h"

int now_millis() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
		).count();
}

/// hash a block of memory 
/// http://create.stephan-brumme.com/fnv-hash/
unsigned int fnv1a(const void* data, size_t numBytes, unsigned int hash) {
	//std::cout << "-- hashing " << numBytes << " bytes with default value " << hash << "..." << std::endl;
	const byte* ptr = (const byte*)data;
	while (numBytes--)
		hash = (*ptr++ ^ hash) * FNV1A_PRIME;
	return hash;
}

sf::Uint32 generateServerID(sf::IpAddress address, unsigned short port) {
	std::string fullAddress = address.toString() + ":" + std::to_string(port);
	return fnv1a((void*) fullAddress.c_str(), fullAddress.size());
}

//////////// packet methods

bool verifyPacket(sf::Packet& packet) {
	sf::Uint8 protocol;
	if (!(packet >> protocol) || protocol != PROTOCOL_VERSION) {
		return false;
	}

	return true;
}

/*
bool verifyPacket(sf::Packet& packet) {
	sf::Uint8 protocol;
	sf::Uint32 fnv1aChecksum;
	if (!(packet >> protocol >> fnv1aChecksum)) {
		return false;
	}

	if (protocol != PROTOCOL_VERSION) {
		return false;
	}

	int offset = sizeof(protocol) + sizeof(fnv1aChecksum);
	sf::Uint32 fnv1aHash = fnv1a((byte*)packet.getData() + offset, packet.getDataSize() - offset);
	if (fnv1aChecksum != fnv1aHash) {
		return false;
	}
	return true;
}

 **
Because this method prepends a hash of the contents of the packet,
this method has to be called after all the data has been inserted.
* 
void prependPacketVerification(sf::Packet& packet) {
	sf::Uint32 checksum = fnv1a(packet.getData(), packet.getDataSize());
	int size = packet.getDataSize();

	byte* data = new byte[size];
	memcpy(data, packet.getData(), size);

	packet.clear();
	packet << PROTOCOL_VERSION << checksum;
	packet.append(data, size);

	delete data;
	data = NULL;
}
*/