#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/GraphicsContext.hpp>

#include <SFML/Window/WindowContext.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/EventUtils.hpp>

#include <SFML/System/Clock.hpp>
#include "SFML/System/Path.hpp"
#include "SFML/System/UnicodeString.hpp"
#include "SFML/System/IO.hpp"

#include <SFML/Base/Optional.hpp>

#include <algorithm>
#include <string>
#include <sstream>
#include <optional>
#include <format>

#include "sand/particle_simulation.hpp"
#include "sand/particles.hpp"
#include "ui/sidebar.hpp"

constexpr sf::Vec2u window_size = {640, 360};

int main() {
    auto graphics_context = sf::GraphicsContext::create().value();
    // auto window_context = sf::WindowContext::create().value();

    auto window = sf::RenderWindow::create({
        .size = window_size,
        .title = "Falling Sand",
        .resizable = false,
        .vsync = false,
    }).value();
    window.setFramerateLimit(45);

    register_material_behaviors();
    register_materials();

    ParticleSimulation sim(sf::Vec2u{64u, 32u});
    Sidebar sidebar({ MaterialID::Sand, MaterialID::Rock, MaterialID::Water, MaterialID::Steam }, { {255, 255, 0}, {128, 128, 128}, {0, 128, 255}, sf::Color::Green });

    int brush_size = 5;

    sf::Vec2i mouse_pos;

    const sf::Font arial = sf::Font::openFromFile("fonts/Arial.ttf").value();

    sf::Text general_info(arial, {
        .position = {15.f, 268.f},
        .characterSize = 15U,
    });

    sf::Text particle_info(arial, {
        .position = {450.f, 288.f},
        .characterSize = 15U,
    });

    bool paused = false;
    std::string paused_info = "\n";

    std::string display_mode_info = "standard";

    sf::Clock clock;

    while (true) {
        mouse_pos = sf::Mouse::getPosition(window);
        sf::base::I32 frame_time = clock.restart().asMilliseconds();

        // Handle input
        while (const auto event = window.pollEvent()) {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event)) {
                return 0;
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

            // Arrow key pressed:
            if (event->is<sf::Event::KeyPressed>()) {
                switch (event->getIf<sf::Event::KeyPressed>()->code) {
                    case sf::Keyboard::Key::Space:
                        paused = !paused;

                        paused_info = (paused) ? "paused\n" : "\n";
                        break;
                    case sf::Keyboard::Key::F:
                        sim.update();
                        break;
                    default:
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
        sf::OutStringStream general_info_str;
        general_info_str << paused_info
            << "selected element: " << materials[sidebar.get_selected_of_index()].identifier
            << "\ndisplay mode: " << display_mode_info
            << "\nframe time: " << frame_time << "ms"
            << "\nparticle count: " << sim.get_particle_count();

        general_info.setString(general_info_str.to<sf::UnicodeString>());

        ParticleInformation info = sim.get_particle_information(mouse_pos);

        if (info.valid_particle) {
            sf::OutStringStream particle_info_str;
            particle_info_str << info.material_name << "\n"
            << info.behavior_name << "\n"
            << info.temp << "c\n";
            particle_info.setString(particle_info_str.to<sf::UnicodeString>());
        }
        else {
            particle_info.setString("");
        }

        window.clear();

        sim.draw_sfml(window);
        sim.draw_brush_outline_sfml(window, brush_size, mouse_pos);

        sidebar.draw_sfml(window);

        window.draw(particle_info);
        window.draw(general_info);

        window.display();
    }
}