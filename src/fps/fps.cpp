#include <SFML/System/Time.hpp>

#include "fps.hpp"


FpsCounter::FpsCounter() {
    fps_clock.start();
}


float FpsCounter::update() {
    frameCount++;
    float elapsed = fps_clock.getElapsedTime().asSeconds();
    if (elapsed >= 1.0f) {
        fps = frameCount / elapsed;
        frameCount = 0;
        fps_clock.restart();
    }

    return fps;
}