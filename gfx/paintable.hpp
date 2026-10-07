#pragma once
#include "raylib.h"
#include <memory>
#include <string>
using namespace std;

// paintable object base class
struct Paintable {
	using ptr = shared_ptr<Paintable>;
	string id;
	float x = 0, y = 0; int z = 0;
	virtual void paint (int offx, int offy) {}
	virtual void update() {}
};
