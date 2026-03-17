#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <variant>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "src/sand/particles.hpp"

class Sidebar {
private:
    struct Option {
        MaterialID id;
        sf::Color color;
    };

    std::vector<Option> options;
    std::string bar_alignment;
    int selected_index = -1;

public:
    Sidebar(std::vector<MaterialID> elements, std::vector<sf::Color> colors, std::string alignment = "right") {
        if (elements.size() != colors.size()) {
            std::cerr << "elements and colors provided do not match";
            return;
        }

        bar_alignment = alignment;

        for (int i = 0; i < elements.size(); i++) {
            options.push_back({ elements[i], colors[i] });
        }
    }

    void handle_click(const sf::Vector2i& mouse_pos, sf::RenderWindow& window, int square_size = 32, int padding = 8) {
        int num_options = static_cast<int>(options.size());
        sf::Vector2u win_size = window.getSize();

        int x = 0, y = 0;
        int bar_width = square_size + 2 * padding;
        int bar_height = num_options * (square_size + padding) + padding;

        if (bar_alignment == "left") {
            x = 0;
            y = (win_size.y - bar_height) / 2;
        }
        else if (bar_alignment == "right") {
            x = win_size.x - bar_width;
            y = (win_size.y - bar_height) / 2;
        }
        else if (bar_alignment == "top") {
            x = (win_size.x - bar_height) / 2;
            y = 0;
        }
        else if (bar_alignment == "bottom") {
            x = (win_size.x - bar_height) / 2;
            y = win_size.y - bar_width;
        }
        else {
            // Default to right
            x = win_size.x - bar_width;
            y = (win_size.y - bar_height) / 2;
        }

        for (int i = 0; i < num_options; ++i) {
            int square_x, square_y;
            if (bar_alignment == "left" || bar_alignment == "right") {
                square_x = x + padding;
                square_y = y + padding + i * (square_size + padding);
            }
            else { // top or bottom
                square_x = x + padding + i * (square_size + padding);
                square_y = y + padding;
            }
            sf::IntRect rect{ {square_x, square_y}, {square_size, square_size} };
            if (rect.contains(mouse_pos)) {
                selected_index = i;
                break;
            }
        }
    }

    MaterialID get_selected_of_index() {
        if (selected_index != -1) {
            return options[selected_index].id;
        }
        return MaterialID::Air;
    }

    void draw_sfml(sf::RenderWindow& window, int square_size = 32, int padding = 8) {
        int num_options = static_cast<int>(options.size());
        sf::Vector2u win_size = window.getSize();

        int x = 0, y = 0;
        int bar_width = square_size + 2 * padding;
        int bar_height = num_options * (square_size + padding) + padding;

        if (bar_alignment == "left") {
            x = 0;
            y = (win_size.y - bar_height) / 2;
        }
        else if (bar_alignment == "right") {
            x = win_size.x - bar_width;
            y = (win_size.y - bar_height) / 2;
        }
        else if (bar_alignment == "top") {
            x = (win_size.x - bar_height) / 2;
            y = 0;
        }
        else if (bar_alignment == "bottom") {
            x = (win_size.x - bar_height) / 2;
            y = win_size.y - bar_width;
        }
        else {
            // Default to right
            x = win_size.x - bar_width;
            y = (win_size.y - bar_height) / 2;
        }

        for (int i = 0; i < num_options; ++i) {
            int square_x, square_y;
            if (bar_alignment == "left" || bar_alignment == "right") {
                square_x = x + padding;
                square_y = y + padding + i * (square_size + padding);
            }
            else { // top or bottom
                square_x = x + padding + i * (square_size + padding);
                square_y = y + padding;
            }

            sf::RectangleShape rect(sf::Vector2f(static_cast<float>(square_size), static_cast<float>(square_size)));
            rect.setPosition(sf::Vector2f(static_cast<float>(square_x), static_cast<float>(square_y)));
            rect.setFillColor(options[i].color);
            window.draw(rect);

            if (i == selected_index) {
                sf::RectangleShape border(sf::Vector2f(static_cast<float>(square_size), static_cast<float>(square_size)));
                border.setPosition(sf::Vector2f(static_cast<float>(square_x), static_cast<float>(square_y)));
                border.setFillColor(sf::Color::Transparent);
                border.setOutlineColor(sf::Color::White);
                border.setOutlineThickness(3.f);
                window.draw(border);
            }
        }
    }
};