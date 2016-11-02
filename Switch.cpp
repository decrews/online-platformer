#include "Switch.h"


Switch::Switch(float x, float y, float s, ID3D11ShaderResourceView* t) {

	height = 4 * s;
	width = 4 * s;

	xPos = x;
	yPos = y;
	tex=  t;
	scale = s;
	rect = new Rect(width, height, x, y);

}



Switch::~Switch() {
}
