#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <set>
#include <iterator>
#include <algorithm>

#include "PPU.h"
#include "definitions.h"

using namespace std;

const u16 LCDC = 0xFF40;

// See https://gbdev.io/pandocs/Rendering.html
// Palettes: BGP (0xFF47), OBP0 (0xFF48), OBP1 (0xFF49)

vector<u16> tileToRowColours(Tile tile) {
	vector<u16> res;
	for (auto& row : tile.data) {
		u16 colours = 0;
		u8 ptr = 0x80;
		for (int i = 7; i >= 0; i--) {
			u8 colour = (((row[1] & ptr) >> i) << 1) | ((row[0] & ptr) >> i);
			colours |= (colour << (2 * i));
			ptr >>= 1;
		}
		res.push_back(colours);
	}
}

vector<Sprite> filterSprites(u8 scanline, vector<Sprite> sprites) {
	vector<Sprite> res;
	u8 height = (sprites[0].tiles.size() > 1) ? 16 : 8;
	for (auto sprite : sprites) {
		if (res.size() >= 10) break;
		int yPosInScreen = sprite.yPos - 16;
		if (yPosInScreen <= scanline && scanline <= yPosInScreen + height) {
			res.push_back(sprite);
		}
	}
	return res;
}

PPU::PPU() {}

int PPU::step() {
	if (this->DMA > 0) {
		u8 page = this->bus->read(0xFF46);
		u16 offset = 160 - this->DMA;
		u16 srcAddr = page << 8 | offset;
		u16 destAddr = 0xFE00 | offset;
		this->bus->write(destAddr, this->bus->read(srcAddr));
		this->DMA--;
	}
	else if (this->getLcdEnable() == 0) {
		return;
	}
	else {
		this->setLY(this->scanline);
		switch (mode) {
		case 0: // HBlank
			this->cycles += 1;
			if (this->cycles == 22) {
				this->cycles = 0;
				this->setMode(2);
				this->scanline += 1;
				if (this->scanline == 144) this->setMode(1);
			}
			return;
		case 1: // VBlank
			this->cycles += 1;
			if (this->cycles == 114) {
				this->cycles = 0;
				this->scanline += 1;
				if (this->scanline == 154) {
					this->scanline = 0;
					this->setMode(2);
				}
			}
			return;
		case 2: // Searching OAM
			this->cycles += 1;
			if (this->cycles == 20) {
				this->cycles = 0;
				this->setMode(3);
			}
			return;
		case 3: // Transferring Data to LCD Controller
			this->cycles += 1;
			if (this->cycles == 72) {
				// Create scanline
				vector<u16> line = this->generateScanline();
				// Send scanline to bus for display
				this->bus->renderScanline(line);
				this->cycles = 0;
				this->setMode(0);
			}
			return;
		}
	}
}

bool spriteCompare(Sprite i, Sprite j) {
	return i.xPos > j.xPos; // smaller xPos has priority -> render last
}


std::vector<u16> PPU::generateScanline() {
	vector<s8> line(160);
	if (this->getObjEnable() == 1) {
		vector<Sprite> sprites = filterSprites(this->scanline, this->getSprites());
		sort(sprites.begin(), sprites.end(), spriteCompare);
		for (auto sprite : sprites) {
			int xPos = sprite.xPos - 8;
			int width = 8;
			if (xPos < 0) {
				width = xPos + 8;
			}
			else if (xPos > 160) {
				width = 168 - xPos;
			}
			if (width <= 0) continue;


		}
	}
}

Tile PPU::getTile(u8 index) {
	u16 addr = 0x8000 + index * 16;
	Tile t;
	for (int i = 0; i < 8; i++) {
		t.data[i].push_back(this->bus->read(addr++));
		t.data[i].push_back(this->bus->read(addr++));
	}
	return t;
}

Tile PPU::getTileSigned(s8 index) {
	u16 addr = 0x8800 + index * 16;
	Tile t;
	for (int i = 0; i < 8; i++) {
		t.data[i].push_back(this->bus->read(addr++));
		t.data[i].push_back(this->bus->read(addr++));
	}
	return t;
}

Tile PPU::getSprite(u8 index) {
	u16 addr = 0x9000 + index * 16;
	Tile t;
	for (int i = 0; i < 8; i++) {
		t.data[i].push_back(this->bus->read(addr++));
		t.data[i].push_back(this->bus->read(addr++));
	}
	return t;
}

vector<vector<Tile>> PPU::getTileMap(u8 index) {
	u16 addr = (index == 0) ? 0x9800 : 0x9C00;
	vector<vector<Tile>> map(32);
	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 32; j++) {
			if (this->getTileIndexType() == 0) {
				map[i][j] = this->getTile(this->bus->read(addr++));
			}
			else {
				map[i][j] = this->getTileSigned((s8) this->bus->read(addr++));
			}
		}
	}
	return map;
}

vector<Sprite> PPU::getSprites() {
	u16 addr = 0xFE00;
	vector<Sprite> list;
	for (int i = 0; i < 40; i++) {
		Sprite s;
		s.yPos = this->bus->read(addr++);
		s.xPos = this->bus->read(addr++);

		u8 tilePos = this->bus->read(addr++);

		if (this->getTileIndexType() == 0) {
			s.tiles.push_back(this->getTile(tilePos));
		}
		else {
			s.tiles.push_back(this->getTileSigned((s8)tilePos));
		}

		if ((this->bus->read(LCDC) >> 2) & 0x1) {
			tilePos++;
			if (this->getTileIndexType() == 0) {
				s.tiles.push_back(this->getTile(tilePos));
			}
			else {
				s.tiles.push_back(this->getTileSigned((s8)tilePos));
			}
		}

		s.attributes = this->bus->read(addr++);

		list.emplace_back(s);
	}
	return list;
}

void PPU::attachBus(Bus* bus) {
	this->bus = bus;
}

void PPU::setLCDC(u8 val) {
	this->bus->write(LCDC, val);
}

u8 PPU::getLcdEnable() {
	u8 lcdc = this->bus->read(LCDC);
	return (lcdc >> 7) & 0x1;
}

u8 PPU::getWindowIndex() {
	u8 lcdc = this->bus->read(LCDC);
	return (lcdc >> 6) & 0x1;
}

u8 PPU::getWindowEnable() {
	u8 lcdc = this->bus->read(LCDC);
	return (lcdc >> 5) & 0x1;
}

u8 PPU::getTileIndexType() {
	u8 lcdc = this->bus->read(LCDC);
	return (lcdc >> 4) & 0x1;
}

u8 PPU::getBackgroundIndex() {
	u8 lcdc = this->bus->read(LCDC);
	return (lcdc >> 3) & 0x1;
}

u8 PPU::getObjSize() {
	u8 lcdc = this->bus->read(LCDC);
	return (lcdc >> 2) & 0x1;
}

u8 PPU::getObjEnable() {
	u8 lcdc = this->bus->read(LCDC);
	return (lcdc >> 1) & 0x1;
}

u8 PPU::getBgAndWindowEnablePriority() {
	u8 lcdc = this->bus->read(LCDC);
	return (lcdc >> 0) & 0x1;
}


u8 PPU::getSCY() {
	return this->bus->read(0xFF42);
}

u8 PPU::getSCX() {
	return this->bus->read(0xFF43);
}

u8 PPU::getWY() {
	return this->bus->read(0xFF42);
}

u8 PPU::getWX() {
	return this->bus->read(0xFF43) - 7;
}


void PPU::setLycLyInteruptSource(bool b) {
	u8 status = this->bus->read(0xFF41);
	status |= b << 6;
	this->bus->write(0xFF41, status);
}

void PPU::setOamInteruptSource(bool b) {
	u8 status = this->bus->read(0xFF41);
	status |= b << 5;
	this->bus->write(0xFF41, status);
}

void PPU::setVBlankInteruptSource(bool b) {
	u8 status = this->bus->read(0xFF41);
	status |= b << 4;
	this->bus->write(0xFF41, status);
}

void PPU::setHBlankInteruptSource(bool b) {
	u8 status = this->bus->read(0xFF41);
	status |= b << 3;
	this->bus->write(0xFF41, status);
}

void PPU::setLycLyFlag(bool b) {
	u8 status = this->bus->read(0xFF41);
	status &= (b << 2) | 0x0B;
	this->bus->write(0xFF41, status);
	// TODO: call interupt if needed
}

void PPU::setMode(u8 mode) {
	this->mode = mode;
	u8 status = this->bus->read(0xFF41);
	status |= mode;
	this->bus->write(0xFF41, status);
}

void PPU::setLY(u8 scanline) {
	this->bus->write(0xFF44, scanline);
	u8 lyc = this->bus->read(0xFF45);
	this->setLycLyFlag(lyc == scanline);
}

void PPU::handleLycSet() {
	u8 lyc = this->bus->read(0xFF45);
	this->setLycLyFlag(lyc == this->scanline);
}

void PPU::triggerDMA() {
	this->DMA = 160;
}