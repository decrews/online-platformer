#include "Pawn.h"


Pawn::Pawn(float x, float y, ID3D11ShaderResourceView * t, int id) {
	this->id = id;
	xPos = x;
	yPos = y;
	tex = t;
	rect = new Rect(width, height, x, y);
}



Pawn::~Pawn()
{
}



void Pawn::draw(VS_CONSTANT_BUFFER* cbuffer, ID3D11DeviceContext* gcontext,
	ID3D11VertexShader* vs, ID3D11PixelShader* ps,
	ID3D11Buffer* constBuffer, ID3D11SamplerState* sampler,
	ID3D11Buffer* vb, UINT stride, UINT offset) {

	// Constant buffer data for background
	cbuffer->currentFrameColumn = 0;
	cbuffer->adjustedWidth = 0;
	cbuffer->currentFrameRow = 0;
	cbuffer->adjustedHeight = 0;

	// Constant buffer variables for movement
	cbuffer->x = rect->x;
	cbuffer->y = rect->y;

	// Scale
	cbuffer->scaleX = 0.07;
	cbuffer->scaleY = 0.07;

	// Setting constants, pixel, and vertex shader.
	gcontext->UpdateSubresource(constBuffer, 0, 0, cbuffer, 0, 0);
	gcontext->VSSetConstantBuffers(0, 1, &constBuffer);
	gcontext->PSSetConstantBuffers(0, 1, &constBuffer);

	// Render the background
	gcontext->PSSetSamplers(0, 1, &sampler);
	gcontext->PSSetShaderResources(0, 1, &tex);
	gcontext->VSSetShader(vs, NULL, 0);
	gcontext->PSSetShader(ps, NULL, 0);

	// Set vertex buffer and Draw
	gcontext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	gcontext->Draw(6, 0);
}



void Pawn::update(float x, float y, long elapsed_microseconds) {
	xPos = x;
	yPos = y;
	rect->x = x;
	rect->y = y;
}
