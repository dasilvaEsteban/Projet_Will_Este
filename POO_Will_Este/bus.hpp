#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

class Bus {
private:
	float x, y;
	float angle;
	float speed;
	float patern;
public:
	Bus(const float x, const float y, const float angle, const float speed, const float patern);

	float getX() const;
	float getY() const;
	float getAngle() const;
	float getSpeed() const;
	float getPatern() const;

	void move();
	void turnLeft();
	void turnRight();
	void speedUp();
	void speedDown();
};
