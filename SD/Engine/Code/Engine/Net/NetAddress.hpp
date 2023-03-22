#pragma once
#include "Engine/Net/NetCommon.hpp"

#include <stdint.h>
#include <string>
#include <vector>

struct NetAddress
{
public:
	unsigned int address = 0;
	uint16_t port = 0;

public:
	std::string ToString();

	static NetAddress FromString(std::string const& string);
	static NetAddress GetLoopBack(uint16_t port);
	static std::vector<NetAddress> GetAllInternals(uint16_t port);
};