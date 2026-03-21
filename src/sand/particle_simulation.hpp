#pragma once

#include <SFML/Graphics/RenderTarget.hpp>

#include <SFML/System/Vector2.hpp>

#include <vector>
#include <string>
#include <unordered_map>

#include "particles.hpp"

class ParticleSimulation {
public:
	//////////////////////////////////////
	// \brief Initializes an empty simulation
	// \param size the size of the simulation {width, height}
	//////////////////////////////////////
    ParticleSimulation(sf::Vector2i size);

	//////////////////////////////////////
	// \brief Updates the simulation one step
	//////////////////////////////////////
	void update();

	//////////////////////////////////////
	// \brief Manipulates values in the simulation 
	// \param brush_size The size of the square of influence
	// \param position The center of the square of influence
	// \param material Every pixel in the square of influence will be set to this material
	//////////////////////////////////////
	void brush(int brush_size, sf::Vector2i position, MaterialID material);

	//////////////////////////////////////
	// \brief Draws the current state of the simulation using sfml
	// \param target The sfml target to draw the image to
	// \param use_temp_coloring If true the colors will be based on the particles temp
	//////////////////////////////////////
	void draw_sfml(sf::RenderTarget& target, bool use_temp_coloring = false);

	void draw_brush_outline_sfml(sf::RenderWindow& window, int brush_size, sf::Vector2i mouse_pos);

	void draw_particle_information_sfml(sf::RenderWindow& window, sf::Vector2i mouse_pos); // TODO: Change to return particle information

	std::size_t get_particle_count() {
		return particle_count;
	}

private:
	int get_index(sf::Vector2i position) const;

	sf::Vector2i get_coordinate(int index) const;

	std::vector<sf::Vector2i> get_surroundings(sf::Vector2i pos) const;

	void swap(sf::Vector2i a, sf::Vector2i b);

	void update_temp(Particle& particle, int coordinate_index, std::vector<sf::Vector2i>& surroundings);

	void update_material(Particle& particle, int coordinate_index);

	void update_movement(Particle& particle, sf::Vector2i coordinate, int coordinate_index, std::vector<sf::Vector2i>& surroundings);

	void update_particle(sf::Vector2i coordinate);

	sf::Vector2i size;

	std::vector<Particle> particle_layers;
	std::vector<Particle> pending_particle_layers;

	std::size_t particle_count = 0;

	std::size_t multithreading_core_count = 4;
	unsigned int multithreading_kernel_size = 8;

	std::uint8_t cell_px = 8;
	std::uint8_t gap = 1;
};