#include <iostream>
#include "gfx/gfx.hpp"
using namespace std;

// define globals
GFX gfx;

void buffertest2() {
	gfx.init();
	gfx.resizable();
	gfx.usebuffer(160, 160);
	gfx.loadtexture("sprites", "../wizzardquest4/assets/sprites.png");
	Sprite s;
	s.tsource("sprites", 16, 2);
	s.x = s.y = 20;

	while (!gfx.shouldquit()) {
		ClearBackground(SKYBLUE);
		DrawRectangle(0, 0, 20, 20, RED);
		DrawCircle(10, 10, 5, MAROON);

		s.paint(0, 0);
		
		gfx.flip();
	}

	gfx.destroy();
}

int main() {
	printf("starting Shmup2...\n");

	buffertest2();
}
