#pragma once

#include <SFML/Graphics/Color.hpp>

#include <string>
#include <random>
#include <cstdint>
#include <algorithm>
#include <array>
#include <vector>


inline std::mt19937 gen(std::random_device{}());


enum class MaterialID : uint8_t {
    Air,
    Rock,
    Lava,
    Sand,
    MoltenGlass,
    Glass,
    Water,
    Ice,
    Steam,
    COUNT
};


enum class BehaviorID : uint8_t {
    Solid,
    Powder,
    Liquid,
    Gas,
    COUNT
};


struct Behavior {
    std::string identifier = "none";
    std::vector<uint8_t> movement_weights;
};


struct Material {
    BehaviorID behavior;

	std::string identifier = "none";

    float density;

    float state_change_high_temp;
    MaterialID state_change_high_new;

	float state_change_low_temp;
    MaterialID state_change_low_new;

	sf::Color base_color;
	int color_offset;
};


struct BehaviorTable {
    std::array<Behavior, (size_t)BehaviorID::COUNT> data;

    Behavior& operator[](BehaviorID id) {
        return data[(size_t)id];
    }
};


struct MaterialTable {
    std::array<Material, (size_t)MaterialID::COUNT> data;

    Material& operator[](MaterialID id) {
        return data[(size_t)id];
    }
};


inline BehaviorTable behaviors;
inline MaterialTable materials;


inline void register_material_behaviors() {
    behaviors[BehaviorID::Solid] = {
        .identifier = "solid",
        .movement_weights = {
            0, 0, 0,
            0, 1, 0,
            0, 0, 0
        }
    };

    behaviors[BehaviorID::Powder] = {
        .identifier = "powder",
        .movement_weights = {
            0, 0, 0,
            0, 1, 0,
            2, 2, 2
        }
    };

    behaviors[BehaviorID::Gas] = {
        .identifier = "gas",
        .movement_weights = {
            3, 3, 3,
            2, 1, 2,
            0, 0, 0
        }
    };

    behaviors[BehaviorID::Liquid] = {
        .identifier = "liquid",
        .movement_weights = {
            0, 0, 0,
            2, 1, 2,
            3, 3, 3
        }
    };
}


inline void register_materials() {
    materials[MaterialID::Air] = {
        .behavior = BehaviorID::Solid,
        .identifier = "air",
        .density = 0.f,
        .state_change_high_temp = 0,
        .state_change_high_new = MaterialID::Air,
        .state_change_low_temp = 0,
        .state_change_low_new = MaterialID::Air,
        .base_color = sf::Color(0, 0, 0),
        .color_offset = 0,
    };

    materials[MaterialID::Rock] = {
        .behavior = BehaviorID::Solid,
        .identifier = "rock",
        .density = 4.0f,
        .state_change_high_temp = 700,
        .state_change_high_new = MaterialID::Lava,
        .state_change_low_temp = 0,
        .state_change_low_new = MaterialID::Rock,
        .base_color = sf::Color(128, 128, 128),
        .color_offset = 5,
    };

    materials[MaterialID::Lava] = {
        .behavior = BehaviorID::Liquid,
        .identifier = "lava",
        .density = 0.3f,
        .state_change_high_temp = 0,
        .state_change_high_new = MaterialID::Lava,
        .state_change_low_temp = 699,
        .state_change_low_new = MaterialID::Rock,
        .base_color = sf::Color(255, 115, 0),
        .color_offset = 25,
    };

    materials[MaterialID::Sand] = {
        .behavior = BehaviorID::Powder,
        .identifier = "powder",
        .density = 1.1f,
        .state_change_high_temp = 1700,
        .state_change_high_new = MaterialID::MoltenGlass,
        .state_change_low_temp = 0,
        .state_change_low_new = MaterialID::Sand,
        .base_color = sf::Color(255, 255, 0),
        .color_offset = 25,
    };

    materials[MaterialID::MoltenGlass] = {
        .behavior = BehaviorID::Liquid,
        .identifier = "liquid",
        .density = 0.2f,
        .state_change_high_temp = 0,
        .state_change_high_new = MaterialID::MoltenGlass,
        .state_change_low_temp = 1499,
        .state_change_low_new = MaterialID::Glass,
        .base_color = sf::Color(255, 188, 79),
        .color_offset = 20,
    };

    materials[MaterialID::Glass] = {
        .behavior = BehaviorID::Solid,
        .identifier = "glass",
        .density = 4.0f,
        .state_change_high_temp = 1500,
        .state_change_high_new = MaterialID::MoltenGlass,
        .state_change_low_temp = 0,
        .state_change_low_new = MaterialID::Glass,
        .base_color = sf::Color(207, 255, 245),
        .color_offset = 5,
    };

    materials[MaterialID::Water] = {
        .behavior = BehaviorID::Liquid,
        .identifier = "water",
        .density = 0.1f,
        .state_change_high_temp = 100,
        .state_change_high_new = MaterialID::Steam,
        .state_change_low_temp = 0,
        .state_change_low_new = MaterialID::Ice,
        .base_color = sf::Color(0,128,255),
        .color_offset = 15,
    };

    materials[MaterialID::Ice] = {
        .behavior = BehaviorID::Solid,
        .identifier = "ice",
        .density = 4.0f,
        .state_change_high_temp = 1,
        .state_change_high_new = MaterialID::Water,
        .state_change_low_temp = 0,
        .state_change_low_new = MaterialID::Ice,
        .base_color = sf::Color(131, 206, 255),
        .color_offset = 15,
    };
    
    materials[MaterialID::Steam] = {
        .behavior = BehaviorID::Gas,
        .identifier = "steam",
        .density = 0.01f,
        .state_change_high_temp = 0,
        .state_change_high_new = MaterialID::Steam,
        .state_change_low_temp = 0,
        .state_change_low_new = MaterialID::Water,
        .base_color = sf::Color(200, 200, 200),
        .color_offset = 5,
    };
}


inline sf::Color random_color(MaterialID material) {
    sf::Color color = materials[material].base_color;
    std::uniform_int_distribution<int> color_dist(-materials[material].color_offset, materials[material].color_offset);

    int r = std::clamp(static_cast<int>(color.r) + color_dist(gen), 0, 255);
    int g = std::clamp(static_cast<int>(color.g) + color_dist(gen), 0, 255);
    int b = std::clamp(static_cast<int>(color.b) + color_dist(gen), 0, 255);
    int a = color.a;

   return sf::Color(r, g, b, a);
}


struct Particle {
	MaterialID material;
	float temp;
	sf::Color color;
    bool moved = false;
};