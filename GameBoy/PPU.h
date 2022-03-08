#pragma once

#include <string>
#include <vector>
#include <functional>

#include "definitions.h"
#include "Bus.h"

struct Tile {
    Tile() : data(8) {}
    std::vector<std::vector<u8>> data;
};

struct Sprite {
    std::vector<Tile> tiles;
    u8 xPos;
    u8 yPos;
    u8 attributes;
};

class PPU {
private:
    Bus* bus = nullptr;

    u8 LY = 0;
    u8 WLC = 0; // window line counter

    u8 DMA = 0;
    u8 scanline = 0; // 154 per frame, 144-153 are VBlank
    u8 cycles = 0; // 456 dots (114 cpu cycles) per scanline
    u8 mode = 2;

    bool doneFrame = false;
    std::vector<u8> currentLine;

    //void triggerVBlank();
    //void triggerLCDC();

    std::span<u8> getTile(u8 index);
    std::span<u8> getTileSigned(s8 index);
    Tile getSprite(u8 index);
    void getTileMapRow(u8 index, u8 row, u8 x, u8 rowIndex);
    std::vector<Sprite> getSprites();

    u8 getBackgroundIndex();
    u8 getTileIndexType();

    u8 getSCY();
    u8 getSCX();
    u8 getWY();
    u8 getWX();
    u8 getLcdEnable();
    u8 getWindowIndex();
    u8 getObjSize();
    u8 getObjEnable();
    u8 getWindowEnable();
    u8 getBgAndWindowEnablePriority();

    void setLycLyInteruptSource(bool b);
    void setOamInteruptSource(bool b);
    void setVBlankInteruptSource(bool b);
    void setHBlankInteruptSource(bool b);
    void setLycLyFlag(bool b);
    void setMode(u8 mode);
    void setLY(u8 scanline);

    void generateScanline();
public:
    PPU();
    void step();
    void attachBus(Bus* bus);

    void setLCDC(u8 val);
    void triggerDMA();

    void handleLycSet();
    bool isDoneFrame() { return doneFrame; }
};