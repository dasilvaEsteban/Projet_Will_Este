#include "bus.hpp"

Bus::Bus(float x, float y, float angle, float speed, float patern)
    : x(x), y(y), angle(angle), speed(speed), patern(patern)
{}

float
Bus::getX() const
{
    return x;
}

float
Bus::getY() const
{
    return y;
}

float
Bus::getAngle() const
{
    return angle;
}

float
Bus::getSpeed() const
{
    return speed;
}

float
Bus::getPatern() const
{
    return patern;
}


void
Bus::move()
{
    x += static_cast<float>(cos(angle * M_PI / 180.0) * speed);
    y += static_cast<float>(sin(angle * M_PI / 180.0) * speed);
}

void
Bus::turnLeft()
{
    angle -= 2.f;
}

void
Bus::turnRight()
{
    angle += 2.f;
}

void
Bus::speedUp()
{
    if (speed < 3.f)
        speed += 0.1f;
}

void
Bus::speedDown()
{
    if (speed > 0.f)
        speed -= 0.1f;
    else
        speed = 0;
}