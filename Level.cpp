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
	cbuffer->x = bgPos;
	cbuffer->y = 0;

	// Scale
	cbuffer->scaleX = 2;
	cbuffer->scaleY = 2;

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
		if (curPlatform->rect->x > -1.2 && curPlatform->rect->x < 1.2) {
			// Constant buffer data for background
			cbuffer->currentFrameColumn = 0;
			cbuffer->adjustedWidth = 0;
			cbuffer->currentFrameRow = 0;
			cbuffer->adjustedHeight = 0;

			// Constant buffer variables for movement
			cbuffer->x = curPlatform->xPos;
			cbuffer->y = curPlatform->yPos;

			// Scale
			cbuffer->scaleX = curPlatform->scale;
			cbuffer->scaleY = curPlatform->scale;

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

			for (auto curDoor : doors) {
				// Constant buffer data for background
				cbuffer->currentFrameColumn = 0;
				cbuffer->adjustedWidth = 0;
				cbuffer->currentFrameRow = 0;
				cbuffer->adjustedHeight = 0;

				// Constant buffer variables for movement
				cbuffer->x = curDoor->xPos;
				cbuffer->y = curDoor->yPos;

				// Scale
				cbuffer->scaleX = curDoor->scale;
				cbuffer->scaleY = curDoor->scale;

				// Setting constants, pixel, and vertex shader.
				gcontext->UpdateSubresource(constBuffer, 0, 0, cbuffer, 0, 0);
				gcontext->VSSetConstantBuffers(0, 1, &constBuffer);
				gcontext->PSSetConstantBuffers(0, 1, &constBuffer);

				// Render the platforms
				gcontext->PSSetSamplers(0, 1, &sampler);
				gcontext->PSSetShaderResources(0, 1, &curDoor->tex);
				gcontext->VSSetShader(vs, NULL, 0);
				gcontext->PSSetShader(ps, NULL, 0);

				// Set vertex buffer and Draw
				gcontext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
				gcontext->Draw(6, 0);
			}
				for (auto curSpike : spikes) {
					// Constant buffer data for background
					cbuffer->currentFrameColumn = 0;
					cbuffer->adjustedWidth = 0;
					cbuffer->currentFrameRow = 0;
					cbuffer->adjustedHeight = 0;

					// Constant buffer variables for movement
					cbuffer->x = curSpike->xPos;
					cbuffer->y = curSpike->yPos;

					// Scale
					cbuffer->scaleX = curSpike->scale;
					cbuffer->scaleY = curSpike->scale;

					// Setting constants, pixel, and vertex shader.
					gcontext->UpdateSubresource(constBuffer, 0, 0, cbuffer, 0, 0);
					gcontext->VSSetConstantBuffers(0, 1, &constBuffer);
					gcontext->PSSetConstantBuffers(0, 1, &constBuffer);

					// Render the platforms
					gcontext->PSSetSamplers(0, 1, &sampler);
					gcontext->PSSetShaderResources(0, 1, &curSpike->tex);
					gcontext->VSSetShader(vs, NULL, 0);
					gcontext->PSSetShader(ps, NULL, 0);

					// Set vertex buffer and Draw
					gcontext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
					gcontext->Draw(6, 0);
				}
	
}



void Level::update(long elapsed_microseconds) {
	// Put level activity here.  Moving platforms, enemies, etc.
	float dt = elapsed_microseconds / 10000.0;

	// Update the blocks with every levelPosChange
	for (auto block : blocks) {
		block->xPos -= levelPosChange;
		block->rect->x -= levelPosChange;

		if (block->falling == true) {
			block->yVel += block->fallingSpeed * dt;

			block->yPos -= block->yVel * dt;
			block->rect->y -= block->yVel * dt;
		}

		if (block->yPos < -10) {
			block->yPos = block->originY;
			block->rect->y = block->originY;
			block->falling = false;
			block->yVel = 0;
		}
	}

	for (auto door : doors) {
		door->xPos -= levelPosChange;
		door->rect->x -= levelPosChange;
	}

	for (auto Spike : spikes) {
		Spike->xPos -= levelPosChange;
		Spike->rect->x -= levelPosChange;

	}

	// Update the background with every levelPosChange;
	bgPos -= levelPosChange / 10;
	
	// levelPosition - Total offset from the player's original position.
	// levelPosChange - The amount of change from this frame.
	levelPosition += levelPosChange;

}



Level::~Level() {

}
