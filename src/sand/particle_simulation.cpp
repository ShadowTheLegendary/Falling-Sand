// #include <SFML/Graphics.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <SFML/System/Vec2.hpp>
#include "SFML/System/Path.hpp"

#include <SFML/Base/Optional.hpp>
#include <SFML/Base/Vector.hpp>
#include <SFML/Base/MinMax.hpp>
#include <SFML/Base/ThreadPool.hpp>

#include <random>
#include <format>
#include <iostream>

#include "particle_simulation.hpp"
#include "particles.hpp"


/////////////////////////////////////////////
// Public functions for ParticleSimulation //
/////////////////////////////////////////////

ParticleSimulation::ParticleSimulation(sf::Vec2u size) : size(size), font(sf::Font::openFromFile("fonts/Arial.ttf").value()) {
    unsigned int thread_count = sf::base::ThreadPool::getHardwareWorkerCount();
    multithreading_core_count = thread_count ? thread_count : 4;

    size_t len = size.x * size.y;
    particle_layers.resize(len);

    for (int i = 0; i < len; i++) {
        Particle particle;
        particle.temp = 20;
        particle.material = MaterialID::Air;
        particle_layers[i] = particle;
    }
}


void ParticleSimulation::update() {
    for (Particle& particle : particle_layers) {
        particle.moved = false;
    }

    for (int y = size.y - 1; y >= 0; y--) {
        for (int x = 0; x < size.x; x++) {
            sf::Vec2i pos{x, y};

           update_particle(pos);
        }
    }
}


void ParticleSimulation::brush(int brush_size, sf::Vec2i position, MaterialID material) {
    int cell_stride = cell_px + gap;

    if (position.x < 0 or position.y < 0 or position.x > size.x * cell_stride or position.y > size.y * cell_stride) {
        return;
    }

    sf::Vec2i grid = position / cell_stride;
    int half = brush_size / 2;

    for (int i = -half; i <= half; ++i) {
        for (int j = -half; j <= half; ++j) {
            int x = grid.x + i;
            int y = grid.y + j;

            if (x < 0 or x >= size.x or y < 0 or y >= size.y) {
                continue;
            }

            int index = get_index({x, y});

            if (x >= 0 and x < size.x and y >= 0 and y < size.y and (particle_layers[index].material == MaterialID::Air or material == MaterialID::Air)) {
                particle_layers[index].temp = 20.0;
                particle_layers[index].material = material;

                particle_layers[index].color = random_color(material);
            }
        }
    }
}


void ParticleSimulation::draw_sfml(sf::RenderTarget& target) {
    for (int x = 0; x < size.x; x++) {
        for (int y = 0; y < size.y; y++) {
            sf::Color color = particle_layers[get_index({x,y})].color;

            sf::Vec2f pos = {x * (cell_px + gap), y * (cell_px + gap)};

            sf::base::Vector<sf::Vertex> cell;
            cell.pushBack(sf::Vertex{.position=pos, .color=color});
            cell.pushBack(sf::Vertex{.position={pos.x+cell_px, pos.y}, .color=color});
            cell.pushBack(sf::Vertex{.position={pos.x+cell_px, pos.y+cell_px}, .color=color});
            cell.pushBack(sf::Vertex{.position={pos.x, pos.y+cell_px}, .color=color});
            cell.pushBack(sf::Vertex{.position=pos, .color=color});
        
            target.draw(cell, sf::PrimitiveType::TriangleStrip);
        }
    }
}


void ParticleSimulation::draw_brush_outline_sfml(sf::RenderWindow& window, int brush_size, sf::Vec2i mouse_pos) {
    int cell_stride = cell_px + gap;

    if (mouse_pos.x < 0 || mouse_pos.y < 0 || mouse_pos.x > (int)(size.x * cell_stride) || mouse_pos.y > (int)(size.y * cell_stride)) {
        return;
    }

    sf::Vec2i grid = mouse_pos / cell_stride;
    int half = brush_size / 2;

    sf::Vec2i min = {
        sf::base::max(0, grid.x - half),
        sf::base::max(0, grid.y - half)
    };

    sf::Vec2i max = {
        sf::base::min((int)size.x - 1, grid.x + half),
        sf::base::min((int)size.y - 1, grid.y + half)
    };

    float left = min.x * cell_stride;
    float top = min.y * cell_stride;
    float width = (max.x - min.x + 1) * cell_stride - gap;
    float height = (max.y - min.y + 1) * cell_stride - gap;

    sf::RectangleShape outline({
        .position = {left, top},
        .size = {width, height},
    });

    outline.setOutlineColor(sf::Color::Red);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineThickness(2.f);

    window.draw(outline);
}


ParticleInformation ParticleSimulation::get_particle_information(sf::Vec2i position) {
    int cell_stride = cell_px + gap;
    
    sf::Vec2i mouse_pos_grid = position / cell_stride;

    int index = get_index(mouse_pos_grid);

    if (index == -1) {
        return ParticleInformation();
    }

    Particle particle = particle_layers[index];

    ParticleInformation info;
    info.valid_particle = true;
    info.material_name = materials[particle.material].identifier;
    info.behavior_name = behaviors[materials[particle.material].behavior].identifier;
    info.temp = particle.temp;

    return info;
}


sf::base::SizeT ParticleSimulation::get_particle_count() {
    sf::base::SizeT particle_count = 0;

    for (const Particle& particle : particle_layers) {
        if (particle.material != MaterialID::Air) {
            particle_count++;
        }
    }

    return particle_count;
}


//////////////////////////////////////////////
// Private functions for ParticleSimulation //
//////////////////////////////////////////////

int ParticleSimulation::get_index(sf::Vec2i position) const {
    if (position.x < 0 or position.x >= size.x or position.y < 0 or position.y >= size.y) {
        return -1;
    }

    return position.y * size.x + position.x;
}


sf::Vec2i ParticleSimulation::get_coordinate(int index) const {
    if (index < 0 or index >= size.x * size.y) {
        return { -1, -1 };
    }

    const int x = index % size.x;
    const int y = index / size.x;
    return { x, y };
}


sf::base::Vector<sf::Vec2i> ParticleSimulation::get_surroundings(sf::Vec2i pos) const {
    static const sf::Vec2i offsets[8] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},          {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };

    sf::base::Vector<sf::Vec2i> neighbors;
    for (int i = 0; i < 8; i++) {
        sf::Vec2i new_position = pos + offsets[i];
        if (new_position.x >= 0 and new_position.x < size.x and new_position.y >= 0 and new_position.y < size.y) {
            neighbors.pushBack(new_position);
        }
    }
    return neighbors;
}


void ParticleSimulation::swap(sf::Vec2i a, sf::Vec2i b) {
    const int index_a = get_index(a);
    const int index_b = get_index(b);

    const Particle particle_a = particle_layers[index_a];

    particle_layers[index_a] = particle_layers[index_b];
    particle_layers[index_b] = particle_a;
    particle_layers[index_a].moved = true;
    particle_layers[index_b].moved = true;
}


void ParticleSimulation::update_temp(Particle& particle, int coordinate_index, sf::base::Vector<sf::Vec2i>& surroundings) {
    static const float temp_transfer = 0.1f;
    float delta = 0.0f;

    for (const sf::Vec2i& coords : surroundings) {
        int extern_index = get_index({ coords.x, coords.y });

        if (particle_layers[extern_index].material == MaterialID::Air) {
            continue;
        }

        float extern_temp = particle_layers[extern_index].temp;

        delta += temp_transfer * (extern_temp - particle.temp);
    }

    particle_layers[coordinate_index].temp = particle.temp + delta;
    particle_layers[coordinate_index].temp = std::clamp(particle_layers[coordinate_index].temp, -273.0f, 5000.0f);
}


void ParticleSimulation::update_material(Particle& particle, int coordinate_index) {
    MaterialID new_material = particle.material;
    float launchpad = 5.f;

    if ((particle.temp > materials[particle.material].state_change_high_temp) and (materials[particle.material].state_change_high_new != particle.material)) {
        new_material = materials[particle.material].state_change_high_new;
        particle.temp += launchpad;
        
    }
    if ((particle.temp < materials[particle.material].state_change_low_temp) and (materials[particle.material].state_change_low_new != particle.material)) {
        new_material = materials[particle.material].state_change_low_new;
        particle.temp -= launchpad;
    }

    if (new_material == particle.material) {
        return;
    }

    particle.material = new_material;
    particle.color = random_color(particle.material);

    particle_layers[coordinate_index] = particle;
}


void ParticleSimulation::update_movement(Particle& particle, sf::Vec2i coordinate, int coordinate_index, sf::base::Vector<sf::Vec2i>& surroundings) {
    const static std::vector<sf::Vec2i> offsets = {
        { -1, -1 }, { 0, -1 }, { 1, -1 },
        { -1, 0 }, { 0, 0 }, { 1, 0 },
        { -1, 1}, { 0, 1 }, { 1, 1 }
    };

    if (particle_layers[coordinate_index].moved) {
        return;
    }

    std::vector<int> valid_moves;
    std::vector<int> valid_weights;

    for (int i = 0; i < 9; i++) {
        int weight = behaviors[materials[particle.material].behavior].movement_weights[i];
        if (weight == 0) {
            continue;
        }

        if (i == 4) { 
            valid_moves.push_back(i); 
            valid_weights.push_back(weight); 
            continue; 
        }

        int index = get_index(coordinate + offsets[i]);
        if (index == -1) {
            continue;
        }

        if (particle_layers[index].moved) {
            continue;
        }

        bool denser = materials[particle.material].density > materials[particle_layers[index].material].density;
        if (not denser) {
            continue;
        }

        valid_moves.push_back(i);
        valid_weights.push_back(behaviors[materials[particle.material].behavior].movement_weights[i]);
    }

    if (valid_moves.empty()) {
        return;
    }

    std::discrete_distribution<> dist(valid_weights.begin(), valid_weights.end());

    int choice = dist(gen);
    int move_index = valid_moves[choice];

    if (move_index == 4) {
        return; // Don't mark particle as moved when it stayed still
    }

    swap(coordinate, coordinate + offsets[move_index]);
}


void ParticleSimulation::update_particle(sf::Vec2i coordinate) {
    int coordinate_index = get_index(coordinate);

    if (particle_layers[coordinate_index].material == MaterialID::Air) {
        return;
    }

    Particle particle = particle_layers[coordinate_index];

    sf::base::Vector<sf::Vec2i> surroundings = get_surroundings(coordinate);

    update_temp(particle, coordinate_index, surroundings);
    
    update_material(particle, coordinate_index);

    update_movement(particle, coordinate, coordinate_index, surroundings);
}