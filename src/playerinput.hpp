#pragma once

#include <functional>
#include <set>
#include <chrono>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/scalar_constants.hpp>

class PlayerInput {
	static float noop() { return 0; }

public:
	using GetF = std::function<float()>;

	// look directions
	glm::vec3 front();
	glm::vec3 right();
	glm::vec3 up();

	glm::vec3 local_move_dir(); // move dir without respect to look direction

	// move dir with respect to look direction
	glm::vec3 move_front();
	glm::vec3 move_right();
	glm::vec3 move_up();

	// input
	GetF moving_left = noop;
	GetF moving_right = noop;
	GetF moving_forward = noop;
	GetF moving_backward = noop;
	GetF jumping = noop;
	GetF shooting = noop;
	GetF mouse_x = noop;
	GetF mouse_y = noop;

	static void jid_callback(int jid, int event);

	static PlayerInput createKeyboardInput(GLFWwindow* window);
	static PlayerInput createGamepadInput(int index);

private:
	static std::set<int> jids;
};
