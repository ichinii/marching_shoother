#include "player.hpp"
#include <glm/gtc/matrix_transform.hpp>

void Player::update(std::chrono::milliseconds delta_time)
{
	m_dir = m_input.front();
	auto local_move_dir = m_input.local_move_dir();
	m_vel = local_move_dir.x * m_input.move_right()
		+ local_move_dir.y * m_input.move_up()
		- local_move_dir.z * m_input.move_front();
	m_vel *= .05f;
	m_pos += m_vel;

	if (m_input.shooting()) {

	}
}
