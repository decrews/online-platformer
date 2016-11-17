#pragma once
#include <D3D11.h>
#include "Rect.h"
#include "STRUCTS.h"

class Pawn
{
public:
	Pawn(float x, float y, ID3D11ShaderResourceView* t, int id);
	~Pawn();
	void draw(VS_CONSTANT_BUFFER* cbuffer, ID3D11DeviceContext* gcontext,
		ID3D11VertexShader* vs, ID3D11PixelShader* ps,
		ID3D11Buffer* constBuffer, ID3D11SamplerState* sampler,
		ID3D11Buffer* vb, UINT stride, UINT offset
	);
	void update(float offset, float y, long elapsed_microseconds);

	int id;

	float xPos;
	float yPos;

	float offset;

	float height;
	float width;
	float scale;
	Rect* rect;
	ID3D11ShaderResourceView* tex;
};

