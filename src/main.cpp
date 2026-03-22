#include <SFML/Graphics/RenderWindow.hpp>

#include <algorithm>
#include <string>
#include <sstream>
#include <optional>

#include "sand/particle_simulation.hpp"
#include "sand/particles.hpp"
#include "ui/sidebar.hpp"
#include "viewport/viewport.hpp"
#include "fps/fps.hpp"


int main() {
    register_material_behaviors();
    register_materials();

    int playback_speed = 0;

    sf::RenderWindow window(sf::VideoMode({ 640, 360 }), "Particle Sim");
    window.setFramerateLimit(playback_speed);

    sf::View view;
    edit_viewport(window, view, {640, 360});

    ParticleSimulation sim({64, 32});
    Sidebar sidebar({ MaterialID::Sand, MaterialID::Rock, MaterialID::Water, MaterialID::Steam }, { sf::Color(255, 255, 0), sf::Color(128, 128, 128), sf::Color(0, 128, 255), sf::Color::Green });

    int brush_size = 5;

    sf::Vector2i mouse_pos;

    sf::Font arial("fonts/Arial.ttf");

    sf::Text general_info(arial, "", 15U);
    general_info.setPosition(sf::Vector2f(15, 268));

    sf::Text particle_info(arial, "", 15U);
    particle_info.setPosition(sf::Vector2f(450, 288));

    bool paused = false;
    std::string paused_info = "\n";

    std::string display_mode_info = "standard";

    FpsCounter counter;

    while (window.isOpen()) {
        mouse_pos = sf::Mouse::getPosition(window);
        float fps = counter.update();

        // Handle input
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>() or sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                window.close();
            }

            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                sidebar.handle_click(mouse_pos, window);
            }

            if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheelScrolled->wheel == sf::Mouse::Wheel::Vertical) {
                    if (mouseWheelScrolled->delta > 0) {
                        brush_size = std::min(brush_size + 1, 50);
                    }
                    else if (mouseWheelScrolled->delta < 0) {
                        brush_size = std::max(brush_size - 1, 1);
                    }
                }
            }

            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                if (textEntered->unicode >= 48 and textEntered->unicode <= 57) { // 1 - 9
                    playback_speed = 15 * (textEntered->unicode - 48);
                    window.setFramerateLimit(playback_speed);
                }
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Space) {
                    paused = !paused;

                    paused_info = (paused) ? "paused\n" : "\n";
                }

                if (keyPressed->code == sf::Keyboard::Key::F && paused) {
                    sim.update();
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                    break;
                }
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sim.brush(brush_size, mouse_pos, sidebar.get_selected_of_index());
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            sim.brush(brush_size, mouse_pos, MaterialID::Air);
        }

        if (!paused) {
            sim.update();
        }

        // Update ui
        std::ostringstream general_info_str;
        general_info_str << paused_info
            << "selected element: " << materials[sidebar.get_selected_of_index()].identifier
            << "\ndisplay mode: " << display_mode_info
            << "\nFPS: " << std::format("{:.1f}", fps) << "/" << playback_speed
            << "\nParticles: " << sim.get_particle_count();

        general_info.setString(general_info_str.str());

        ParticleInformation info = sim.get_particle_information(mouse_pos);

        if (info.valid_particle) {
            std::ostringstream particle_info_str;
            particle_info_str << info.material_name << "\n"
            << info.behavior_name << "\n"
            << info.temp << "c\n";
            particle_info.setString(particle_info_str.str());
        }
        else {
            particle_info.setString("");
        }

        window.clear();

        sim.draw_sfml(window, false);
        sim.draw_brush_outline_sfml(window, brush_size, mouse_pos);

        sidebar.draw_sfml(window);

        window.draw(particle_info);
        window.draw(general_info);

        window.display();
    }
}