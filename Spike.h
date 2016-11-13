
#pragma once
#include <D3D11.h>
#include "Rect.h"

class Spike
{
public:
	Spike(float x, float y, float s, ID3D11ShaderResourceView* t, int type);
	~Spike();

	bool falling = false;
	float fallingSpeed = 0.00005;
	int type;

	float xPos;
	float yPos;
	float xVel = 0;
	float yVel = 0;

	float height;
	float width;
	float scale;
	Rect* rect;
	ID3D11ShaderResourceView* tex;
};
