#pragma once
#include <D3D11.h>
#include "Rect.h"

class Platform
{
public:
	Platform(float x, float y, float s, ID3D11ShaderResourceView* t);
	~Platform();

	bool falling = false;
	float xPos;
	float yPos;
	float height;
	float width;
	float scale;
	Rect* rect;
	ID3D11ShaderResourceView* tex;
};
