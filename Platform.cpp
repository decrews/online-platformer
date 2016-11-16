#include "Platform.h"


Platform::Platform(float x, float y, float s, ID3D11ShaderResourceView* t, int tp) {
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



Platform::~Platform() {
}
