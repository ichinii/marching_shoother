#include "playerinput.hpp"
#include <iostream>

glm::vec3 PlayerInput::front()
{
	auto mouse_pos = glm::vec2(mouse_x(), mouse_y());
	return glm::normalize(
		glm::vec3(glm::sin(mouse_pos.x) * glm::cos(mouse_pos.y),
		glm::sin(mouse_pos.y),
		glm::cos(mouse_pos.x) * glm::cos(mouse_pos.y))
	);
}

glm::vec3 PlayerInput::right()
{
	return glm::normalize(
		glm::cross(
			front(),
			glm::vec3(0, 1, 0)
		)
	);
}

glm::vec3 PlayerInput::up()
{
	return glm::normalize(
		glm::cross(
			right(),
			front()
		)
	);
}

glm::vec3 PlayerInput::local_move_dir()
{
	return glm::vec3(
		moving_right() - moving_left(),
		0,
		moving_backward() - moving_forward()
	);
}

glm::vec3 PlayerInput::move_front()
{
	auto dir = front();
	return glm::normalize(glm::vec3(dir.x, 0, dir.z));
}

glm::vec3 PlayerInput::move_right()
{
	auto dir = right();
	return glm::normalize(glm::vec3(dir.x, 0, dir.z));
}

glm::vec3 PlayerInput::move_up()
{
	return glm::vec3(0, 1, 0);
}

std::set<int> PlayerInput::jids;
void PlayerInput::jid_callback(int jid, int event)
{
	if (event == GLFW_CONNECTED) {
		jids.insert(jid);
		std::cout << "joystick connected" << std::endl;
	}
	else if (event == GLFW_DISCONNECTED) {
		jids.erase(jid);
		std::cout << "joystick disconnected" << std::endl;
	}

	std::cout << "with name: " << glfwGetJoystickName(jid) << std::endl;
	if (glfwJoystickIsGamepad(jid)) {
		std::cout << "is a gamepad with name: " << glfwGetGamepadName(jid) << std::endl;
	}
}

PlayerInput PlayerInput::createKeyboardInput(GLFWwindow* window)
{
	return PlayerInput{
		.moving_left = [=] { return glfwGetKey(window, GLFW_KEY_A) ? 1.f : 0.f; },
		.moving_right = [=] { return glfwGetKey(window, GLFW_KEY_D) ? 1.f : 0.f; },
		.moving_forward = [=] { return glfwGetKey(window, GLFW_KEY_W) ? 1.f : 0.f; },
		.moving_backward = [=] { return glfwGetKey(window, GLFW_KEY_S) ? 1.f : 0.f; },
		.jumping = [=] { return glfwGetKey(window, GLFW_KEY_SPACE) ? 1.f : 0.f; },
		.shooting = [=] { return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) ? 1.f : 0.f; },
		.mouse_x = [=] {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			auto m = -glm::pi<float>() * (x - w * .5) / h;
			return static_cast<float>(m);
		},
		.mouse_y= [=] {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			auto m = -glm::pi<float>() * (y - h * .5) / h;
			return static_cast<float>(m);
		},
	};
}

PlayerInput PlayerInput::createGamepadInput(int index)
{
	enum {
		left = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
		right = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
		forward = GLFW_GAMEPAD_BUTTON_DPAD_UP,
		backward = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
		jump = GLFW_GAMEPAD_BUTTON_A,
		fire = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
		axis_left_x = GLFW_GAMEPAD_AXIS_LEFT_X,
		axis_left_y = GLFW_GAMEPAD_AXIS_LEFT_Y,
		axis_right_x = GLFW_GAMEPAD_AXIS_RIGHT_X,
		axis_right_y = GLFW_GAMEPAD_AXIS_RIGHT_Y,
	};

	auto jid = [&] (int index) {
		int count = 0;
		for (int jid : jids)
			if (glfwJoystickIsGamepad(jid))
				if (index == count++)
					return jid;
		return -1;
	};

	auto button = [&] (int id, int jid) {
		if (jid == -1)
			return 0;
		GLFWgamepadstate state;
		glfwGetGamepadState(jid, &state);
		return static_cast<int>(state.buttons[id]);
	};

	auto axis = [&] (int id, int jid) {
		if (jid == -1)
			return 0.f;
		GLFWgamepadstate state;
		glfwGetGamepadState(jid, &state);
		return static_cast<float>(state.axes[id]);
	};

	auto smoothstep = [] (float a, float b, float v) {
		return glm::clamp((v - a) / (b - a), 0.f, 1.f);
	};

	auto abs_smoothstep = [=] (float a, float b, float v) {
		return glm::sign(v) * smoothstep(a, b, glm::abs(v));
	};

	auto mouse = glm::vec2(0);
	auto axis_threshold_low = .2f;
	auto axis_threshold_high = .9f;

	auto axis_smoothstep = [=] (float v) {
		return smoothstep(axis_threshold_low, axis_threshold_high, v);
	};

	auto axis_abs_smoothstep = [=] (float v) {
		return abs_smoothstep(axis_threshold_low, axis_threshold_high, v);
	};

	return PlayerInput{
		.moving_left = [=] {
			return button(left, jid(index)) ? 1.f :
				axis_smoothstep(-axis(axis_left_x, jid(index)));
		},
		.moving_right = [=] {
			return button(right, jid(index)) ? 1.f :
				axis_smoothstep(axis(axis_left_x, jid(index)));
		},
		.moving_forward = [=] {
			return button(forward, jid(index)) ? 1.f :
				axis_smoothstep(-axis(axis_left_y, jid(index)));
		},
		.moving_backward = [=] {
			return button(backward, jid(index)) ? 1.f :
				axis_smoothstep(axis(axis_left_y, jid(index)));
		},
		.jumping = [=] { return button(jump, jid(index)) ? 1.f : 0.f; },
		.shooting = [=] { return button(jump, jid(index)) ? 1.f : 0.f; },
		.mouse_x = [=] () mutable {
			mouse.x += .005f * axis_abs_smoothstep(axis(axis_right_x, jid(index)));
			auto m = -glm::pi<float>() * mouse.x;
			return static_cast<float>(m);
		},
		.mouse_y = [=] () mutable {
			mouse.y += .005f * axis_abs_smoothstep(axis(axis_right_y, jid(index)));
			mouse.y = glm::clamp(mouse.y, -.45f, .45f);
			auto m = -glm::pi<float>() * mouse.y;
			return static_cast<float>(m);
		},
	};
};
