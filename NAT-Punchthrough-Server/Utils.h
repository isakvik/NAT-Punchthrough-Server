#pragma once

#include "Constants.h"

int now_millis();
unsigned int fnv1a(const void* data, size_t numBytes, unsigned int hash = FNV1A_SEED);
sf::Uint32 generateServerID(sf::IpAddress address, unsigned short port);

// packets
bool verifyPacket(sf::Packet& packet);
//void prependPacketVerification(sf::Packet& packet);