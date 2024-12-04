#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <iostream>

enum class Traffic_color
{
    green = 0,
    orange = 1,
    red = 2
};

Traffic_color operator++(Traffic_color& traffic_color);

class Traffic_light
{
private:
    Traffic_color traffic_color_;

public:
    explicit Traffic_light(const Traffic_color& traffic_color);
    void operator++();
    void set_traffic_color(const Traffic_color& traffic_color);
    const Traffic_color& get_traffic_color() const;
};

std::ostream& operator<<(std::ostream& os, const Traffic_light& traffic_light);

#endif // TRAFFIC_LIGHT_H
