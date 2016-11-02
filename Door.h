#pragma once
#include <D3D11.h>
#include "Rect.h"

class Door
{
public:
	Door(float x, float y, float s, ID3D11ShaderResourceView* t);
	~Door();

	float xPos;
	float yPos;

	float height;
	float width;
	float scale;
	Rect* rect;
	ID3D11ShaderResourceView* tex;
};