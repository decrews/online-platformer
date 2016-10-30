#include "Player.h"


Player::Player(Level* cLevel) {
	currentLevel = cLevel;
}



bool Player::groundCheck(Rect* rect) {
	if (this->rect->collides(rect)) {
		if (yPos - (height / 2) < rect->y + (rect->height / 2)) {
			return false;
		}
		else {
			return true;
		}
	}
	return false;
}



void Player::update(long elapsed_microseconds) {
	// Time since last frame, always multiply by this
	// when moving the player
	float dt = elapsed_microseconds / 10000.0;

	if (a_down) {
		xVel = -speed;
		currentFrameRow = 1;
		animationActive = true;
	}
	else if (d_down) {
		xVel = speed;
		currentFrameRow = 2;
		animationActive = true;
	}
	else {
		currentFrameColumn = 2;
		animationActive = false;
		xVel = 0;
	}

	if (w_down) {
		/*
		if (jumpTime) {
			yVel = jumpHeight;
		}
		*/
		if (grounded) {
			yVel = jumpHeight;
			//jumpTime = true;
		}
	}

	if (grounded == false) {
		yVel -= currentLevel->gravity * dt;
	}
	/*
	if (jumpTime == true) {
		if (jumpWindow < jumpMax) {
			yVel -= currentLevel->gravity * dt;
			jumpWindow++;
		}
		else {
			jumpWindow = 0;
			jumpTime = false;
		}
	}
	*/
	// Move Player
	yPos += yVel  * dt;
	currentLevel->levelPos -= xVel  * dt;
	rect->y += yVel  * dt;

	// Update level blocks and check collision
	grounded = false;
	for (Platform* block : currentLevel->blocks) {
		block->xPos -= xVel  * dt;
		block->rect->x -= xVel  * dt;

		if (groundCheck(block->rect)) {
			grounded = true;
			yVel = 0;
			yPos = block->rect->y + height/2 + block->rect->height/1.6;
			rect->y = block->rect->y + height/2 + block->rect->height/1.6;
			OutputDebugStringW(L"Collision with Platform happened.\n");
		}
	}

}



void Player::draw(VS_CONSTANT_BUFFER* cbuffer, ID3D11DeviceContext* gcontext,
	ID3D11VertexShader* vs, ID3D11PixelShader* ps,
	ID3D11Buffer* constBuffer, ID3D11SamplerState* sampler,
	ID3D11Buffer* vb, UINT stride, UINT offset) {

	// Constant buffer data for animation
	cbuffer->currentFrameColumn = currentFrameColumn;
	cbuffer->adjustedWidth = adjustedWidth;
	cbuffer->currentFrameRow = currentFrameRow;
	cbuffer->adjustedHeight = adjustedHeight;

	// Constant buffer data for movement
	cbuffer->x = xPos;
	cbuffer->y = yPos;

	// Scale
	cbuffer->scale = 1;

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



Player::~Player()
{
}
