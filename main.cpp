#include <iostream>
#include "gfxlib/gfx.hpp"
using namespace std;

// define globals
GFX gfx;
const int screenw = 160, screenh = 160;
Container scene;

struct Score {
	int score = 0;

	void add(const string& target) {
		if (target == "saucer")  score += 10;
	}
} score;

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
			auto& spr = *dynamic_pointer_cast<Sprite>(p);
			// check collision with enemy
			for (auto p : scene.children) {
				if (p->id != "saucer")  continue;
				auto& spr2 = *dynamic_pointer_cast<Sprite>(p);
				if (spr.collide(spr2)) {
					score.add(spr2.id);
					spr.id = spr2.id = "dead";
					goto next_bullet;
				}
			}
			// move
			spr.y -= SPEED;
			if (spr.y <= -12)  spr.id = "dead";
			next_bullet:
		}
		scene.remove("dead");
	}
} bullets;

struct Enemys {
	const int COOLDOWN_MAX = 20;
	const float SPEED = 1.0;
	int cooldown = COOLDOWN_MAX;

	void spawn() {
		if (cooldown > 0)  return;
		cooldown = COOLDOWN_MAX;
		auto sprptr = make_shared<Sprite>();
		auto& spr = *sprptr;
		spr.id = "saucer";
		spr.x = 80, spr.y = -16, spr.z = 10;
		spr.tsource("sprites", 16, 2);
		scene.append(sprptr);
	}

	void update() {
		spawn();
		cooldown = max(cooldown-1, 0);
		for (auto p : scene.children) {
			if (p->id != "saucer")  continue;
			auto& spr = *dynamic_pointer_cast<Sprite>(p);
			spr.y += SPEED;
			if (spr.y > screenh + 20)  spr.id = "dead";
		}
		scene.remove("dead");
	}
} enemys;

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
		enemys.update();

		// draw
		scene.paint(0, 0);
		gfx.font.selected = 1;
		gfx.print("score: "+to_string(score.score), 0, 0);
		gfx.print("actors:"+to_string(scene.children.size()), 0, 8);
		gfx.flip();
	}

	gfx.destroy();
}

int main() {
	printf("starting Shmup2...\n");

	buffertest2();
}
