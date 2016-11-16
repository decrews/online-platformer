//--------------------------------------------------------------------------------------
// File: Tutorial022.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer VS_CONSTANT_BUFFER : register(b0)
{	
	float currentFrameColumn;
	float adjustedWidth;
	float currentFrameRow;
	float adjustedHeight;
	float xLoc;
	float yLoc;
	float scaleX;
	float scaleY;
	float extra;
	float extraTwo;
	float extraThree;
	float extraFour;
	float extraFive;
	float extraSix;
	float extraSeven;
	float extraEight;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;	
	
	// Animation
	output.Tex = input.Tex;
	output.Tex.x = output.Tex.x += (adjustedWidth * currentFrameColumn);
	output.Tex.y = output.Tex.y += (currentFrameRow * adjustedHeight);
	
	// Movement
	output.Pos = input.Pos;
	output.Pos.x *= scaleX;
	output.Pos.y *= scaleY;

	output.Pos.x = output.Pos.x += xLoc;
	output.Pos.y = output.Pos.y += yLoc;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float2 texture_coordinates = input.Tex;
	float4 color = txDiffuse.Sample(samLinear, texture_coordinates);
	if (extra == 1) {
		color = float4(1, 0, 0, 1);
	}
	return color;
}
