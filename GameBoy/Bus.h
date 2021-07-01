#pragma once
#include <string>
#include <vector>

#include "definitions.h"

class Bus {
private:
	std::vector<u8> memory;
	std::vector<u8> file;
public:
	Bus();
	u8 read(u16 addr);
	void write(u16 addr, u8 val);
};