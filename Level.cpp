#include "Level.h"


Level::Level() {

}



void Level::draw(VS_CONSTANT_BUFFER* cbuffer, ID3D11DeviceContext* gcontext,
	ID3D11VertexShader* vs, ID3D11PixelShader* ps,
	ID3D11Buffer* constBuffer, ID3D11SamplerState* sampler,
	ID3D11Buffer* vb, UINT stride, UINT offset) {

	// Constant buffer data for background
	cbuffer->currentFrameColumn = 0;
	cbuffer->adjustedWidth = 0;
	cbuffer->currentFrameRow = 0;
	cbuffer->adjustedHeight = 0;

	// Constant buffer variables for movement
	cbuffer->x = 0.2;
	cbuffer->y = 0.2;

	// Scale
	cbuffer->scale = 2;

	// Setting constants, pixel, and vertex shader.
	gcontext->UpdateSubresource(constBuffer, 0, 0, cbuffer, 0, 0);
	gcontext->VSSetConstantBuffers(0, 1, &constBuffer);
	gcontext->PSSetConstantBuffers(0, 1, &constBuffer);

	// Render the background
	gcontext->PSSetSamplers(0, 1, &sampler);
	gcontext->PSSetShaderResources(0, 1, &bgTex);
	gcontext->VSSetShader(vs, NULL, 0);
	gcontext->PSSetShader(ps, NULL, 0);

	// Set vertex buffer and Draw
	gcontext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	gcontext->Draw(6, 0);
	

	for (auto curPlatform : blocks) {
		// Constant buffer data for background
		cbuffer->currentFrameColumn = 0;
		cbuffer->adjustedWidth = 0;
		cbuffer->currentFrameRow = 0;
		cbuffer->adjustedHeight = 0;

		// Constant buffer variables for movement
		cbuffer->x = curPlatform->xPos;
		cbuffer->y = curPlatform->yPos;

		// Scale
		cbuffer->scale = curPlatform->scale;

		// Setting constants, pixel, and vertex shader.
		gcontext->UpdateSubresource(constBuffer, 0, 0, cbuffer, 0, 0);
		gcontext->VSSetConstantBuffers(0, 1, &constBuffer);
		gcontext->PSSetConstantBuffers(0, 1, &constBuffer);

		// Render the platforms
		gcontext->PSSetSamplers(0, 1, &sampler);
		gcontext->PSSetShaderResources(0, 1, &curPlatform->tex);
		gcontext->VSSetShader(vs, NULL, 0);
		gcontext->PSSetShader(ps, NULL, 0);

		// Set vertex buffer and Draw
		gcontext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		gcontext->Draw(6, 0);
	}
}



void Level::update(long elapsed_microseconds) {
	// Put level activity here.  Moving platforms, enemies, etc.
	for (auto block : blocks) {
		block->xPos -= levelPosChange;
		block->rect->x -= levelPosChange;

		if (block->falling == true) {
			block->yPos -= 0.0001;
			block->rect->y -= 0.0001;
		}
	}
	
}



Level::~Level() {

}
