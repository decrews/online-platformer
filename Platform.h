#pragma once
#include <D3D11.h>
#include "Rect.h"

class Platform
{
public:
	Platform(float x, float y, float s, ID3D11ShaderResourceView* t, int type);
	~Platform();

	bool falling = false;
	float fallingSpeed = 0.0001;
	int type;
	float xPos;
	float yPos;
	float height;
	float width;
	float scale;
	Rect* rect;
	ID3D11ShaderResourceView* tex;
};
