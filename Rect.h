#pragma once
class Rect
{
public:
	Rect(float width_in, float height_in, float x_in, float y_in);
	~Rect();
	bool collides(Rect* rect);

	float width;
	float height;
	float x;
	float y;
};

