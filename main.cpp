#include "gfxlib/gfx.hpp"
#include <iostream>
#include <cmath>
using namespace std;

// define constants
const int screenw = 160, screenh = 240;
const Color PAL_BLACK = { 16, 8, 32, 255 };
const int
	Z_STARS         = -100,
	Z_EXPLOSION     = 10,
	Z_ENEMY         = 20,
	Z_BULLET        = 50,
	Z_PLAYER        = 100;
const int
	SCORE_SAUCER    = 10;

// define globals
GFX gfx;
Container scene;
shared_ptr<Sprite> shipptr;


// -- GAME --

struct Score {
	int score = 0, multiplier = 1;
	void add(int points) { score += points * multiplier; }
} score;

struct Explosion : Paintable {
	struct Star { float x, y, dx, dy; };
	vector<Star> stars;
	int alpha = 255;

	static void spawn(const Sprite& spr) {
		scene.append(make_shared<Explosion>(spr.x + spr.width/2, spr.y + spr.height/2));
	}

	Explosion(int mx, int my) {
		id = "explosion", x = mx, y = my, z = Z_EXPLOSION;
		for (int i = 0; i < 20; i++) {
			float rot = rand()%360, speed = ((rand()%3)+1)/2.0;
			auto point = gfx.rot2point(rot, speed);
			stars.push_back({ 0, 0, point.x, point.y });
		}
	}
	virtual void update() {
		if (alpha -= 5, alpha <= 0)
			return id = "dead", alpha = 0, void();
		for (auto& star : stars)
			star.x += star.dx, star.y += star.dy;
	}
	virtual void paint(int xoff, int yoff) {
		auto col = RED;  col.a = alpha;
		for (const auto& star : stars)
			DrawPixel(xoff+x+star.x, yoff+y+star.y, col);
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
		spr.x = x, spr.y = y, spr.z = Z_BULLET;
		spr.source("sprites", 16, 0, 8, 8);
		scene.append(sprptr);
	}

	void update() {
		cooldown = max(cooldown-1, 0);
		for (size_t i = 0; i < scene.children.size(); i++) {  // no iterator, since we are modifying scene.children
			if (scene.children[i]->id != "bullet")  continue;
			auto& spr = *dynamic_pointer_cast<Sprite>(scene.children[i]);
			// check collision with enemy
			for (size_t j = 0; j < scene.children.size(); j++) {
				if (scene.children[j]->id != "saucer")  continue;
				auto& spr2 = *dynamic_pointer_cast<Sprite>(scene.children[j]);
				if (spr.collide(spr2)) {
					score.add(SCORE_SAUCER);
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
		spr.x = 80, spr.y = -16, spr.z = Z_ENEMY;
		spr.tsource("sprites", 16, 2);
		scene.append(sprptr);
	}

	void update() {
		spawn();
		cooldown = max(cooldown-1, 0);
		for (size_t i = 0; i < scene.children.size(); i++) {
			if (scene.children[i]->id != "saucer")  continue;
			auto& spr = *dynamic_pointer_cast<Sprite>(scene.children[i]);
			// TODO: movement patterns
			spr.y += SPEED;
			if (spr.y > screenh + 20)
				spr.id = "dead";
			else if (shipptr->id != "dead" && spr.collide(shipptr)) {
				Explosion::spawn(spr);
				Explosion::spawn(*shipptr);
				spr.id = shipptr->id = "dead";
			}
		}
	}
} enemys;

struct Stars : Paintable {
	const float SPEED = 0.5;
	vector<Vector2> stars;

	Stars() {
		id = "stars", z = Z_STARS;
		srand(101);
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

void reset() {
	score.score = 0;
	auto& ship = *shipptr;
	ship.id = "ship", ship.x = (screenw-ship.width)/2, ship.y = screenh-ship.height-6;
	scene.children = { make_shared<Stars>(), shipptr };
}

void mainloop() {
	gfx.init(screenw*4, screenh*4);
	gfx.resizable();
	gfx.usebuffer(screenw, screenh);
	
	gfx.loadtexture("sprites", "assets/sprites.png");
	shipptr = make_shared<Sprite>();
	auto& ship = *shipptr;
		ship.tsource("sprites", 16, 0);
		ship.id = "ship", ship.z = Z_PLAYER;
	float speed = 1.5;

	reset();

	while (!gfx.shouldquit()) {
		ClearBackground(PAL_BLACK);

		// interface actions
		if (IsKeyDown(KEY_R))
			reset();
		// move player
		if (ship.id != "dead") {
			if (IsKeyDown(KEY_LEFT))
				ship.x = max(ship.x-speed, 0.0f);
			if (IsKeyDown(KEY_RIGHT))
				ship.x = min(ship.x+speed, float(screenw-ship.width));
			if (IsKeyDown(KEY_SPACE))
				bullets.shoot(ship.x+4, ship.y-9);
		}

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

	mainloop();
}
