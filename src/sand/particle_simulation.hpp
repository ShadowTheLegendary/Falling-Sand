#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/System/Vec2.hpp>

#include <SFML/Base/SizeT.hpp>
#include <SFML/Base/Vector.hpp>

#include <unordered_map>

#include "particles.hpp"


class ParticleSimulation {
public:
	////////////////////////////////////////////////////////////
	// \brief Initializes an empty simulation                 
	// \param size the size of the simulation {width, height} 
	////////////////////////////////////////////////////////////
    ParticleSimulation(sf::Vec2u size);

	////////////////////////////////////////////
	// \brief Updates the simulation one step 
	////////////////////////////////////////////
	void update();

	/////////////////////////////////////////////////////////////////////////////////////////
	// \brief Manipulates values in the simulation                                         
	// \param brush_size The size of the square of influence                               
	// \param position The center of the square of influence                               
	// \param material Every pixel in the square of influence will be set to this material 
	/////////////////////////////////////////////////////////////////////////////////////////
	void brush(int brush_size, sf::Vec2i position, MaterialID material);

	/////////////////////////////////////////////////////////////////////////////////////
	// \brief Draws the current state of the simulation using sfml                     
	// \param target The sfml target to draw the image to                            
	// \param use_temp_coloring If true the colors will be based on the particles temp 
	/////////////////////////////////////////////////////////////////////////////////////
	void draw_sfml(sf::RenderTarget& target);

	void draw_brush_outline_sfml(sf::RenderWindow& window, int brush_size, sf::Vec2i mouse_pos);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// \brief Returns information about the particle at position                                 
	// \param position The position (relative to the window) of the particle you want the info of
	////////////////////////////////////////////////////////////////////////////////////////////////
	ParticleInformation get_particle_information(sf::Vec2i position);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// \brief Returns the number of non air particles in the simulation
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	sf::base::SizeT get_particle_count();

private:
	int get_index(sf::Vec2i position) const;

	sf::Vec2i get_coordinate(int index) const;

	sf::base::Vector<sf::Vec2i> get_surroundings(sf::Vec2i pos) const;

	void swap(sf::Vec2i a, sf::Vec2i b);

	void update_temp(Particle& particle, int coordinate_index, sf::base::Vector<sf::Vec2i>& surroundings);

	void update_material(Particle& particle, int coordinate_index);

	void update_movement(Particle& particle, sf::Vec2i coordinate, int coordinate_index, sf::base::Vector<sf::Vec2i>& surroundings);

	void update_particle(sf::Vec2i coordinate);

	const sf::Font font;

	sf::Vec2u size;

	sf::base::Vector<Particle> particle_layers;

	sf::base::SizeT multithreading_core_count = 4;
	unsigned int multithreading_kernel_size = 8;

	sf::base::U8 cell_px = 8;
	sf::base::U8 gap = 1;
};