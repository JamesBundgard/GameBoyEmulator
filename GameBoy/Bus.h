#pragma once
#include <string>
#include <vector>

#include "definitions.h"
#include "CPU.h"
#include "PPU.h"

class Bus {
private:
	std::vector<u8> memory;
	std::vector<u8> file;
	CPU* cpu;
	PPU* ppu;
public:
	Bus(CPU* cpu, PPU* ppu);
	u8 read(u16 addr);
	void write(u16 addr, u8 val);

	void renderScanline(std::vector<u16> scanline);
};