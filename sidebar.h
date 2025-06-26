#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <variant>
#include <unordered_map>

#include <SFML/Graphics.hpp>

class Sidebar {
private:
    struct Option {
        std::string name;
        std::variant<sf::Color, std::string> visual; // color or image path
        bool is_image() const { return std::holds_alternative<std::string>(visual); }
        const sf::Color& color() const { return std::get<sf::Color>(visual); }
        const std::string& image_path() const { return std::get<std::string>(visual); }
    };

    std::vector<Option> options;
    std::unordered_map<std::string, sf::Texture> textures; // cache for loaded images
    std::string bar_alignment;
    int selected_index = -1;

public:
    Sidebar(std::vector<std::string> elements, std::vector<sf::Color> colors, std::string alignment = "right") {
        if (elements.size() != colors.size()) {
            std::cerr << "elements and colors provided do not match";
            return;
        }

        bar_alignment = alignment;

        for (int i = 0; i < elements.size(); i++) {
            options.push_back({ elements[i], colors[i] });
        }
    }

    void add_option_img(const std::string& name, const std::string& filepath) {
        // Load texture if not already loaded
        if (textures.find(filepath) == textures.end()) {
            sf::Texture tex;
            if (!tex.loadFromFile(filepath)) {
                std::cerr << "Failed to load image: " << filepath << std::endl;
                return;
            }
            textures[filepath] = std::move(tex);
        }
        options.push_back({ name, filepath });
    }

    void handle_click(const sf::Vector2i& mouse_pos, sf::RenderWindow& window, int square_size = 32, int padding = 8) {
        int num_options = static_cast<int>(options.size());
        int bar_width = square_size + 2 * padding;
        int bar_height = num_options * (square_size + padding) + padding;
        sf::Vector2u win_size = window.getSize();
        int x = (bar_alignment == "left") ? 0 : (win_size.x - bar_width);
        int y = (win_size.y - bar_height) / 2;

        for (int i = 0; i < num_options; ++i) {
            int square_x = x + padding;
            int square_y = y + padding + i * (square_size + padding);
            sf::IntRect rect{ {square_x, square_y}, {square_size, square_size} };
            if (rect.contains(mouse_pos)) {
                selected_index = i;
                break;
            }
        }
    }

    std::string get_selected_of_index() {
        if (selected_index != -1) {
            return options[selected_index].name;
        }
        else {
            return "none";
        }
    }

    void draw_sfml(sf::RenderWindow& window, int square_size = 32, int padding = 8) {
        int num_options = static_cast<int>(options.size());
        int bar_width = square_size + 2 * padding;
        int bar_height = num_options * (square_size + padding) + padding;

        sf::Vector2u win_size = window.getSize();
        int x = (bar_alignment == "left") ? 0 : (win_size.x - bar_width);
        int y = (win_size.y - bar_height) / 2;

        for (int i = 0; i < num_options; ++i) {
            int square_x = x + padding;
            int square_y = y + padding + i * (square_size + padding);

            if (options[i].is_image()) {
                const std::string& path = options[i].image_path();
                auto it = textures.find(path);
                if (it != textures.end()) {
                    sf::Sprite sprite(it->second);
                    sprite.setPosition(sf::Vector2f(static_cast<float>(square_x), static_cast<float>(square_y)));
                    // Scale sprite to fit square_size
                    auto texSize = it->second.getSize();
                    float scale_x = square_size / static_cast<float>(texSize.x);
                    float scale_y = square_size / static_cast<float>(texSize.y);
                    sprite.setScale(sf::Vector2f(scale_x, scale_y));
                    window.draw(sprite);
                }
            }
            else {
                sf::RectangleShape rect(sf::Vector2f(static_cast<float>(square_size), static_cast<float>(square_size)));
                rect.setPosition(sf::Vector2f(static_cast<float>(square_x), static_cast<float>(square_y)));
                rect.setFillColor(options[i].color());
                window.draw(rect);
            }

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

    void discover_element(std::string element, sf::Color color) {
        options.push_back({ element, color });
    }

    bool is_discovered(std::string element) {
        for (const Option& option : options) {
            if (option.name == element) {
                return true;
            }
        }
        return false;
    }
};