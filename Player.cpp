#include "Player.h"


Player::Player(Level* cLevel) {
	currentLevel = cLevel;
}



//bool Player::groundCheck(Rect* rect) {
//	if (this->rect->collides(rect)) {
//		if (this->rect->y > rect->y + (rect->height / 2)
//			&& this->rect->x - (this->rect->width / 500) > rect->x - (rect->width / 2)
//			&& this->rect->x + (this->rect->width / 500) < rect->x + (rect->width / 2)) {
//			OutputDebugString(L"Testing\n");
//			return true;
//		}
//		else {
//			return false;
//		}
//	}
//	return false;
//}

bool Player::groundCheck(Rect* rect) {
	if (this->rect->collides(rect)) {
		float diff = sqrt((this->rect->y - (rect->y + rect->height/ 2))*(this->rect->y - (rect->y + rect->height / 2)));
		if (diff < rect->height) {
			return true;
		}
		else {
			return false;
		}
		return false;
	}
}

bool Player::wallCheck(Rect* rect) {
	if (this->rect->collides(rect)) {
		float diff = sqrt((xPos - (rect->x + rect->width / 2))*(xPos - (rect->x + rect->width / 2)));
		if (diff < rect->width) {
			return true;
		}
		else {
			return false;
		}
		return false;
	}
}

void Player::movePlayer(float x, float y) {
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
		if (grounded && jumpRelease) {
			yVel = jumpHeight;
		}
	}
	if (grounded == false) {
		yVel -= currentLevel->gravity * dt;
	}
	

	// Move Player
	yPos += yVel  * dt;
	rect->y += yVel  * dt;
	currentLevel->levelPosChange = xVel  * dt;

	// Update level blocks and check collision
	grounded = false;
	againstWall = false;
	hitGround = false;

	for (Platform* block : currentLevel->blocks) {

		// Checks to see if there is a collision with the ground.
		if (groundCheck(block->rect)) {
			hitGround = true;
			grounded = true;

			// If the block is a falling block (type 1), set the falling flag.
			if (block->type == 1) {
				block->falling = true;
			}
		}

		// If ground collision did happen (hitGround), then undo the move (-yVel)
		if (hitGround == true) {
			yPos -= yVel  * dt;
			rect->y -= yVel  * dt;
			yVel = 0;
		}

		// Checks every block to see if the player is against it.
		// If so, flag it so that the wall updates are handled correctly.
		if (wallCheck(block->rect)) {
			againstWall = true;
			OutputDebugStringW(L"Wall collision triggered.\n");
		}
	}

	// Smooth out walking on falling blocks.
	if (hitGround == true) {
		yPos += 0.00001;
		rect->y += 0.00001;
	}

	// If the player is not against the wall, move the platforms
	// and save this "move" in xVelPrev in case we have to move 
	// it back next frame.
	if (againstWall == false) {
		for (Platform* block : currentLevel->blocks) {
			currentLevel->levelPosChange = xVel  * dt;
		}

		xVelPrev = xVel  * dt;
	}

	// Wall collision happened this frame!  Undo to the previous move
	// by subtracting the previous velocity to restore the previous platform
	// position.
	else {
		currentLevel->levelPosChange = -xVelPrev;
	}

	if (yPos <= -1.5) {
		PostQuitMessage(0);
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
