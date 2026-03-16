#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

#include <sstream>
#include <random>

#include "src/multi-threading/thread_pool.hpp"
#include "particles.hpp"

extern sf::Font arial;

inline std::string round_to_str(float value, int precision = 2) {
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(precision) << value;
	return stream.str();
}

class ParticleSimulation {
private:
	sf::Vector2i size;

	int get_index(sf::Vector2i position) const;

	sf::Vector2i get_coordinate(int index) const;

	std::vector<Particle> particle_layers;
	std::vector<Particle> pending_particle_layers;

	int cell_px = 8;
	int gap = 1;

public:
	std::unordered_map<std::string, bool> discovered_particles;
	int particle_count = 0;

    ParticleSimulation(sf::Vector2i size);

	std::vector<sf::Vector2i> get_surroundings(sf::Vector2i pos) const;

	void swap(sf::Vector2i a, sf::Vector2i b);

	void update_temp(Particle& particle, int coordinate_index, std::vector<sf::Vector2i>& surroundings);

	void update_material(Particle& particle, int coordinate_index);

	void update_movement(Particle& particle, sf::Vector2i coordinate, std::vector<sf::Vector2i>& surroundings);

	void update();

	void brush(int brush_size, sf::Vector2i mouse_pos, MaterialID material, float power = 1);

	void draw_sfml(sf::RenderWindow& window, bool use_temp_coloring = false, int cell_size = 5);

	void draw_brush_outline_sfml(sf::RenderWindow& window, int brush_size, sf::Vector2i mouse_pos);

	void draw_particle_information_sfml(sf::RenderWindow& window, sf::Vector2i mouse_pos);
};