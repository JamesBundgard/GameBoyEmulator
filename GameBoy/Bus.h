#pragma once
#include <string>
#include <vector>

#include "definitions.h"

class PPU;
class CPU;

class Bus {
private:
	std::vector<u8> memory;
	std::vector<u8> file;
	CPU* cpu;
	PPU* ppu;
	Display* display;

public:
	Bus(CPU* cpu, PPU* ppu, Display* display);
	u8 read(u16 addr);
	void write(u16 addr, u8 val);

	void renderScanline(u8 row, std::vector<u8> colours);
};