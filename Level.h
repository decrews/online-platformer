#pragma once
#include <vector>
#include <D3D11.h>
#include "Platform.h"
#include "Door.h"
#include "STRUCTS.h"

class Level
{
public:
	Level();
	~Level();
	void draw(VS_CONSTANT_BUFFER* cbuffer, ID3D11DeviceContext* gcontext,
		ID3D11VertexShader* vs, ID3D11PixelShader* ps,
		ID3D11Buffer* constBuffer, ID3D11SamplerState* sampler,
		ID3D11Buffer* vb, UINT stride, UINT offset
		);
	void update(long elapsed_microseconds);

	ID3D11ShaderResourceView* bgTex; // Background Texture
	float bgPos = 0;

	std::vector<Platform*> blocks;
	std::vector<Platform*> fallingBlocks;
	std::vector<Door*> doors;
	float levelPosChange = 0;
	float gravity = 0.0005;
};

