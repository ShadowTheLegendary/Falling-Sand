#pragma once

#include <SFML/Graphics/Color.hpp>

#include <SFML/Base/Vector.hpp>
#include <SFML/Base/String.hpp>
#include <SFML/Base/Array.hpp>
#include <SFML/Base/Clamp.hpp>
#include <SFML/Base/IntTypes.hpp>

#include <random>


inline std::mt19937 gen(std::random_device{}());


enum class MaterialID : sf::base::U8 {
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


enum class BehaviorID : sf::base::U8 {
    Solid,
    Powder,
    Liquid,
    Gas,
    COUNT
};


struct Behavior {
    sf::base::String identifier = "none";
    sf::base::Vector<sf::base::U8> movement_weights;
};


struct Material {
    BehaviorID behavior;

	sf::base::String identifier = "none";

    float density;

    float state_change_high_temp;
    MaterialID state_change_high_new;

	float state_change_low_temp;
    MaterialID state_change_low_new;

	sf::Color base_color;
	int color_offset;
};


template <typename T, typename TID>
struct Table {
    sf::base::Array<T, (size_t)TID::COUNT> data;

    T& operator[](TID id) {
        return data[(size_t)id];
    }
};


inline Table<Behavior, BehaviorID> behaviors;
inline Table<Material, MaterialID> materials;


inline void register_material_behaviors() {
    behaviors[BehaviorID::Solid] = {
        .identifier = "solid",
        .movement_weights = {
            00, 00, 00,
            00, 10, 00,
            00, 00, 00
        }
    };

    behaviors[BehaviorID::Powder] = {
        .identifier = "powder",
        .movement_weights = {
            00, 00, 00,
            00, 05, 00,
            10, 20, 10
        }
    };

    behaviors[BehaviorID::Gas] = {
        .identifier = "gas",
        .movement_weights = {
            20, 30, 20,
            10, 05, 10,
            00, 00, 00
        }
    };

    behaviors[BehaviorID::Liquid] = {
        .identifier = "liquid",
        .movement_weights = {
            00, 00, 00,
            10, 05, 10,
            20, 30, 20
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
        .base_color = { 0, 0, 0 },
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
        .base_color = {128, 128, 128},
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
        .base_color = {255, 115, 0},
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
        .base_color = {255, 255, 0},
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
        .base_color = {255, 188, 79},
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
        .base_color = {207, 255, 245},
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
        .base_color = {0,128,255},
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
        .base_color = {131, 206, 255},
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
        .base_color = {200, 200, 200},
        .color_offset = 5,
    };
}


inline sf::Color random_color(MaterialID material) {
    sf::Color color = materials[material].base_color;
    std::uniform_int_distribution<int> color_dist(-materials[material].color_offset, materials[material].color_offset);

    sf::base::U8 r = sf::base::clamp(color.r + color_dist(gen), 0, 255);
    sf::base::U8 g = sf::base::clamp(color.g + color_dist(gen), 0, 255);
    sf::base::U8 b = sf::base::clamp(color.b + color_dist(gen), 0, 255);
    sf::base::U8 a = color.a;

   return {r, g, b, a};
}


class ParticleInformation {
public:
	bool valid_particle = false;
    sf::base::String material_name = "";
    sf::base::String behavior_name = "";
    float temp = 0;
};


struct Particle {
	MaterialID material;
	float temp;
	sf::Color color;
    bool moved = false;
};