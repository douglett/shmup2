#include "gfxlib/gfx.hpp"
#include <iostream>
#include <cmath>
using namespace std;

// define globals
const int screenw = 160, screenh = 160;
const Color PAL_BLACK = { 16, 8, 32, 255 };
GFX gfx;
Container scene;

struct Score {
	int score = 0;

	void add(const string& target) {
		if (target == "saucer")  score += 10;
	}
} score;

struct Explosion : Paintable {
	struct Star { float x, y, dx, dy; };
	vector<Star> stars;

	static void spawn(const Sprite& spr) {
		scene.append(make_shared<Explosion>(spr.x + spr.width/2, spr.y + spr.height/2));
	}

	Explosion(int mx, int my) {
		id = "explosion", x = mx, y = my, z = 100;

		for (int i = 0; i < 20; i++) {
			float rot = rand()%360, speed = ((rand()%3)+1)/2.0;
			auto point = gfx.rot2point(rot, speed);
			stars.push_back({ 0, 0, point.x, point.y });
		}
	}
	virtual void update() {
		for (auto& star : stars)
			star.x += star.dx, star.y += star.dy;
	}
	virtual void paint(int xoff, int yoff) {
		for (const auto& star : stars)
			DrawPixel(xoff+x+star.x, yoff+y+star.y, RED);
		DrawPixel(xoff+x, yoff+y, GREEN);
	}
};

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
		for (size_t i = 0; i < scene.children.size(); i++) {  // no iterator, since we are modifying scene.children
			if (scene.children[i]->id != "bullet")  continue;
			auto p = dynamic_pointer_cast<Sprite>(scene.children[i]);  // hold ptr
			auto& spr = *p;
			// check collision with enemy
			for (auto p : scene.children) {
				if (p->id != "saucer")  continue;
				auto& spr2 = *dynamic_pointer_cast<Sprite>(p);
				if (spr.collide(spr2)) {
					score.add(spr2.id);
					spr.id = spr2.id = "dead";
					Explosion::spawn(spr2);
					goto next_bullet;
				}
			}
			// move
			spr.y -= SPEED;
			if (spr.y <= -12)  spr.id = "dead";
			next_bullet:
		}
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
	}
} enemys;

struct Stars : Paintable {
	const float SPEED = 0.5;
	vector<Vector2> stars;

	Stars() {
		id = "stars", z = -100;
		srand(100);
		for (int i = 0; i < 50; i++)
			stars.push_back({ float(rand() % screenw), float(rand() % screenh) });
	}
	virtual void update() {
		for (auto& star : stars)
			star.y = fmod(star.y + SPEED, screenh);
	}
	virtual void paint(int xoff, int yoff) {
		for (const auto& star : stars)
			DrawPixel(xoff+x+star.x, yoff+y+star.y, WHITE);
	}
};

void mainloop() {
	gfx.init(screenw*4, screenh*4);
	gfx.resizable();
	gfx.usebuffer(screenw, screenh);
	
	gfx.loadtexture("sprites", "assets/sprites.png");
	auto shipptr = make_shared<Sprite>();
	auto& ship = *shipptr;
		ship.tsource("sprites", 16, 0);
		ship.id = "ship";
		ship.x = (screenw - ship.width) / 2;
		ship.y = screenh - ship.height - 6;
		scene.append(shipptr);
	float speed = 1.5;

	auto starsptr = make_shared<Stars>();
	// auto& stars = *starsptr;
	scene.append(starsptr);

	while (!gfx.shouldquit()) {
		ClearBackground(PAL_BLACK);

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
		// update scene
		scene.update();
		// bring out ya dead!
		scene.remove("dead");

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

	// for (int i = 0; i < 4; i++) {
	// 	auto r = 360.0/4*i;
	// 	auto p = gfx.rot2point(r);
	// 	printf("%f: %f %f\n", r, p.x, p.y);
	// }

	mainloop();
}
