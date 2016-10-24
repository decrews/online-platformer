#pragma once
#include <vector>
#include <D3D11.h>
#include "Platform.h"
#include "STRUCTS.h"

class Level
{
public:
	Level();
	~Level();
	void Draw(VS_CONSTANT_BUFFER* cbuffer, ID3D11DeviceContext* gcontext,
		ID3D11VertexShader* vs, ID3D11PixelShader* ps,
		ID3D11Buffer* constBuffer, ID3D11SamplerState* sampler,
		ID3D11Buffer* vb, UINT stride, UINT offset
		);
	void Update(long elapsed_microseconds);

	ID3D11ShaderResourceView* bgTex; // Background Texture

	std::vector<Platform*> blocks;
	float levelPos = 0;
	float gravity = 0.0005;
};

