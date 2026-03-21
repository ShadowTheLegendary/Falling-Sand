#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/System/Vector2.hpp>

#include <vector>
#include <string>
#include <unordered_map>

#include "particles.hpp"

class ParticleSimulation {
private:
	sf::Vector2i size;


	std::vector<Particle> particle_layers;
	std::vector<Particle> pending_particle_layers;


	int get_index(sf::Vector2i position) const;


	sf::Vector2i get_coordinate(int index) const;


	std::vector<sf::Vector2i> get_surroundings(sf::Vector2i pos) const;


	void swap(sf::Vector2i a, sf::Vector2i b);


	void update_temp(Particle& particle, int coordinate_index, std::vector<sf::Vector2i>& surroundings);


	void update_material(Particle& particle, int coordinate_index);


	void update_movement(Particle& particle, sf::Vector2i coordinate, int coordinate_index, std::vector<sf::Vector2i>& surroundings);


	void update_particle(sf::Vector2i coordinate);


	int cell_px = 8;
	int gap = 1;


public:
	int particle_count = 0;


    ParticleSimulation(sf::Vector2i size);


	void update();


	void brush(int brush_size, sf::Vector2i mouse_pos, MaterialID material, float power = 1);


	void draw_sfml(sf::RenderWindow& window, bool use_temp_coloring = false, int cell_size = 5);


	void draw_brush_outline_sfml(sf::RenderWindow& window, int brush_size, sf::Vector2i mouse_pos);


	void draw_particle_information_sfml(sf::RenderWindow& window, sf::Vector2i mouse_pos);

};