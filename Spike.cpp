#include "Spike.h"

Spike::Spike(float x, float y, float s, ID3D11ShaderResourceView* t, int tp) {
	type = tp;

	height = 3 * s;
	width = 3 * s;

	xPos = x;
	yPos = y;
	tex = t;
	scale = s;
	rect = new Rect(width, height, x, y);
}



Spike::~Spike() {
}
