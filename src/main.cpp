#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "sand/particle_simulation.hpp"
#include "ui/sidebar.hpp"
#include "viewport/viewport.hpp"

#include <string>
#include <sstream>

int main() {
    register_material_behaviors();
    register_materials();

    int playbackspeed = 15;

    sf::RenderWindow window(sf::VideoMode({ 640, 360 }), "Particle Sim");
    window.setFramerateLimit(playbackspeed);

    sf::View view;
    edit_viewport(window, view, {640, 360});

    ParticleSimulation sim({64, 32});

    Sidebar sidebar({ "sand", "rock", "water" }, { sf::Color(255, 255, 0), sf::Color(128, 128, 128), sf::Color(0, 128, 255) });
    sidebar.add_option_img("heat", "assets/heat_element.png");
    sidebar.add_option_img("cool", "assets/cool_element.png");
    sidebar.add_option_img("none", "assets/none_element.png");

    Sidebar display_sidebar({ "standard" }, { sf::Color(255, 255, 0) }, "bottom");
	display_sidebar.add_option_img("temperature", "assets/temp_display_element.png");

    bool paused = false;
    int brush_size = 5;
    bool draw_mode = false;

    sf::Vector2i mouse_pos;

    sf::Text info_top(arial, "", 15U);
    info_top.setPosition(sf::Vector2f(15, 268));
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
                    if (mouseWheelScrolled->delta > 0) {
                        brush_size = std::min(brush_size + 1, 50);
                    }
                    else if (mouseWheelScrolled->delta < 0) {
                        brush_size = std::max(brush_size - 1, 1);
                    }
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

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Space) {
                    paused = !paused;

                    paused_info = (paused) ? "paused\n" : "\n";
                }
                if (keyPressed->code == sf::Keyboard::Key::F && paused) {
                    sim.update();
                }
            }
            
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                edit_viewport(window, view, {640, 360});
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            break;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sim.brush(brush_size, mouse_pos, MaterialID::Sand, 5);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
                
                //sim.brush(brush_size, mousePos.x, mousePos.y, sidebar.get_selected_of_index(), 5);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                //sim.brush(brush_size, mousePos.x, mousePos.y, sidebar.get_selected_of_index(), 0.5);
            }
            else {
                //sim.brush(brush_size, mousePos.x, mousePos.y, sidebar.get_selected_of_index());
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            //sim.brush(brush_size, mousePos.x, mousePos.y, "none");
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

        window.clear();

        mouse_pos = sf::Mouse::getPosition(window);

        // Display FPS in the info text
        std::ostringstream oss;
        oss << paused_info
            << "selected element: " << sidebar.get_selected_of_index()
            << "\ndisplay mode: " << display_mode_info
            << "\nFPS: " << static_cast<int>(fps) << "/" << playbackspeed
            << "\nParticles: " << sim.particle_count;

        info_top.setString(oss.str());

        sim.draw_sfml(window, draw_mode);
        sim.draw_brush_outline_sfml(window, brush_size, mouse_pos);
        sim.draw_particle_information_sfml(window, mouse_pos);

        sidebar.draw_sfml(window);
        display_sidebar.draw_sfml(window);

        window.draw(info_top);

        window.display();
    }
}