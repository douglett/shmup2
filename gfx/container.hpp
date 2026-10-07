#pragma once
#include "raylib.h"
#include "paintable.hpp"
#include <vector>
#include <algorithm>
using namespace std;

// paintable object with children
struct Container : Paintable {
	vector<Paintable::ptr> children;

	void append(Paintable::ptr p) { children.push_back(p); }
	void remove(Paintable::ptr p) { children.erase(find(children.begin(), children.end(), p)); }

	void remove(const string& id) {
		for (int i = 0; i < (int)children.size(); i++)
			if (children.at(i)->id == id)
				children.erase(children.begin()+i), i--;
	}
	
	virtual void paint(int xoff, int yoff) {
		for (auto &c : children)
			c->paint(xoff + x, yoff + y);
	}

	virtual void update() {
		for (auto &c : children)
			c->update();
	}
};
