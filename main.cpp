#include <iostream>
#include "gfxlib/gfx.hpp"
using namespace std;

// define globals
GFX gfx;
const int screenw = 160, screenh = 160;
Container scene;

struct Bullets {
	const int COOLDOWN_MAX = 10;
	const float SPEED = 2.0;
	int cooldown = 0;

	void shoot(int x, int y) {
		if (cooldown > 0)  return;
		cooldown = COOLDOWN_MAX;
		auto sprptr = make_shared<Sprite>();
		auto& spr = *sprptr;
		spr.id = "bullet";
		spr.x = x, spr.y = y, spr.z = 100;
		spr.source("sprites", 16, 0, 8, 8);
		scene.append(sprptr);
	}

	void update() {
		cooldown = max(cooldown-1, 0);
		for (auto p : scene.children) {
			if (p->id != "bullet")  continue;
			auto& b = *dynamic_pointer_cast<Sprite>(p);
			b.y -= SPEED;
			if (b.y <= -12)  b.id = "dead";

			// auto p2 = dynamic_pointer_cast<Sprite>(p);
			// // p2->y -= SPEED;
			// auto& 
		}
		scene.remove("dead");
	}
} bullets;

void buffertest2() {
	gfx.init(screenw*4, screenh*4);
	gfx.resizable();
	gfx.usebuffer(screenw, screenh);
	
	gfx.loadtexture("sprites", "assets/sprites.png");
	auto shipptr = make_shared<Sprite>();
	auto& ship = *shipptr;
	ship.tsource("sprites", 16, 0);
	ship.x = (screenw - ship.width) / 2;
	ship.y = screenh - ship.height - 6;
	scene.append(shipptr);
	float speed = 1.5;

	while (!gfx.shouldquit()) {
		ClearBackground(SKYBLUE);
		// DrawRectangle(0, 0, 20, 20, RED);
		// DrawCircle(10, 10, 5, MAROON);

		// move player
		if (IsKeyDown(KEY_LEFT))
			ship.x = max(ship.x-speed, 0.0f);
		if (IsKeyDown(KEY_RIGHT))
			ship.x = min(ship.x+speed, float(screenw-ship.width));
		if (IsKeyDown(KEY_SPACE))
			bullets.shoot(ship.x+4, ship.y-9);

		// move actors
		bullets.update();

		// draw
		scene.paint(0, 0);
		gfx.font.selected = 1;
		gfx.print("actors:"+to_string(scene.children.size()), 2, 2);
		gfx.flip();
	}

	gfx.destroy();
}

int main() {
	printf("starting Shmup2...\n");

	buffertest2();
}
