#include "SFML/Graphics/View.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/System/Vector2.hpp"

#include <cmath>

unsigned int calculate_scale(sf::Vector2u size) {
    sf::Vector2u scale = {
        size.x / 640,
        size.y / 360
    };

    return std::min(scale.x, scale.y);
}

void edit_viewport(sf::RenderWindow& window, sf::View& view, const sf::Vector2f& base) {
    const sf::Vector2f win = {
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y)
    };

    const float scale_float = std::min(win.x / base.x, win.y / base.y);
    const unsigned int scale_int = static_cast<unsigned int>(std::floor(scale_float + 1e-6f));
    const float used_scale = (scale_int >= 1u) ? static_cast<float>(scale_int) : scale_float;

    const sf::Vector2f scaled = base * used_scale;

    sf::Vector2f required_percent_of_screen = {
        (scaled.x / win.x) * 100.0f,
        (scaled.y / win.y) * 100.0f
    };

    const sf::Vector2f vp_norm_width_height = {
        scaled.x / win.x,
        scaled.y / win.y
    };

    const sf::Vector2f vp_norm_left_top = {
        (1.f - vp_norm_width_height.x) / 2.f,
        (1.f - vp_norm_width_height.y) / 2.f
    };

    view.setSize(base);
    view.setCenter(base * 0.5f);

    view.setViewport(sf::FloatRect({ vp_norm_left_top.x, vp_norm_left_top.y }, { vp_norm_width_height.x, vp_norm_width_height.y }));
    window.setView(view);
}