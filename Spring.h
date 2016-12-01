#pragma once
#include <D3D11.h>
#include "Rect.h"
#include "Level.h"
#include "STRUCTS.h"

class Spring
{
public:
	Spring(float x, float y, float s, ID3D11ShaderResourceView* t, int type);
	~Spring();

	bool falling = false;
	float fallingSpeed = 0.00005;
	int type;

	bool groundCheck(Rect* rect);
	bool wallCheck(Rect* rect);

	float xPos;
	float yPos;
	float originX;
	float originY;
	float xVel = 0;
	float yVel = 0;

	float height;
	float width;
	float scale;
	Rect* rect;
	ID3D11ShaderResourceView* tex;
};
