#include "gfxlib/gfx.hpp"
#include <iostream>
#include <cmath>
#include <format>
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
	int score = 0, multiplier = 1, dist = 0;
	void add(int points) { score += points * multiplier; }
	void reset() { score = dist = 0, multiplier = 1; }
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
		auto snapshot = scene.snapshot();
		for (auto p : snapshot) {
			if (p->id != "bullet")  continue;
			auto& spr = *dynamic_pointer_cast<Sprite>(p);
			// move
			spr.y -= SPEED;
			if (spr.y <= -12) {
				spr.id = "dead";
				continue;
			}
			// check collision with enemy
			for (auto p : snapshot) {
				if (!(p->id == "saucer" || p->id == "saucer:2"))  continue;
				auto& spr2 = *dynamic_pointer_cast<Sprite>(p);
				if (spr.collide(spr2)) {
					score.add(SCORE_SAUCER);
					spr.id = spr2.id = "dead";
					Explosion::spawn(spr2);
					break;
				}
			}
		}
	}
} bullets;

struct Enemys {
	inline const static float SPEED = 1.0;
	const static int COOLDOWN_MAX = 10;
	int cooldown = COOLDOWN_MAX;

	void update() {
		if (--cooldown > 0)  return;
		cooldown = COOLDOWN_MAX;
		scene.append(make_shared<Saucer2>());
	}

	// Straight-Down Saucers
	struct Saucer : Sprite {
		Saucer() {
			tsource("sprites", 16, 2);
			id = "saucer", x = (screenw-width)/2, y = -height;
		}
		virtual void update() {
			y += SPEED;
			checkdead();
		}
		virtual void checkdead() {
			if (y > screenh + 20)
				id = "dead";
			else if (shipptr->id != "dead" && collide(shipptr)) {
				Explosion::spawn(*this);
				Explosion::spawn(*shipptr);
				id = shipptr->id = "dead";
			}
		}
	};

	// Wobbling Saucers
	struct Saucer2 : Saucer {
		float dir = 90;
		Saucer2() : Saucer() {
			id = "saucer:2";
		}
		virtual void update() {
			dir += 5;
			auto pos = gfx.rot2point(dir, SPEED);
			x += pos.x*1.5, y += fabs(pos.y);
			checkdead();
		}
	};
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
	score.reset();
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
		score.dist++;

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
		gfx.printm({
			format("{}pt", score.score),
			format("{:.2f}km", score.dist/1000.0f),
			format("actors: {}",scene.children.size())
		}, 0, 0);
		gfx.flip();
	}

	gfx.destroy();
}

int main() {
	printf("starting Shmup2...\n");

	mainloop();
}
