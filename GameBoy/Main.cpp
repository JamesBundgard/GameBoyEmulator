#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"
#include "CPU.h"
#include "Bus.h"

#define SCREEN_HEIGHT 100
#define SCREEN_WIDTH 150
#define PIXEL_SIZE 5

class GameBoy : public olc::PixelGameEngine
{
private:
	int nClockSpeed = 4194304;
	float fResidualTime = 0.0f;
	int  nCycles = 0;
	
	CPU cpu;
	Bus bus;

public:
	// Called once at the start, so create things here
	bool OnUserCreate() override
	{
		cpu.attachBus(&bus);
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
			if (nCycles == 0) {
				nCycles = cpu.step();
			}
			nCycles--;
		}
		return true;
	}

	void start() {
		Start();
		Draw(5, 5, olc::WHITE);
	}
};

int main() {
	//GameBoy gb;
	//if (gb.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_SIZE, PIXEL_SIZE)) gb.start();
	CPU cpu;
	Bus bus;
	cpu.attachBus(&bus);
	while (!cpu.isStopped()) {
		cpu.step();
	}
	return 0;
}