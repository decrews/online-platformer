#include "Rect.h"


Rect::Rect(float width_in, float height_in, float x_in, float y_in) {
	width = width_in;
	height = height_in;
	x = x_in;
	y = y_in;
}



bool Rect::collides(Rect * rect) {
	if (x > rect->x - (rect->width / 2) && x < (rect->x + rect->width / 2)
		&& (y + (height / 2) > rect->y && y - (height / 2) < (rect->y))) {
		return true;
	}
	else {
		return false;
	}
}



Rect::~Rect() {
}
