#include <string>
#include <iostream>
#include <fstream>

#include "Bus.h"
#include "definitions.h"

using namespace std;

Bus::Bus() {
	//ifstream bootstrapStream("BootstrapROM.bin", ios::binary);
	//vector<u8> bootstrap((istreambuf_iterator<char>(bootstrapStream)), istreambuf_iterator<char>());

	//memory = bootstrap;

	//const string inputFile = "individual\\01-special.gb"; // "4F7E89D2 DAA Failed #6"
	const string inputFile = "individual\\02-interrupts.gb"; // EI Failed #2
	//const string inputFile = "individual\\03-op sp,hl.gb"; // Passed!
	//const string inputFile = "individual\\04-op r,imm.gb"; // Passed!
	//const string inputFile = "individual\\05-op rp.gb"; // Passed!
	//const string inputFile = "individual\\06-ld r,r.gb"; // Passed!
	//const string inputFile = "individual\\07-jr,jp,call,ret,rst.gb"; // Passed!
	//const string inputFile = "individual\\08-misc instrs.gb"; // Passed!
	//const string inputFile = "individual\\09-op r,r.gb"; // Passed!
	//const string inputFile = "individual\\10-bit ops.gb"; // Passed!
	//const string inputFile = "individual\\11-op a,(hl).gb"; // Failed: 27 (DAA)


	ifstream stream(inputFile, ios::binary);
	vector<u8> contents((istreambuf_iterator<char>(stream)), istreambuf_iterator<char>());
	file = contents;

	if (file.size() == 0) throw new exception("Error Reading File");
	memory.resize(0x10000, 0);
	memory.insert(memory.begin(), file.begin(), file.begin() + 0x8000);
}

u8 Bus::read(u16 addr) {
	if (0 <= addr && addr <= 0x3FFF) { // Bank 0
		return memory[addr];
	}
	else if (0x4000 <= addr && addr <= 0x7FFF) { // Switchable Bank 01..NN
		return memory[addr];
	}
	else if (0x8000 <= addr && addr <= 0x9FFF) { // Video RAM (VRAM)
		return memory[addr];
	}
	else if (0xA000 <= addr && addr <= 0xBFFF) { // External RAM
		return memory[addr];
	}
	else if (0xC000 <= addr && addr <= 0xCFFF) { // Work RAM Bank 0 (WRAM)
		return memory[addr];
	}
	else if (0xD000 <= addr && addr <= 0xDFFF) { // Work RAM Bank 1 (WRAM)
		return memory[addr];
	}
	else if (0xE000 <= addr && addr <= 0xFDFF) { // Same as C000-DDFF (ECHO) (typically not used)
		return memory[addr];
	}
	else if (0xFE00 <= addr && addr <= 0xFE9F) { // Sprite Attribute Table (OAM)
		return memory[addr];
	}
	else if (0xFEA0 <= addr && addr <= 0xFEFF) { // Not Usable
		return memory[addr];
	}
	else if (0xFF00 <= addr && addr <= 0xFF7F) { // I/O Ports
		return memory[addr];
	}
	else if (0xFF80 <= addr && addr <= 0xFFFE) { // High RAM (HRAM)
		return memory[addr];
	}
	else if (0xFFFF <= addr && addr <= 0xFFFF) { // Interupt Enable Register
		return memory[addr];
	}
	else {
		throw new exception("INVALID ADDRESS");
	}
}

void Bus::write(u16 addr, u8 val) {
	if (0 <= addr && addr <= 0x3FFF) { // Bank 0
		if (0x2000 <= addr && addr <= 0x3FFF) {
			int bankNumber = addr & 0x1F;
			memory.insert(memory.begin() + 0x4000, file.begin() + 0x4000 * bankNumber, file.begin() + 0x4000 * (bankNumber + 1));
		}
		/*else {
			memory[addr] = val;
		}*/
	}
	else if (0x4000 <= addr && addr <= 0x7FFF) { // Switchable Bank 01..NN
		 //memory[addr] = val;
	}
	else if (0x8000 <= addr && addr <= 0x9FFF) { // Video RAM (VRAM)
		memory[addr] = val;
	}
	else if (0xA000 <= addr && addr <= 0xBFFF) { // External RAM
		memory[addr] = val;
	}
	else if (0xC000 <= addr && addr <= 0xCFFF) { // Work RAM Bank 0 (WRAM)
		memory[addr] = val;
	}
	else if (0xD000 <= addr && addr <= 0xDFFF) { // Work RAM Bank 1 (WRAM)
		memory[addr] = val;
	}
	else if (0xE000 <= addr && addr <= 0xFDFF) { // Same as C000-DDFF (ECHO) (typically not used)
		memory[addr] = val;
	}
	else if (0xFE00 <= addr && addr <= 0xFE9F) { // Sprite Attribute Table (OAM)
		memory[addr] = val;
	}
	else if (0xFEA0 <= addr && addr <= 0xFEFF) { // Not Usable
		memory[addr] = val;
	}
	else if (0xFF00 <= addr && addr <= 0xFF7F) { // I/O Ports
		memory[addr] = val;
		if (addr == 0xFF02 && val == 0x81) cout << memory[0xFF01] << flush;
		//if (addr == 0xFF01) cout << val << flush;
	}
	else if (0xFF80 <= addr && addr <= 0xFFFE) { // High RAM (HRAM)
		memory[addr] = val;
	}
	else if (0xFFFF <= addr && addr <= 0xFFFF) { // Interupt Enable Register
		memory[addr] = val;
	}
	else {
		throw new exception("INVALID ADDRESS");
	}
	return;
}