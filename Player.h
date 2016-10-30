#pragma once
#include "Rect.h"
#include "Level.h"
#include "STRUCTS.h"

class Player
{
public:
	Player(Level* cLevel);
	bool groundCheck(Rect* rect);
	bool wallCheck(Rect* rect);
	void movePlayer(float x, float y);
	void update(long elapsed_microseconds);
	void draw(VS_CONSTANT_BUFFER* cbuffer, ID3D11DeviceContext* gcontext,
		ID3D11VertexShader* vs, ID3D11PixelShader* ps,
		ID3D11Buffer* constBuffer, ID3D11SamplerState* sampler,
		ID3D11Buffer* vb, UINT stride, UINT offset
	);

	~Player();

	// Input variables
	bool w_down = false;
	bool a_down = false;
	bool s_down = false;
	bool d_down = false;
	bool w_up = true;

	// Animation variables
	bool animationActive = false;
	float spriteSheetHeight = 192;
	float spriteHeight = 48;
	float spriteSheetWidth = 128;
	float spriteWidth = 32;
	float totalSpriteColumns = (spriteSheetWidth / spriteWidth);
	float totalSpriteRows = (spriteSheetHeight / spriteHeight);
	float adjustedHeight = 1 / totalSpriteRows;
	float adjustedWidth = 1 / totalSpriteColumns;
	float currentFrameRow = 0;
	float currentFrameColumn = 0;
	float scaledSpriteHeight = 48 / 480;
	float scaledSpriteWidth = 32 / 600;

	ID3D11ShaderResourceView* tex;

	// Player attributes 
	bool grounded = false;
	bool againstWall = false;
	bool jumpTime = false;
	int jumpWindow = 0;
	int jumpMax = 500; // The amount of time you can increase jump height
	float width = 0;
	float height = 0;
	float speed = 0.005;
	float jumpHeight = 0.015;
	
	float xPos = 0;
	float yPos = 0;

	float xVelPrev = 0;

	float xVel = 0;
	float yVel = 0;
	Rect* rect = new Rect(adjustedWidth, adjustedHeight, xPos, yPos);
	Level* currentLevel;
};

