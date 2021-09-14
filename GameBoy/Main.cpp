#define OLC_PGE_APPLICATION

#include <vector>

#include "olcPixelGameEngine.h"
#include "CPU.h"
#include "Bus.h"
#include "PPU.h"

#define SCREEN_HEIGHT 144
#define SCREEN_WIDTH 160
#define PIXEL_SIZE 5

using namespace std;

static const olc::Pixel
	LIGHTEST(155, 188, 15), LIGHT(139, 172, 15), DARK(48, 98, 48), DARKEST(15, 56, 15);

class GameBoy : public olc::PixelGameEngine, public Display
{
private:
	int nClockSpeed = 4194304;
	float fResidualTime = 0.0f;
	int  nCycles = 0;
	int counter = 0;
	
	CPU cpu;
	Bus* bus;
	PPU ppu;

public:
	// Called once at the start, so create things here
	bool OnUserCreate() override
	{
		bus = new Bus(&cpu, &ppu, this);
		cpu.attachBus(bus);
		ppu.attachBus(bus);
		return true;
	}

	// called once per frame
	bool OnUserUpdate(float fElapsedTime) override
	{
		if (fResidualTime > 0.0f)
			fResidualTime -= fElapsedTime;
		else
		{
			fResidualTime += (1.0f / nClockSpeed) - fElapsedTime;

			// Get user input and process
			// ex. GetKey(olc::Key::W).bHeld

			// cycles for a frame
			for (int i = 0; i < 17556; i++) {
				if (counter % 4 == 0) {
					if (nCycles == 0) {
						if (!cpu.isStopped())
							nCycles = cpu.step();
					}
					nCycles--;
				}
				ppu.step();
				counter++;
			}
		}
		
		return true;
	}

	void drawScanline(u8 row, vector<u8> scanline) {
		for (int i = 0; i < 160; i++) {
			olc::Pixel p;
			switch (scanline[i]) {
			case 0:
				p = LIGHTEST;
				break;
			case 1:
				p = LIGHT;
				break;
			case 2:
				p = DARK;
				break;
			case 3:
				p = DARKEST;
				break;
			}

			Draw(i, row, p);
		}
	}

	void start() {
		Start();
	}
};

int main() {
	GameBoy gb;
	if (gb.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_SIZE, PIXEL_SIZE)) gb.start();
	/*CPU cpu;
	PPU ppu;
	Bus bus(&cpu, &ppu, &gb);
	cpu.attachBus(&bus);
	ppu.attachBus(&bus);
	int i = 0;
	while (!cpu.isStopped()) {
		if (i % 4 == 0)
			cpu.step();
		//ppu.step();
		i++;
	}*/
	return 0;
}