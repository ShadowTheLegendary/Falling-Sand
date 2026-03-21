#include <SFML/System/Clock.hpp>

class FpsCounter {
public:
    FpsCounter();


    float update();


private:
    sf::Clock fps_clock;
    int frameCount = 0;
    float fps = 0.0f;

};