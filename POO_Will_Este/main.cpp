#include "traffic_light.hpp"
#include "car.hpp"
#include "bus.hpp"
#include <cstdlib>
#include <iostream> // std::cout
#include <thread>   // std::thread, std::this_thread::yield
#include <mutex>
#include <vector>
#include <tuple>
#include <random>
#include <vector>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std::chrono_literals;
using namespace std;
using namespace sf;

#ifdef _MSC_VER 
#define _PATH_IMG_ "C:/SFML/img/"
#else
#define _PATH_IMG_ "../img/"
#endif

const std::string path_image(_PATH_IMG_);

static const sf::Color Orange(255, 165, 0);

const auto time_transit = 3s;
const auto time_waiting = 8s;

std::mutex traffic_mutex;
std::condition_variable traffic_cv;
Traffic_color current_master_traffic_light = Traffic_color::red;
Traffic_color current_slave_traffic_light = Traffic_color::red;


vector<Car> cars;
mutex car_mutex;
vector<Bus> buses;
mutex bus_mutex;


// Coordonnées de spawn pour les voitures (globales)
const std::vector<std::tuple<int, int, int>> car_spawn_points = {
    {0, 417, 0},   // Gauche -> Droite
    {377, 0, 90},  // Haut -> Bas
    {735, 377, 180}, // Droite -> Gauche
    {417, 735, 270} // Bas -> Haut
};

// Coordonnées de spawn pour les bus (globales)
const std::vector<std::tuple<int, int, int>> bus_spawn_points = {
    {0, 455, 0},   // Gauche -> Droite
    {340, 0, 90},  // Haut -> Bas
    {765, 340, 180}, // Droite -> Gauche
    {470, 735, 270} // Bas -> Haut
};


void generate_cars(stop_token stop_token) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, car_spawn_points.size() - 1);

    while (!stop_token.stop_requested()) {
        std::this_thread::sleep_for(1s);

        // Sélectionner un point aléatoire depuis car_spawn_points
        auto [x, y, angle] = car_spawn_points[dist(gen)];

        // Ajouter une nouvelle voiture
        {
            std::lock_guard<std::mutex> lock(car_mutex);
            cars.emplace_back(x, y, angle, 0, 1);
        }
    }
}

void generate_buses(stop_token stop_token) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, bus_spawn_points.size() - 1);

    while (!stop_token.stop_requested()) {
        std::this_thread::sleep_for(2s);

        // Sélectionner un point aléatoire depuis bus_spawn_points
        auto [x, y, angle] = bus_spawn_points[dist(gen)];

        // Ajouter un nouveau bus
        {
            std::lock_guard<std::mutex> lock(bus_mutex);
            buses.emplace_back(x, y, angle, 0, 1);
        }
    }
}

const sf::Color& get_SFML_color(const Traffic_light& traffic_light)
{
    switch (traffic_light.get_traffic_color())
    {
    case Traffic_color::green:
        return sf::Color::Green;
    case Traffic_color::red:
        return sf::Color::Red;
    }
    return Orange;
}

void run_traffic_light(Traffic_light& traffic_light_master, Traffic_light& traffic_light_slave, std::stop_token stop_token)
{
    traffic_light_master.set_traffic_color(Traffic_color::green);
    traffic_light_slave.set_traffic_color(Traffic_color::red);
    while (!stop_token.stop_requested())
    {
        {
            std::lock_guard<std::mutex> lock(traffic_mutex);
            current_master_traffic_light = traffic_light_master.get_traffic_color();
            current_slave_traffic_light = traffic_light_slave.get_traffic_color();
        }
        traffic_cv.notify_all();

        std::this_thread::sleep_for(time_waiting);
        if (traffic_light_master.get_traffic_color() == Traffic_color::green)
        {
            ++traffic_light_master;
            ++traffic_light_slave;
        }
        else
        {
            ++traffic_light_slave;
            ++traffic_light_master;
        }
    }
}

void print_traffic_light(Traffic_light& traffic_light_master, Traffic_light& traffic_light_slave, std::stop_token stop_token)
{
    while (!stop_token.stop_requested())
    {
        std::this_thread::sleep_for(1s);
        std::cout << "Taffic light master : " << traffic_light_master << " Taffic light slave : " << traffic_light_slave << std::endl;
    }
}


void run_all_voitures(vector<Car>& cars, stop_token stop_token) {

    while (!stop_token.stop_requested()) {
        std::this_thread::sleep_for(10ms); // Pause pour simuler le délai

        unique_lock<std::mutex> lock(car_mutex); // Verrouiller pour travailler sur les voitures
        int indice = 0;

        for (auto& car : cars) {
            bool is_blocked = false;

            {
                // Synchronisation avec les feux (indépendant pour chaque voiture)
                std::unique_lock<std::mutex> traffic_lock(traffic_mutex);

                // Vérification des feux pour bloquer seulement les voitures concernées
                if ((car.getY() > 600 && car.getAngle() == 270 && current_slave_traffic_light == Traffic_color::red) || // Bas vers Haut
                    (car.getY() < 200 && car.getAngle() == 90 && current_slave_traffic_light == Traffic_color::red) ||  // Haut vers Bas
                    (car.getX() > 600 && car.getAngle() == 180 && current_master_traffic_light == Traffic_color::red) || // Droite vers Gauche
                    (car.getX() < 200 && car.getAngle() == 0 && current_master_traffic_light == Traffic_color::red)) {    // Gauche vers Droite
                    is_blocked = true; // Bloquer cette voiture si les conditions sont remplies
                }
            }

            if (is_blocked) {
                indice++;
                continue; // Skip le reste du traitement pour cette voiture
            }

            // Déplacer la voiture
            car.move();

            // Augmenter la vitesse si nécessaire
            if (car.getPatern() == 1) {
                car.speedUp();
            }

            // Supprimer les voitures hors écran
            if (car.getX() > 800 || car.getY() > 800 || car.getX() < 0 || car.getY() < 0) {
                cars.erase(cars.begin() + indice);
                indice--;
            }
            indice++;
        }
    }
}

void run_all_buses(vector<Bus>& buses, stop_token stop_token) {
    while (!stop_token.stop_requested()) {
        std::this_thread::sleep_for(10ms); // Pause pour simuler le délai

        unique_lock<std::mutex> lock(bus_mutex); // Verrouiller pour travailler sur les bus
        int indice = 0;

        for (auto& bus : buses) {
            bool is_blocked = false;

            {
                // Synchronisation avec les feux
                std::unique_lock<std::mutex> traffic_lock(traffic_mutex);

                if ((bus.getY() > 600 && bus.getAngle() == 270 && current_slave_traffic_light == Traffic_color::red) || // Bas vers Haut
                    (bus.getY() < 200 && bus.getAngle() == 90 && current_slave_traffic_light == Traffic_color::red) ||  // Haut vers Bas
                    (bus.getX() > 600 && bus.getAngle() == 180 && current_master_traffic_light == Traffic_color::red) || // Droite vers Gauche
                    (bus.getX() < 200 && bus.getAngle() == 0 && current_master_traffic_light == Traffic_color::red)) {    // Gauche vers Droite
                    is_blocked = true;
                }
            }

            if (is_blocked) {
                indice++;
                continue;
            }

            bus.move();

            if (bus.getPatern() == 1) {
                bus.speedUp();
            }

            if (bus.getX() > 800 || bus.getY() > 800 || bus.getX() < 0 || bus.getY() < 0) {
                buses.erase(buses.begin() + indice);
                indice--;
            }
            indice++;
        }
    }
}



int main()
{
    Texture carImage;
    Sprite carSprite;
    Texture backgroundImage;
    Sprite backgroundSprite;

    if (!carImage.loadFromFile(path_image + "Voiture.png")) {
        cerr << "Erreur pendant le chargement des images" << endl;
        return EXIT_FAILURE; // On ferme le programme
    }
    if (!backgroundImage.loadFromFile(path_image + "Route1.PNG")) {
        cerr << "Erreur pendant le chargement des images" << endl;
        return EXIT_FAILURE; // On ferme le programme
    }

    cars.push_back(Car(417, 735, 270, 0, 1));
    cars.push_back(Car(377, 0, 90, 0, 1));
    cars.push_back(Car(0, 417, 0, 0, 1));
    cars.push_back(Car(735, 377, 180, 0, 1));

    buses.push_back(Bus(470, 735, 270, 0, 1));
    buses.push_back(Bus(340, 0, 90, 0, 1));
    buses.push_back(Bus(0, 455, 0, 0, 1));
    buses.push_back(Bus(765, 340, 180, 0, 1));

    carSprite.setTexture(carImage);
    carSprite.setScale(sf::Vector2f(0.1, 0.1));
    backgroundSprite.setTexture(backgroundImage);
    // Calculer le facteur d'échelle pour s'adapter à une fenêtre 800x800
    float scaleX = 800.f / backgroundImage.getSize().x;
    float scaleY = 800.f / backgroundImage.getSize().y;

    // Appliquer l'échelle au sprite
    backgroundSprite.setScale(scaleX, scaleY);

    sf::RenderWindow window(sf::VideoMode(800, 800), "My window");

    float l1 = 240, l2 = 580, size = 800, radius = 20;
    /*f::Vertex line1[] = {sf::Vertex(sf::Vector2f(0, l1)), sf::Vertex(sf::Vector2f(size, l1))};
    sf::Vertex line2[] = { sf::Vertex(sf::Vector2f(0, l2)), sf::Vertex(sf::Vector2f(size, l2)) };
    sf::Vertex line3[] = { sf::Vertex(sf::Vector2f(l1, 0)), sf::Vertex(sf::Vector2f(l1, size)) };
    sf::Vertex line4[] = { sf::Vertex(sf::Vector2f(l2, 0)), sf::Vertex(sf::Vector2f(l2, size)) };*/

    sf::CircleShape circle1(radius);
    circle1.setFillColor(sf::Color::Blue);
    circle1.setOrigin(circle1.getRadius() / 2, circle1.getRadius() / 2);
    circle1.setPosition(l2 - 2*radius, l2 + radius / 2);
    sf::CircleShape circle2(radius);
    circle2.setFillColor(sf::Color::Green);
    circle2.setOrigin(circle2.getRadius() / 2, circle2.getRadius() / 2);
    circle2.setPosition(l2 + radius / 2, l1 );
    sf::CircleShape circle3(radius);
    circle3.setFillColor(sf::Color::Blue);
    circle3.setOrigin(circle3.getRadius() / 2, circle3.getRadius() / 2);
    circle3.setPosition(l1 , l1 - 2*radius);
    sf::CircleShape circle4(radius);
    circle4.setFillColor(sf::Color::Green);
    circle4.setOrigin(circle4.getRadius() / 2, circle4.getRadius() / 2);
    circle4.setPosition(l1 - 2*radius, l2 - 2*radius);


    std::stop_source stopping;
    Traffic_light traffic_light_master{ Traffic_color::red };
    Traffic_light traffic_light_slave{ Traffic_color::red };
    std::jthread thread_traffic_light_master(run_traffic_light,
        std::ref(traffic_light_master), std::ref(traffic_light_slave), stopping.get_token());

    std::jthread write_traffic_light(print_traffic_light,
        std::ref(traffic_light_master), std::ref(traffic_light_slave), stopping.get_token());

    jthread voiture(run_all_voitures, ref(cars), stopping.get_token());

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                stopping.request_stop();
                window.close();
                return 0;
            }
        }

        
        window.clear(sf::Color::Black);

        window.draw(backgroundSprite);

        /*window.draw(line1, 2, sf::Lines);
        window.draw(line2, 2, sf::Lines);
        window.draw(line3, 2, sf::Lines);
        window.draw(line4, 2, sf::Lines);*/
        circle1.setFillColor(get_SFML_color(traffic_light_slave));
        circle2.setFillColor(get_SFML_color(traffic_light_master));
        circle3.setFillColor(get_SFML_color(traffic_light_slave));
        circle4.setFillColor(get_SFML_color(traffic_light_master));
        window.draw(circle1);
        window.draw(circle2);
        window.draw(circle3);
        window.draw(circle4);

        for (const auto& car : cars)
        {
            carSprite.setPosition(car.getX(), car.getY());
            carSprite.setRotation(car.getAngle());
            window.draw(carSprite);
        }


        window.display();
    }

    return 0;
}
