#pragma once

#include <SFML/Graphics/RenderTarget.hpp>

#include <SFML/System/Vector2.hpp>

#include <vector>
#include <string>

class MenuObject {
public:
	virtual void draw(sf::RenderTarget& target) = 0;

	virtual void update(sf::Vector2i mouse_pos) = 0;

	std::string id;

private:
	sf::Vector2f position;


};

class Button : public MenuObject {
public:
	void draw(sf::RenderTarget& target) override;

	void update(sf::Vector2i mouse_position) override;

private:
	bool pressed;

};

class Menu {
	std::vector<MenuObject> objects;
};