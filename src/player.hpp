#pragma once

#include <chrono>
#include "playerinput.hpp"

class Player {
public:
	void update(std::chrono::milliseconds delta_time);

	glm::vec3 m_pos;
	glm::vec3 m_dir;
	glm::vec3 m_vel;
	PlayerInput m_input;
};
