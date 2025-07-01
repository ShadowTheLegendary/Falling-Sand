#pragma once

#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include <utility>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <random>
#include <unordered_map>
#include <thread>

std::unordered_map<std::string, sf::Color> color_by_type = {
	{"air", sf::Color(25, 25, 25, 255)},
	{"sand", sf::Color(255, 255, 0)},
	{"rock", sf::Color(128, 128, 128)},
	{"water", sf::Color(0, 128, 255)},
	{"ice", sf::Color(131, 206, 255)},
	{"steam", sf::Color(200, 200, 200)},
	{"lava", sf::Color(255, 115, 0)},
	{"molten glass", sf::Color(255, 188, 79)},
	{"glass", sf::Color(207, 255, 245)},
	{"fallback", sf::Color(255, 255, 255)} 
};

std::unordered_map<std::string, int> color_offsets_by_type = {
	{"air", 0},
	{"sand", 25},
	{"rock", 5},
	{"water", 15},
	{"ice", 15},
	{"steam", 5},
	{"lava", 25},
	{"molten glass", 20},
	{"glass", 5},
	{"fallback", 0}
};

std::unordered_map<std::string, float> density_by_type = {
	{"air", 0.0f},
	{"steam", 0.0f},
	{"water", 0.1f},
	{"molten glass", 0.2f},
	{"lava", 0.3f},
	{"sand", 1.1f},
	{"rock", 4.0f},
	{"ice", 4.0f},
	{"glass", 4.0f}
};

std::string round_to_str(float value, int precision = 2) {
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(precision) << value;
	return stream.str();
}

class Particle {
public:
	float temp;
	float density; // helps to reduce the size of if starements
	/* Density guide:
		4 - anything that cant be fallen trough
		0 - anything that falls will fall through this particle
		0 - 1 liquids
		1 - 2 powders
	*/
	std::string type;
	std::string state;

	int state_change_cooldown = 5; // When it hit's 0 the particle can change state

	sf::Color color;

	Particle() {
		temp = 20.0f;
		type = "air";
		state = "special";
		color = color_by_type["air"];
		density = 0;
	}
};

class ParticleSimulation {
private:
	int simulation_height;
	int simulation_width;

	std::vector<std::vector<Particle>> particle_layers;
	std::vector<std::vector<Particle>> pending_particle_layers;

	std::string particle_types[9] = { "air", "sand", "rock", "water", "ice", "steam", "lava", "molten glass", "glass" };
	std::string particle_states[5] = { "special", "solid", "liquid", "gas", "powder" };

	int cell_px = 10;
	int gap = 1;

	std::mt19937 gen;

public:
	std::unordered_map<std::string, bool> discovered_particles;
	int particle_count = 0;

    ParticleSimulation(int height, int width) {
        simulation_height = height;
        simulation_width = width;

        std::vector<Particle> temporary_buffer;

        for (int i = 0; i < simulation_height; i++) {
            temporary_buffer.clear();
            for (int i = 0; i < simulation_width; i++) {
                temporary_buffer.emplace_back(Particle());
            }
            particle_layers.push_back(temporary_buffer);
        }

        pending_particle_layers = particle_layers;

        std::random_device rd;
        gen = std::mt19937(rd());
    }

	std::vector<std::pair<int, int>> get_surroundings(int x, int y) const {
		static const int offsets[8][2] = {
			{-1, -1}, {0, -1}, {1, -1},
			{-1,  0},          {1,  0},
			{-1,  1}, {0,  1}, {1,  1}
		};

		std::vector<std::pair<int, int>> neighbors;
		for (int i = 0; i < 8; ++i) {
			int nx = x + offsets[i][0];
			int ny = y + offsets[i][1];
			if (nx >= 0 && nx < simulation_width && ny >= 0 && ny < simulation_height) {
				neighbors.emplace_back(nx, ny);
			}
		}
		return neighbors;
	}

	void swap(int i, int j, int tx, int ty) {
		Particle particle = pending_particle_layers[i][j];

		pending_particle_layers[i][j] = particle_layers[tx][ty];

		pending_particle_layers[tx][ty] = particle;
	}

	void update() {
		// Copy current state to pending layers
		pending_particle_layers = particle_layers;
		particle_count = 0;

		for (int j = 0; j < simulation_height; j++) {
			for (int i = 0; i < simulation_width; i++) {
				// -- Update temp -- //

				Particle particle = particle_layers[i][j];

				std::pair<int, int> coordinate = {i, j};

				auto surroundings = get_surroundings(i, j);

				float temp_transfer = 0.1f;

				if (particle.type == "air") {
					temp_transfer = 0.025f;
				}

				float delta = 0.0f;

				for (const auto& coords : surroundings) {
					float extern_temp = particle_layers[coords.first][coords.second].temp;
					std::string extern_type = particle_layers[coords.first][coords.second].type;

					if (extern_type == "air") {
						temp_transfer = 0.0125f;
					}

					delta += temp_transfer * (extern_temp - particle.temp);
				}

				pending_particle_layers[i][j].temp = particle.temp + delta;

				pending_particle_layers[i][j].temp = std::clamp(pending_particle_layers[i][j].temp, -273.0f, 5000.0f);

				if (particle.type == "air") {
					continue;
				}

				particle_count++;

				// -- state based movement -- //

				std::vector<std::pair<int, int>> bottom_neighbors;
				std::vector<std::pair<int, int>> top_neighbors;
				std::vector<std::pair<int, int>> side_neighbors;

				for (int x = 0; x < surroundings.size(); x++) {
					if (surroundings[x].second > j &&
						(particle_layers[surroundings[x].first][surroundings[x].second].type == "air" ||
						particle_layers[surroundings[x].first][surroundings[x].second].state == "gas" ||
						particle_layers[surroundings[x].first][surroundings[x].second].state == "liquid")) {
						
						bottom_neighbors.emplace_back(surroundings[x].first, surroundings[x].second);
					}
					if (surroundings[x].second < j && particle_layers[surroundings[x].first][surroundings[x].second].type == "air") {

						top_neighbors.emplace_back(surroundings[x].first, surroundings[x].second);
					}
					if (surroundings[x].second == j && particle_layers[surroundings[x].first][surroundings[x].second].density == 0.0f) {

						side_neighbors.emplace_back(surroundings[x].first, surroundings[x].second);
					}
				}

				// move particle based on state
				if (particle.state != "solid") {
					bool moved = false;
					if (!bottom_neighbors.empty() && particle.state != "gas") {
						std::uniform_int_distribution<> dist(0, bottom_neighbors.size() - 1);
						std::pair<int, int> other_particle = bottom_neighbors[dist(gen)];
						int tx = other_particle.first;
						int ty = other_particle.second;

						// Only move if the target is air or liquid or gas in both current and pending layers // this if statement is janky
						if (particle.density > particle_layers[tx][ty].density and particle.density > pending_particle_layers[tx][ty].density) {

							swap(i, j, tx, ty);
							coordinate = { tx, ty };
							moved = true;
							pending_particle_layers[tx][ty].state_change_cooldown = particle_layers[i][j].state_change_cooldown;
						}
					}
					if (!top_neighbors.empty() && particle.state == "gas" && !moved) {
						std::uniform_int_distribution<> dist(0, top_neighbors.size() - 1);
						std::pair<int, int> particle = top_neighbors[dist(gen)];
						int tx = particle.first;
						int ty = particle.second;

						// Only move if the target is air or in both current and pending layers
						if (particle_layers[tx][ty].type == "air" && pending_particle_layers[tx][ty].type == "air") {
							swap(i, j, tx, ty);
							coordinate = { tx, ty };
							pending_particle_layers[tx][ty].state_change_cooldown = particle_layers[i][j].state_change_cooldown;
						}
					}
					if (!side_neighbors.empty() && (particle.state == "liquid" || particle.state == "gas") && !moved) {
						std::uniform_int_distribution<> dist(0, side_neighbors.size() - 1);
						std::pair<int, int> particle = side_neighbors[dist(gen)];
						int tx = particle.first;
						int ty = particle.second;

						if (particle_layers[tx][ty].type == "air" && pending_particle_layers[tx][ty].type == "air") {
							swap(i, j, tx, ty);
							coordinate = { tx, ty };
							pending_particle_layers[tx][ty].state_change_cooldown = particle_layers[i][j].state_change_cooldown;
						}
					}
				}

				// -- update state by temp -- //

				int tx = coordinate.first;
				int ty = coordinate.second;
				particle = pending_particle_layers[tx][ty];
				std::string type = particle.type;

				if (pending_particle_layers[tx][ty].state_change_cooldown <= 0) {
					if (particle.type == "sand") {
						if (particle.temp > 1700) {
							pending_particle_layers[tx][ty].state = "liquid";
							pending_particle_layers[tx][ty].type = "molten glass";
							discovered_particles["molten glass"] = true;
						}
					}
					else if (particle.type == "rock") {
						if (particle.temp >= 1200) {
							pending_particle_layers[tx][ty].state = "liquid";
							pending_particle_layers[tx][ty].type = "lava";
							discovered_particles["lava"] = true;
						}
					}
					else if (particle.type == "water") {
						if (particle.temp <= 0) {
							pending_particle_layers[tx][ty].state = "solid";
							pending_particle_layers[tx][ty].type = "ice";
							discovered_particles["ice"] = true;
						}
						else if (particle.temp >= 100) {
							pending_particle_layers[tx][ty].state = "gas";
							pending_particle_layers[tx][ty].type = "steam";
							discovered_particles["steam"] = true;
						}
					}
					else if (particle.type == "ice") {
						if (particle.temp > 0) {
							pending_particle_layers[tx][ty].state = "liquid";
							pending_particle_layers[tx][ty].type = "water";
						}
					}
					else if (particle.type == "steam") {
						if (particle.temp < 100) {
							pending_particle_layers[tx][ty].state = "liquid";
							pending_particle_layers[tx][ty].type = "water";
						}
					}
					else if (particle.type == "lava") {
						if (particle.temp < 1000) {
							pending_particle_layers[tx][ty].state = "solid";
							pending_particle_layers[tx][ty].type = "rock";
						}
					}
					else if (particle.type == "molten glass") {
						if (particle.temp < 1500) {
							pending_particle_layers[tx][ty].state = "solid";
							pending_particle_layers[tx][ty].type = "glass";
							discovered_particles["glass"] = true;
						}
					}
					else if (particle.type == "glass") {
						if (particle.temp > 1200) {
							pending_particle_layers[tx][ty].state = "liquid";
							pending_particle_layers[tx][ty].type = "molten glass";
							discovered_particles["molten glass"] = true;
						}
					}
				}

				if (pending_particle_layers[tx][ty].type != type) { // update color and cooldown
					pending_particle_layers[tx][ty].state_change_cooldown = 5; // reset cooldown

					type = pending_particle_layers[tx][ty].type;

					sf::Color color = color_by_type[type];
					std::uniform_int_distribution<int> color_dist(color_offsets_by_type[type] * -1, color_offsets_by_type[type]);

					int r = std::clamp(static_cast<int>(color.r) + color_dist(gen), 0, 255);
					int g = std::clamp(static_cast<int>(color.g) + color_dist(gen), 0, 255);
					int b = std::clamp(static_cast<int>(color.b) + color_dist(gen), 0, 255);
					int a = color.a;

					pending_particle_layers[tx][ty].color = sf::Color(r, g, b, a);
				}
				else {
					if (!(particle_layers[tx][ty].state_change_cooldown <= 0)) {
						pending_particle_layers[tx][ty].state_change_cooldown -= 1;
						pending_particle_layers[tx][ty].state_change_cooldown = std::max(0, pending_particle_layers[tx][ty].state_change_cooldown);
					}
				}
			}
		}

		particle_layers = pending_particle_layers;
	}

	void brush(int brush_size, int mouse_x, int mouse_y, std::string draw_type, float power = 1) {
		int cell_stride = cell_px + gap;

		if (mouse_x < 0 || mouse_y < 0 || mouse_x > simulation_width * cell_stride || mouse_y > simulation_height * cell_stride) {
			return;
		}

		int grid_x = mouse_x / cell_stride;
		int grid_y = mouse_y / cell_stride;

		int half = brush_size / 2;

		std::string state = "";
		std::string type = draw_type;
		float density = 0.0f;

		if (draw_type == "air") {
			state = "special";
		}
		else if (draw_type == "sand") {
			state = "powder";
		}
		else if (draw_type == "rock") {
			state = "solid";
		}
		else if (draw_type == "water") {
			state = "liquid";
		}
		else if (draw_type == "ice") {
			state = "solid";
		}
		else if (draw_type == "steam") {
			state = "gas";
		}
		else if (draw_type == "glass") {
			state = "solid";
		}
		else if (draw_type == "none") {
			state = "special";
			type = "air";
		}

		density = density_by_type[type];

		sf::Color color = color_by_type[type];

		std::uniform_int_distribution<int> color_dist(color_offsets_by_type[type] * -1, color_offsets_by_type[type]);

		for (int i = -half; i <= half; ++i) {
			for (int j = -half; j <= half; ++j) {
				int x = grid_x + i;
				int y = grid_y + j;

				if (x < 0 || x >= simulation_width || y < 0 || y >= simulation_height) {
					continue;
				}

				float temp = particle_layers[x][y].temp;

				if (draw_type == "heat") {
					temp += 10.0f * power;
				}
				else if (draw_type == "cool") {
					temp -= 10.0f * power;
				}
				else if (x >= 0 && x < simulation_width && y >= 0 && y < simulation_height && (particle_layers[x][y].type == "air" || draw_type == "none")) {
					particle_layers[x][y].temp = 20.0;
					particle_layers[x][y].state = state;
					particle_layers[x][y].type = type;
					particle_layers[x][y].density = density;

					if (draw_type == "none") {
						particle_layers[x][y].color = color;
						continue;
					}

					int r = std::clamp(static_cast<int>(color.r) + color_dist(gen), 0, 255);
					int g = std::clamp(static_cast<int>(color.g) + color_dist(gen), 0, 255);
					int b = std::clamp(static_cast<int>(color.b) + color_dist(gen), 0, 255);
					int a = color.a;
					particle_layers[x][y].color = sf::Color(r, g, b, a);
				}

				temp = std::clamp(temp, -273.0f, 5000.0f);
				particle_layers[x][y].temp = temp;
			}
		}
	}

	void draw_sfml(sf::RenderWindow& window, bool use_temp_coloring = false, int cell_size = 5) {
		const int grid_size_x = simulation_width;
		const int grid_size_y = simulation_height;

		sf::RectangleShape cell(sf::Vector2f(
			static_cast<float>(cell_px),
			static_cast<float>(cell_px)
		));

		for (int i = 0; i < grid_size_x; ++i) {
			for (int j = 0; j < grid_size_y; ++j) {
				sf::Color color;
				if (use_temp_coloring) {
					float t = particle_layers[i][j].temp;

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
				else {
					color = particle_layers[i][j].color;
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

	void draw_brush_outline_sfml(sf::RenderWindow& window, int brush_size, int mouse_x, int mouse_y) {
		int cell_stride = cell_px + gap;

		if (mouse_x < 0 || mouse_y < 0 || mouse_x > simulation_width * cell_stride || mouse_y > simulation_height * cell_stride) {
			return;
		}

		int grid_x = mouse_x / cell_stride;
		int grid_y = mouse_y / cell_stride;
		int half = brush_size / 2;

		int min_x = std::max(0, grid_x - half);
		int min_y = std::max(0, grid_y - half);
		int max_x = std::min(simulation_width - 1, grid_x + half);
		int max_y = std::min(simulation_height - 1, grid_y + half);

		float left = min_x * cell_stride;
		float top = min_y * cell_stride;
		float width = (max_x - min_x + 1) * cell_stride - gap;
		float height = (max_y - min_y + 1) * cell_stride - gap;

		sf::RectangleShape outline(sf::Vector2f(width, height));
		outline.setPosition(sf::Vector2f(static_cast<float>(left),static_cast<float>(top)));
		outline.setFillColor(sf::Color::Transparent);
		outline.setOutlineColor(sf::Color::Red);
		outline.setOutlineThickness(2.f);

		window.draw(outline);
	}

	void draw_particle_information_sfml(sf::RenderWindow& window, int mouse_x, int mouse_y) {
		int cell_stride = cell_px + gap;

		if (mouse_x <= 0 || mouse_y <= 0 || mouse_x >= simulation_width * cell_stride || mouse_y >= simulation_height * cell_stride) {
			return;
		}

		try {
			int mouse_x_grid = mouse_x / cell_stride;
			int mouse_y_grid = mouse_y / cell_stride;

			Particle particle = particle_layers[mouse_x_grid][mouse_y_grid];

 			sf::Font arial("ARIAL.TTF");
			sf::Text info(arial, particle.type + "\nTemp: " + round_to_str(particle.temp) + "\nState: " + particle.state, 15U);
			info.setPosition(sf::Vector2f(550, 590));
			window.draw(info);
		}
		catch (...) { // not sure what the error is but this stops it
			return;
		}
	}
};