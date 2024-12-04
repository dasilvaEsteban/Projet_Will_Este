#include "traffic_light.hpp"
#include<thread>

using namespace std::chrono_literals;
const auto time_transit = 3s;
const auto time_waiting = 8s;


Traffic_color operator++(Traffic_color& traffic_color)
{
    switch (traffic_color)
    {
    case Traffic_color::red:
        traffic_color = Traffic_color::green;
        break;

    case Traffic_color::green:
        std::this_thread::sleep_for(time_transit);
        traffic_color = Traffic_color::orange;
        std::this_thread::sleep_for(time_transit);
        traffic_color = Traffic_color::red;
    }
    return traffic_color;
}

// Constructeur de Traffic_light
Traffic_light::Traffic_light(const Traffic_color& traffic_color)
    : traffic_color_{ traffic_color }
{
}

// Méthode pour incrémenter l'état du feu
void Traffic_light::operator++()
{
    ++traffic_color_;
}

// Setter pour la couleur du feu
void Traffic_light::set_traffic_color(const Traffic_color& traffic_color)
{
    traffic_color_ = traffic_color;
}

// Getter pour la couleur du feu
const Traffic_color& Traffic_light::get_traffic_color() const
{
    return traffic_color_;
}

std::ostream& operator<<(std::ostream& os, const Traffic_light& traffic_light)
{
    std::string ptr;
    switch (traffic_light.get_traffic_color())
    {
    case Traffic_color::red:
        ptr = "Red";
        break;
    case Traffic_color::green:
        ptr = "Green";
        break;
    case Traffic_color::orange:
        ptr = "Orange";
    }
    os << ptr;
    return os;
}