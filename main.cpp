#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Windows.h>
#include "particle_simulation.h"
#include "sidebar.h"
#include <string>
#include <sstream>

int main() {
    int playbackspeed = 15;

    sf::RenderWindow window(sf::VideoMode({ 650, 650 }), "Particle Sim");
    window.setFramerateLimit(playbackspeed);

    ParticleSimulation sim(50, 50);

    Sidebar sidebar({ "sand", "rock", "water" }, { sf::Color(255, 255, 0), sf::Color(128, 128, 128), sf::Color(0, 128, 255) });
    sidebar.add_option_img("heat", "heat_element.png");
    sidebar.add_option_img("cool", "cool_element.png");
    sidebar.add_option_img("none", "none_element.png");

    Sidebar display_sidebar({ "standard" }, { sf::Color(255, 255, 0) }, "bottom");
	display_sidebar.add_option_img("temperature", "temp_display_element.png");

    bool paused = false;
    int brush_size = 5;
    bool draw_mode = false;

    sf::Vector2i mousePos;
    int mouse_x = 0;
    int mouse_y = 0;

    sf::Font arial("ARIAL.TTF");
    sf::Text info_top(arial, "", 15U);
    info_top.setPosition(sf::Vector2f(15, 556));
    std::string paused_info = "\n";
    std::string display_mode_info = "standard";

    // Framerate tracking variables
    sf::Clock fpsClock;
    int frameCount = 0;
    float fps = 0.0f;

    while (window.isOpen()) {
        // Framerate tracking: count frames and update FPS every second
        frameCount++;
        float elapsed = fpsClock.getElapsedTime().asSeconds();
        if (elapsed >= 1.0f) {
            fps = frameCount / elapsed;
            frameCount = 0;
            fpsClock.restart();
        }

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                sidebar.handle_click({ mouseButtonPressed->position.x , mouseButtonPressed->position.y }, window);
                display_sidebar.handle_click({ mouseButtonPressed->position.x , mouseButtonPressed->position.y }, window);
            }

            if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheelScrolled->wheel == sf::Mouse::Wheel::Vertical) {
                    if (mouseWheelScrolled->delta > 0)
                        brush_size = std::min(brush_size + 1, 50);
                    else if (mouseWheelScrolled->delta < 0)
                        brush_size = std::max(brush_size - 1, 1);
                }
            }

            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                if (textEntered->unicode >= 49 and textEntered->unicode <= 57) { // 1 - 9
                    int playbackspeed_copy = playbackspeed;
                    playbackspeed = 15 * (textEntered->unicode - 48);
                    if (playbackspeed != playbackspeed_copy) {
                        window.setFramerateLimit(playbackspeed);
                    }
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            break;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            paused = !paused;
            if (paused) {
                paused_info = "paused\n";
            }
            else {
                paused_info = "\n";
            }

            Sleep(500);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F) && paused) {
            sim.update();
            Sleep(500);
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
                sim.brush(brush_size, mousePos.x, mousePos.y, sidebar.get_selected_of_index(), 5);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                sim.brush(brush_size, mousePos.x, mousePos.y, sidebar.get_selected_of_index(), 0.5);
            }
            else {
                sim.brush(brush_size, mousePos.x, mousePos.y, sidebar.get_selected_of_index());
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sim.brush(brush_size, mousePos.x, mousePos.y, "none");
        }

        if (!paused) {
            sim.update();
        }

        if (display_sidebar.get_selected_of_index() == "temperature") {
            draw_mode = true;
            display_mode_info = "temperature";
		}
        else {
            draw_mode = false;
            display_mode_info = "standard";
        }

        if (sim.discovered_particles.find("glass") != sim.discovered_particles.end() and !sidebar.is_discovered("glass")) {
            sidebar.discover_element("glass", sf::Color(207, 255, 245));
        }

        system("cls");
        window.clear();

        mousePos = sf::Mouse::getPosition(window);
        mouse_x = mousePos.x;
        mouse_y = mousePos.y;

        // Display FPS in the info text
        std::ostringstream oss;
        oss << paused_info
            << "selected element: " << sidebar.get_selected_of_index()
            << "\ndisplay mode: " << display_mode_info
            << "\nFPS: " << static_cast<int>(fps) << "/" << playbackspeed
            << "\nParticles: " << sim.particle_count;

        info_top.setString(oss.str());

        sim.draw_sfml(window, draw_mode);
        sim.draw_brush_outline_sfml(window, brush_size, mouse_x, mouse_y);
        sim.draw_particle_information_sfml(window, mouse_x, mouse_y);

        sidebar.draw_sfml(window);
        display_sidebar.draw_sfml(window);

        window.draw(info_top);

        window.display();
    }
}