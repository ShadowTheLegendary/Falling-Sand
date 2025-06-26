#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <Windows.h>

#include "particle_simulation.h"
#include "sidebar.h"

int main() {
    sf::RenderWindow window(sf::VideoMode({ 650, 550 }), "Particle Sim");
    window.setFramerateLimit(30);

    ParticleSimulation sim;

    Sidebar sidebar({"sand", "rock", "water"}, {sf::Color(255, 255, 0), sf::Color(128, 128, 128), sf::Color(0, 128, 255)});
    sidebar.add_option_img("heat", "heat_element.png");
    sidebar.add_option_img("cool", "cool_element.png");
    sidebar.add_option_img("none", "none_element.png");

    bool paused = false;
    int brush_size = 5;
    bool draw_mode = false;

    int mouse_x = 0;
    int mouse_y = 0;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                sidebar.handle_click({ mouseButtonPressed->position.x , mouseButtonPressed->position.y }, window);
            }

            if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheelScrolled->wheel == sf::Mouse::Wheel::Vertical) {
                    if (mouseWheelScrolled->delta > 0)
                        brush_size = std::min(brush_size + 1, 50);
                    else if (mouseWheelScrolled->delta < 0)
                        brush_size = std::max(brush_size - 1, 1);
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            break;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            paused = !paused;
            Sleep(500);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F) && paused) {
            sim.update();
            Sleep(500);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
            draw_mode = false;
            Sleep(500);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
            draw_mode = true;
            Sleep(500);
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) { 
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sim.brush(brush_size, mousePos.x, mousePos.y, sidebar.get_selected_of_index());
        }

        if (!paused) {
            sim.update();
        }

        system("cls");
        window.clear();

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        mouse_x = mousePos.x;
        mouse_y = mousePos.y;
        
        sim.draw_sfml(window, draw_mode);
        sim.draw_brush_outline_sfml(window, brush_size, mouse_x, mouse_y);
        sidebar.draw_sfml(window);

        window.display();
    }
}