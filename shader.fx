//--------------------------------------------------------------------------------------
// File: Tutorial022.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer VS_CONSTANT_BUFFER : register(b0)
{	
	float cb_a;
	float cb_b;
	float cb_c;
	float cb_d;
	float cb_e;
	float cb_f;
	float cb_g;
	float cb_h;
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
	output.Tex.x = output.Tex.x += (cb_b * cb_a);
	output.Tex.y = output.Tex.y += (cb_c * cb_d);
	
	// Movement
	output.Pos = input.Pos;
	output.Pos.x *= cb_g;
	output.Pos.y *= cb_g;

	output.Pos.x = output.Pos.x += cb_e;
	output.Pos.y = output.Pos.y += cb_f;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float2 texture_coordinates = input.Tex;
	float4 color = txDiffuse.Sample(samLinear, texture_coordinates);
	if (cb_h == 1) {
		color = float4(1, 0, 0, 1);
	}
	return color;
}
