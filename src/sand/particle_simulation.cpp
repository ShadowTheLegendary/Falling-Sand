#include <SFML/Graphics.hpp>

#include <vector>
#include <sstream>
#include <random>
#include <format>

#include "particle_simulation.hpp"
#include "particles.hpp"


sf::Font arial("fonts/Arial.ttf");


int ParticleSimulation::get_index(sf::Vector2i position) const {
    if (position.x < 0 or position.x >= size.x or position.y < 0 or position.y >= size.y) {
        return -1;
    }

    return position.y * size.x + position.x;
}


sf::Vector2i ParticleSimulation::get_coordinate(int index) const {
    if (index < 0 or index >= size.x * size.y) {
        return { -1, -1 };
    }

    const int x = index % size.x;
    const int y = index / size.x;
    return { x, y };
}


ParticleSimulation::ParticleSimulation(sf::Vector2i size) : size(size) {
    std::vector<Particle> temporary_buffer;

    size_t len = size.x * size.y;
    particle_layers.resize(len);

    for (int i = 0; i < len; i++) {
        Particle particle;
        particle.temp = 20;
        particle.material = MaterialID::Air;
        particle_layers[i] = particle;
    }

    pending_particle_layers = particle_layers;
}


std::vector<sf::Vector2i> ParticleSimulation::get_surroundings(sf::Vector2i pos) const {
    static const sf::Vector2i offsets[8] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},          {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };

    std::vector<sf::Vector2i> neighbors;
    for (int i = 0; i < 8; i++) {
        sf::Vector2i new_position = pos + offsets[i];
        if (new_position.x >= 0 and new_position.x < size.x and new_position.y >= 0 and new_position.y < size.y) {
            neighbors.push_back(new_position);
        }
    }
    return neighbors;
}


void ParticleSimulation::swap(sf::Vector2i a, sf::Vector2i b) {
    const int index_a = get_index(a);
    const int index_b = get_index(b);

    pending_particle_layers[index_a] = particle_layers[index_b];
    pending_particle_layers[index_b] = particle_layers[index_a];
    pending_particle_layers[index_a].moved = true;
    pending_particle_layers[index_b].moved = true;
}


void ParticleSimulation::update_temp(Particle& particle, int coordinate_index, std::vector<sf::Vector2i>& surroundings) {
    static const float temp_transfer = 0.1f;
    float delta = 0.0f;

    for (const sf::Vector2i& coords : surroundings) {
        int extern_index = get_index({ coords.x, coords.y });

        if (particle_layers[extern_index].material == MaterialID::Air) {
            continue;
        }

        float extern_temp = particle_layers[extern_index].temp;

        delta += temp_transfer * (extern_temp - particle.temp);
    }

    pending_particle_layers[coordinate_index].temp = particle.temp + delta;
    pending_particle_layers[coordinate_index].temp = std::clamp(pending_particle_layers[coordinate_index].temp, -273.0f, 5000.0f);
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

    pending_particle_layers[coordinate_index] = particle;
}


void ParticleSimulation::update_movement(Particle& particle, sf::Vector2i coordinate, int coordinate_index, std::vector<sf::Vector2i>& surroundings) {
    const static std::vector<sf::Vector2i> offsets = {
        { -1, -1 }, { 0, -1 }, { 1, -1 },
        { -1, 0 }, { 0, 0 }, { 1, 0 },
        { -1, 1}, { 0, 1 }, { 1, 1 }
    };

    if (pending_particle_layers[coordinate_index].moved) {
        return;
    }

    std::vector<int> valid_moves;
    std::vector<int> valid_weights;

    for (int i = 0; i < 9; i++) {
        int index = get_index(coordinate + offsets[i]);

        if ((index == -1) or (materials[particle.material].density < materials[particle_layers[index].material].density) or pending_particle_layers[index].moved) {
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

    swap(coordinate, coordinate + offsets[move_index]);
}


void ParticleSimulation::update_particle(sf::Vector2i coordinate) {
    int coordinate_index = get_index(coordinate);

    if (particle_layers[coordinate_index].material == MaterialID::Air) {
        return;
    }
    particle_count++;

    Particle particle = particle_layers[coordinate_index];

    std::vector<sf::Vector2i> surroundings = get_surroundings(coordinate);

    update_temp(particle, coordinate_index, surroundings);
    
    update_material(particle, coordinate_index);

    update_movement(particle, coordinate, coordinate_index, surroundings);
}


void ParticleSimulation::update() {
    pending_particle_layers = particle_layers;
    particle_count = 0;

    for (Particle& particle : pending_particle_layers) {
        particle.moved = false;
    }

    for (int y = size.y - 1; y >= 0; y--) {
        for (int x = 0; x < size.x; x++) {
            sf::Vector2i pos{x, y};

           update_particle(pos);
        }
    }

    particle_layers = pending_particle_layers;
}


void ParticleSimulation::brush(int brush_size, sf::Vector2i mouse_pos, MaterialID material, float power) {
    int cell_stride = cell_px + gap;

    if (mouse_pos.x < 0 or mouse_pos.y < 0 or mouse_pos.x > size.x * cell_stride or mouse_pos.y > size.y * cell_stride) {
        return;
    }

    sf::Vector2i grid = mouse_pos / cell_stride;
    int half = brush_size / 2;

    for (int i = -half; i <= half; ++i) {
        for (int j = -half; j <= half; ++j) {
            int x = grid.x + i;
            int y = grid.y + j;

            if (x < 0 or x >= size.x or y < 0 or y >= size.y) {
                continue;
            }

            int index = get_index({x, y});

            // float temp = particle_layers[index].temp;

            if (x >= 0 and x < size.x and y >= 0 and y < size.y and (particle_layers[index].material == MaterialID::Air or material == MaterialID::Air)) {
                particle_layers[index].temp = 20.0;
                particle_layers[index].material = material;

                particle_layers[index].color = random_color(material);
            }

            //temp = std::clamp(temp, -273.0f, 5000.0f);
            //particle_layers[index].temp = temp;
        }
    }
}


void ParticleSimulation::draw_sfml(sf::RenderWindow& window, bool use_temp_coloring, int cell_size) {
    const int grid_size_x = size.x;
    const int grid_size_y = size.y;

    sf::RectangleShape cell(sf::Vector2f(
        static_cast<float>(cell_px),
        static_cast<float>(cell_px)
    ));

    for (int i = 0; i < grid_size_x; ++i) {
        for (int j = 0; j < grid_size_y; ++j) {
            sf::Color color;
            if (use_temp_coloring) {
                if (particle_layers[get_index({i,j})].material == MaterialID::Air) {
                    color = particle_layers[get_index({i,j})].color;
                }
                else {
                    float t = particle_layers[get_index({i, j})].temp;

                    if (t <= 0.0f) {
                        // Purple fades to black as it gets colder
                        float ratio = (t + 273.0f) / 273.0f; // maps [-273, 0] → [0, 1]
                        unsigned char r = static_cast<unsigned char>(75 * ratio);  // from 0 to 75
                        unsigned char g = static_cast<unsigned char>(0);
                        unsigned char b = static_cast<unsigned char>(130 * ratio); // from 0 to 130
                        color = sf::Color(r, g, b);
                    }
                    else if (t <= 1000.0f) {
                        // Black to Red
                        float ratio = t / 1000.0f;
                        color = sf::Color(
                            static_cast<unsigned char>(255 * ratio),
                            0,
                            255
                        );
                    }
                    else if (t <= 2000.0f) {
                        // Red to Yellow (increasing green)
                        float ratio = (t - 1000.0f) / 1000.0f;
                        color = sf::Color(
                            255,
                            static_cast<unsigned char>(255 * ratio),
                            0
                        );
                    }
                    else {
                        // Yellow to White (increasing blue)
                        float ratio = (t - 2000.0f) / 1000.0f;
                        color = sf::Color(
                            255,
                            255,
                            static_cast<unsigned char>(255 * ratio)
                        );
                    }
                }

            }
            else {
                color = particle_layers[get_index({i,j})].color;
            }

            cell.setPosition(
                sf::Vector2f(
                    static_cast<float>(i * (cell_px + gap)),
                    static_cast<float>(j * (cell_px + gap))
                )
            );
            cell.setFillColor(color);
            window.draw(cell);
        }
    }
}


void ParticleSimulation::draw_brush_outline_sfml(sf::RenderWindow& window, int brush_size, sf::Vector2i mouse_pos) {
    int cell_stride = cell_px + gap;

    if (mouse_pos.x < 0 or mouse_pos.y < 0 or mouse_pos.x > size.x * cell_stride or mouse_pos.y > size.y * cell_stride) {
        return;
    }

    sf::Vector2i grid = mouse_pos / cell_stride;
    int half = brush_size / 2;

    sf::Vector2i min = { std::max(0, grid.x - half), std::max(0, grid.y - half) };

    sf::Vector2i max = { std::min(size.x - 1, grid.x + half), std::min(size.y - 1, grid.y + half) };

    float left = min.x * cell_stride;
    float top = min.y * cell_stride;
    float width = (max.x - min.x + 1) * cell_stride - gap;
    float height = (max.y - min.y + 1) * cell_stride - gap;

    sf::RectangleShape outline(sf::Vector2f(width, height));
    outline.setPosition(sf::Vector2f(static_cast<float>(left),static_cast<float>(top)));
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color::Red);
    outline.setOutlineThickness(2.f);

    window.draw(outline);
}


void ParticleSimulation::draw_particle_information_sfml(sf::RenderWindow& window, sf::Vector2i mouse_pos) {
    int cell_stride = cell_px + gap;
    
    sf::Vector2i mouse_pos_grid = mouse_pos / cell_stride;

    int index = get_index(mouse_pos_grid);

    if (index == -1) {
        return;
    }

    Particle particle = particle_layers[index];

    sf::Text info(arial, materials[particle.material].identifier + "\nTemp: " + std::format("{:.2f}", particle.temp) + "c\nState: " + behaviors[materials[particle.material].behavior].identifier, 15U);
    info.setPosition(sf::Vector2f(512, 300));
    window.draw(info);

}