#include "Player.h"
#include <string>


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
		float diff = this->rect->y - (rect->y + rect->height / 2);
		if (diff < rect->height
			&& this->rect->x - (this->rect->width / 75) > rect->x - (rect->width / 2)
			&& this->rect->x + (this->rect->width / 75) < rect->x + (rect->width / 2)) {
			return true;
		}
		else {
			return false;
		}
		return false;
	}
}


bool Player::headCheck(Rect* rect) {
	float buffer = rect->width * 0.46;

	if (this->rect->collides(rect)) {
		float diff = this->rect->y + (rect->y - rect->height / 2);
		if (diff - 0.7 < rect->height) {
			if (this->rect->x >= rect->x - buffer
				&& this->rect->x <= rect->x + buffer) {
				return true;
			}
		}
		else {
			return false;
		}
		return false;
	}
}


bool Player::wallCheck(Rect* rect) {
	if (this->rect->collides(rect)) {
		float diff = this->rect->x - (rect->x + rect->width / 2);
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
	rect->y += yVel  * dt;
	currentLevel->levelPosChange = xVel  * dt;

	// Update level blocks and check collision
	grounded = false;
	againstWall = false;
	hitGround = false;

	std::wstring test = std::to_wstring(dt);
	test += L"\n";
	OutputDebugString(test.c_str());

	for (Platform* block : currentLevel->blocks) {
		if (block->rect->x > -0.2 && block->rect->x < 0.2) {
			// Checks to see if there is a collision with the ground.
			if (groundCheck(block->rect)) {

				//OutputDebugStringW(L"Ground collision triggered.\n");
				hitGround = true;
				grounded = true;

				// If the block is a falling block (type 1), set the falling flag.
				if (block->type == 1) {
					block->falling = true;
				}

				if (block->type == 2 && w_down == true)
				{
					this->yVel = -dt*20;
				}


				//testing moving platform
				if (block->type == 3)
				{
					onBlock = true;
					if (onBlock == true) {
						block->rect->x += 0.01;
						block->xPos += 0.01;
					}
				}
			}

			// If ground collision did happen (hitGround), then undo the move (-yVel)
			if (hitGround == true) {
				rect->y -= yVel  * dt;
				yVel = 0;
			}

			// Checks every block to see if the player is against it.
			// If so, flag it so that the wall updates are handled correctly.
			if (wallCheck(block->rect) && !block->falling) {
				againstWall = true;
				//OutputDebugStringW(L"Wall collision triggered.\n");

				//push block
				if (block->type == 2)
				{
					block->rect->x = block->rect->x + xVel;
					block->xPos = block->xPos + this->xVel;
				}
			}

			

			if (headCheck(block->rect) && !block->falling) {
				//OutputDebugStringW(L"Head check triggered.\n");
				// Set the location of the player to the bottom of the block
				rect->y = (block->rect->y - block->rect->height / 2) - 0.03;
				// Reverse the velocity to "bounce" the player back down
				yVel = -yVel;

				grounded = false;
				hitGround = false;
			}
		}
	}

	//check point system
	for (Door* block : currentLevel->doors)
	{
		if (wallCheck(block->rect))
		{
			this->initialX = block->xPos;
			this->initialY = block->yPos + 0.1;

			currentLevel->offsetX = currentLevel->levelPosition;
			OutputDebugStringW(L"Checkpoint reached.\n");
		}
	}

	// Smooth out walking on falling blocks.
	if (hitGround == true) {
		rect->y += 0.004;
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

	if (this->rect->y <= -1.5) {
		// Reset player's position on the screen
		this->rect->x = this->initialX;
		this->rect->y = this->initialY;

		// Move the player back to his original position (-levelPosition) and then add checkpoint (+offset):
		currentLevel->levelPosChange = -currentLevel->levelPosition + currentLevel->offsetX;
		//alive = true;
	}

	//hit spikes
	for (Spike* block : currentLevel->spikes)
	{
		if (groundCheck(block->rect))
		{
			//alive = false;
			// Reset player's position on the screen
			this->rect->y = this->initialY;
			yVel = 0;
			xVel = 0;

			// Move the player back to his original position (-levelPosition) and then add checkpoint (+offset):
			currentLevel->levelPosChange = -currentLevel->levelPosition + currentLevel->offsetX;
			OutputDebugStringW(L"Ouch!.\n");
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
	cbuffer->x = this->rect->x;
	cbuffer->y = this->rect->y;

	// Scale
	cbuffer->scaleX = 1;
	cbuffer->scaleY = 1;

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
