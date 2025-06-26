#pragma once

#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include <utility>
#include <cstring>
#include <random>

class ParticleSimulation {
private:
	static const int HEIGHT = 50;
	static const int WIDTH = 50;
	float particle_layers[3][WIDTH][HEIGHT];
	float pending_particle_layers[3][WIDTH][HEIGHT];

	const int CELL_PX = 10;
	const int GAP = 1;
public:
	ParticleSimulation() {
		for (int i = 0; i < WIDTH; i++) {
			for (int j = 0; j < HEIGHT; j++) {
				float temp = 20.0;

				particle_layers[0][i][j] = temp; // room temp
				particle_layers[1][i][j] = 0; // background state
				particle_layers[2][i][j] = 0; // air
				pending_particle_layers[0][i][j] = temp; // room temp
				pending_particle_layers[1][i][j] = 0; // background state
				pending_particle_layers[2][i][j] = 0; // air
			}
		}
	}

	std::vector<std::pair<int, int>> get_surroundings(int x, int y) {
		static const int offsets[8][2] = {
			{-1, -1}, {0, -1}, {1, -1},
			{-1,  0},          {1,  0},
			{-1,  1}, {0,  1}, {1,  1}
		};

		std::vector<std::pair<int, int>> neighbors;
		for (int i = 0; i < 8; ++i) {
			int nx = x + offsets[i][0];
			int ny = y + offsets[i][1];
			if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
				neighbors.emplace_back(nx, ny);
			}
		}
		return neighbors;
	}

	void swap(int i, int j, int tx, int ty) {
		float temp = particle_layers[0][i][j];
		int state = particle_layers[1][i][j];
		int type = particle_layers[2][i][j];

		pending_particle_layers[0][i][j] = particle_layers[0][tx][ty];
		pending_particle_layers[1][i][j] = particle_layers[1][tx][ty];
		pending_particle_layers[2][i][j] = particle_layers[2][tx][ty];

		pending_particle_layers[0][tx][ty] = temp;
		pending_particle_layers[1][tx][ty] = state;
		pending_particle_layers[2][tx][ty] = type;
	}

	void update() {
		// Copy current state to pending layers
		std::memcpy(pending_particle_layers, particle_layers, sizeof(particle_layers));

		for (int i = 0; i < WIDTH; i++) {
			for (int j = 0; j < HEIGHT; j++) {
				// -- Update temp -- //
				float temp = particle_layers[0][i][j];
				int state = particle_layers[1][i][j];
				int type = particle_layers[2][i][j];

				std::pair<int, int> coordinate = {i, j};

				auto surroundings = get_surroundings(i, j);

				float temp_transfer = 0.1f;

				if (type == 0) {
					temp_transfer = 0.05f;
				}

				float delta = 0.0f;

				for (const auto& coords : surroundings) {
					float extern_temp = particle_layers[0][coords.first][coords.second];
					int extern_type = particle_layers[2][coords.first][coords.second];

					if (extern_type == 0) {
						temp_transfer = 0.025f;
					}

					delta += temp_transfer * (extern_temp - temp);
				}
				pending_particle_layers[0][i][j] = temp + delta;

				// -- state based movement -- //
				temp += delta;

				std::vector<std::pair<int, int>> bottom_neighbors;
				std::vector<std::pair<int, int>> top_neighbors;
				std::vector<std::pair<int, int>> side_neighbors;

				for (int x = 0; x < surroundings.size(); x++) {
					// Only add as a bottom neighbor if it's within bounds and is air or liquid
					if (surroundings[x].second > j &&
						(particle_layers[2][surroundings[x].first][surroundings[x].second] == 0 ||
							(particle_layers[1][surroundings[x].first][surroundings[x].second] == 2 && state == 4))
						&& surroundings[x].second < HEIGHT) {
						bottom_neighbors.emplace_back(surroundings[x].first, surroundings[x].second);
					}
					if (surroundings[x].second < j &&
						(particle_layers[2][surroundings[x].first][surroundings[x].second] == 0)) {
						top_neighbors.emplace_back(surroundings[x].first, surroundings[x].second);
					}
					if (surroundings[x].second == j && (particle_layers[2][surroundings[x].first][surroundings[x].second] == 0 ||
						particle_layers[1][surroundings[x].first][surroundings[x].second] == 2 && state == 4)) {
						side_neighbors.emplace_back(surroundings[x].first, surroundings[x].second);
					}
				}

				std::random_device rd;
				std::mt19937 gen(rd());

				if (state == 2 || state == 3 || state == 4) { // liquid, gas, and powder
					if (!bottom_neighbors.empty() && state != 3) {
						std::uniform_int_distribution<> dist(0, bottom_neighbors.size() - 1);
						std::pair<int, int> particle = bottom_neighbors[dist(gen)];
						int tx = particle.first;
						int ty = particle.second;

						// Only move if the target is air or liquid in both current and pending layers
						if ((particle_layers[2][tx][ty] == 0 && pending_particle_layers[2][tx][ty] == 0) ||
							particle_layers[1][tx][ty] == 2 && pending_particle_layers[1][tx][ty] == 2 && state == 4) {
							// exchange position
							swap(i, j, tx, ty);
							coordinate = { tx, ty };
						}
					}
					else if (!top_neighbors.empty() && state == 3) {
						std::uniform_int_distribution<> dist(0, top_neighbors.size() - 1);
						std::pair<int, int> particle = top_neighbors[dist(gen)];
						int tx = particle.first;
						int ty = particle.second;

						// Only move if the target is air or in both current and pending layers
						if (particle_layers[2][tx][ty] == 0 && pending_particle_layers[2][tx][ty] == 0) {
							// exchange position
							swap(i, j, tx, ty);
							coordinate = { tx, ty };
						}
					}
					else if (!side_neighbors.empty() && (state == 2 || state == 3)) {
						std::uniform_int_distribution<> dist(0, side_neighbors.size() - 1);
						std::pair<int, int> particle = side_neighbors[dist(gen)];
						int tx = particle.first;
						int ty = particle.second;

						if (particle_layers[2][tx][ty] == 0 && pending_particle_layers[2][tx][ty] == 0) {
							swap(i, j, tx, ty);
							coordinate = { tx, ty };
						}
					}
				}

				// -- update state by temprature -- //

				int tx = coordinate.first;
				int ty = coordinate.second;

				if (type == 3) { // Water
					if (temp <= 0) { // freezing temp in c
						pending_particle_layers[1][tx][ty] = 1; // solid
						pending_particle_layers[2][tx][ty] = 4; // ice
					}
					else if (temp >= 100) { // boiling temp in c
						pending_particle_layers[1][tx][ty] = 3; // gas
						pending_particle_layers[2][tx][ty] = 5; // steam
					}
				}
				else if (type == 4) { // ice
					if (temp > 0) { // freezing temp in c
						pending_particle_layers[1][tx][ty] = 2; // liquid
						pending_particle_layers[2][tx][ty] = 3; // water
					}
				}
				else if (type == 5) { // steam
					if (temp < 100) { // boiling temp in c
						pending_particle_layers[1][tx][ty] = 2; // liquid
						pending_particle_layers[2][tx][ty] = 3; // water
					}
				}
				else if (type == 2) { // Rock
					if (temp >= 250) { // rock melting point
						pending_particle_layers[1][tx][ty] = 2; // liquid
						pending_particle_layers[2][tx][ty] = 6; // lava
					}
				}
				else if (type == 6) { // lava
					if (temp < 100) { // rock melting point
						pending_particle_layers[1][tx][ty] = 1; // solid
						pending_particle_layers[2][tx][ty] = 2; // rock
					}
				}
			}
		}

		std::memcpy(particle_layers, pending_particle_layers, sizeof(particle_layers));
	}

	void draw(bool debug = true) {
		if (debug) {
			for (int i = 0; i < WIDTH; i++) {
				for (int j = 0; j < HEIGHT; j++) {
					std::cout << particle_layers[0][i][j] << " ";
				}
				std::cout << "\n";
			}
		}
	}

	void draw_sfml(sf::RenderWindow& window, bool use_temp_coloring = false, int cell_size = 5) {
		const int grid_size_x = WIDTH;
		const int grid_size_y = HEIGHT;
		const int cell_px = CELL_PX;
		const int gap = GAP;

		sf::RectangleShape cell(sf::Vector2f(
			static_cast<float>(cell_px),
			static_cast<float>(cell_px)
		));

		for (int i = 0; i < grid_size_x; ++i) {
			for (int j = 0; j < grid_size_y; ++j) {
				sf::Color color;
				if (use_temp_coloring) {
					float temp = particle_layers[0][i][j];
					float t = std::max(0.0f, std::min(100.0f, temp));
					color = sf::Color(
						static_cast<unsigned char>(255 * (t / 100.0f)),
						0,
						static_cast<unsigned char>(255 * (1.0f - t / 100.0f))
					);
				}
				else {
					int type = static_cast<int>(particle_layers[2][i][j]);
					switch (type) {
					case 0: // air
						color = sf::Color(0, 0, 0, 0); // transparent
						break;
					case 1: // sand
						color = sf::Color(255, 255, 0); // yellow
						break;
					case 2: // rock
						color = sf::Color(128, 128, 128); // gray
						break;
					case 3: // water
						color = sf::Color(0, 128, 255); // blue
						break;
					case 4: // ice
						color = sf::Color(161, 236, 255); // light blue
						break;
					case 5: // Steam
						color = sf::Color(200, 200, 200); // light transparent gray
						break;
					case 6: // Lava
						color = sf::Color(255, 115, 0);
						break;
					default:
						color = sf::Color(0, 0, 0, 0); // fallback transparent
						break;
					}
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

	void brush(int brush_size, int mouse_x, int mouse_y, std::string draw_type) {
		const int cell_px = CELL_PX;
		const int gap = GAP;
		int cell_stride = cell_px + gap;

		int state = 0;
		int type = 0;

		if (draw_type == "air") {
			state = 0;
			type = 0;
		}
		else if (draw_type == "sand") {
			state = 4;
			type = 1;
		}
		else if (draw_type == "rock") {
			state = 1;
			type = 2;
		}
		else if (draw_type == "water") {
			state = 2;
			type = 3;
		}
		else if (draw_type == "ice") {
			state = 1;
			type = 4;
		}
		else if (draw_type == "steam") {
			state = 3;
			type = 5;
		}
		else if (draw_type == "none") {
			state = 0;
			type = 0;
		}

		int grid_x = mouse_x / cell_stride;
		int grid_y = mouse_y / cell_stride;

		int half = brush_size / 2;

		for (int i = -half; i <= half; ++i) {
			for (int j = -half; j <= half; ++j) {
				int x = grid_x + i;
				int y = grid_y + j;
				if (draw_type == "heat" && x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
					particle_layers[0][x][y] += 10;
				}
				else if (draw_type == "cool" && x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
					particle_layers[0][x][y] -= 10;
				}
				else if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && (particle_layers[2][x][y] == 0 || draw_type == "none")) {
					particle_layers[0][x][y] = 20.0;
					particle_layers[1][x][y] = state;
					particle_layers[2][x][y] = type;
				}
			}
		}
	}

	void draw_brush_outline_sfml(sf::RenderWindow& window, int brush_size, int mouse_x, int mouse_y) {
		const int cell_px = CELL_PX;
		const int gap = GAP;
		int cell_stride = cell_px + gap;

		int grid_x = mouse_x / cell_stride;
		int grid_y = mouse_y / cell_stride;
		int half = brush_size / 2;

		int min_x = std::max(0, grid_x - half);
		int min_y = std::max(0, grid_y - half);
		int max_x = std::min(WIDTH - 1, grid_x + half);
		int max_y = std::min(HEIGHT - 1, grid_y + half);

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
};