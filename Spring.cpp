#include "Spring.h"


Spring::Spring(float x, float y, float s, ID3D11ShaderResourceView* t, int tp) {
	type = tp;

	height = 4 * s;
	width = 4 * s;

	xPos = x;
	yPos = y;

	originX = x;
	originY = y;

	tex = t;
	scale = s;
	rect = new Rect(width, height, x, y);
}

bool Spring::groundCheck(Rect* rect) {
	if (this->rect->collides(rect)) {
		float diff = this->rect->y - (rect->y + rect->height / 2);
		if (diff < rect->height
			&& this->rect->x - (this->rect->width / 75) > rect->x - (rect->width / 2)
			&& this->rect->x + (this->rect->width / 75) < rect->x + (rect->width / 2)) {
			return true;
		}
		else {
			return false;
		}
		return false;
	}
}

bool Spring::wallCheck(Rect* rect) {
	if (this->rect->collides(rect)) {
		float diff = this->rect->x - (rect->x + rect->width / 2);
		if (diff < rect->width) {
			return true;
		}
		else {
			return false;
		}
		return false;
	}
}

Spring::~Spring() {
}
