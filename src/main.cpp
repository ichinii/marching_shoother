#include <iostream>
#include <numeric>
#include <algorithm>
#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <chrono>
#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader.hpp"
#include "watcher.hpp"
#include "player.hpp"
#include "misc.hpp"

using namespace std::chrono_literals;

int main()
{
	std::srand(std::time(0));

	if (!glfwInit()) return 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	auto window_size = glm::ivec2{1280, 720};
	auto window = glfwCreateWindow(window_size.x, window_size.y, "glsl", nullptr, nullptr);

	// keyboard
	glfwSetKeyCallback(window, [] (GLFWwindow* window, int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, int mods) {
		if (!mods && key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, true);
	});

	// joysticks
	glfwSetJoystickCallback(PlayerInput::jid_callback);
	for (auto i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; ++i)
		if (glfwJoystickPresent(i))
			PlayerInput::jid_callback(i, GLFW_CONNECTED);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	if (glewInit() != GLEW_OK) return 0;
	glClearColor(.2, .1, 0, 1);

	// std::this_thread::sleep_for(1s);
	auto display_program = createProgram({{GL_VERTEX_SHADER, "res/vertex.glsl"}, {GL_FRAGMENT_SHADER, "res/fragment.glsl"}});
	auto compute_program = createProgram({{GL_COMPUTE_SHADER, "res/compute.glsl"}});
	glUseProgram(compute_program);
	auto compute_shader_watcher = Watcher("res/compute.glsl", [&] () {
		if (compute_program > 0)
			glDeleteProgram(compute_program);
		compute_program = createProgram({{GL_COMPUTE_SHADER, "res/compute.glsl"}});
	});

	auto frame_tex_size = glm::uvec2(window_size);
	GLuint frame_tex_out;
	glGenTextures(1, &frame_tex_out);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, frame_tex_out);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, frame_tex_size.x, frame_tex_size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindImageTexture(0, frame_tex_out, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glUseProgram(display_program);
	enum { vertex_position, vertex_uv };
	GLuint vao;
	GLuint vbos[2];
	glCreateVertexArrays(1, &vao);
	glGenBuffers(2, vbos);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(vertex_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[vertex_position]);
	glm::vec2 vertex_positions[] = { {-1, -1}, {1, -1}, {1, 1}, {-1, -1}, {1, 1}, {-1, 1} };
	glBufferData(GL_ARRAY_BUFFER, sizeof (vertex_positions), vertex_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(vertex_position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(vertex_uv);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[vertex_uv]);
	glm::vec2 vertex_uvs[] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
	glBufferData(GL_ARRAY_BUFFER, sizeof (vertex_uvs), vertex_uvs, GL_STATIC_DRAW);
	glVertexAttribPointer(vertex_uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindVertexArray(0);
	glUseProgram(0);

	auto players = std::vector<Player>();
	auto player_index = 0ul;
	std::generate_n(std::back_inserter(players), 1, [&] () {
		auto player = Player{
			.m_pos = {0, 0, (player_index - .5f) * 4.f},
			.m_dir = glm::normalize(glm::vec3{0, 0, -glm::sign(player_index - .5)}),
			.m_vel = {0, 0, 0},
			.m_input = player_index == 1 ?
				PlayerInput::createKeyboardInput(window) :
				PlayerInput::createGamepadInput(0)
		};
		++player_index;
		return player;
	});

	using clock = std::chrono::steady_clock;
	auto start_time = clock::now();
	auto elapsed_time = 0ms;
	auto fps_print_time = elapsed_time;

	auto frames = 0ul;
	while (!glfwWindowShouldClose(window)) {
		auto cur_time = clock::now();
		auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - (start_time + elapsed_time));
		elapsed_time += delta_time;

		compute_shader_watcher.update();

		glfwPollEvents();
		glfwGetWindowSize(window, &window_size.x, &window_size.y);
		glViewport(0, 0, window_size.x, window_size.y);
		frame_tex_size = glm::uvec2(window_size);
		glBindTexture(GL_TEXTURE_2D, frame_tex_out);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, frame_tex_size.x, frame_tex_size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
		glBindImageTexture(0, frame_tex_out, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		glm::dvec2 mouse;
		glfwGetCursorPos(window, &mouse.x, &mouse.y);
		auto m = -glm::pi<float>() * glm::vec2(mouse - glm::dvec2(window_size) * .5) / static_cast<float>(window_size.y);

		auto camera_pos = glm::vec3(0, 2, 0)
			+ glm::vec3(glm::sin(m.x) * glm::cos(m.y), glm::sin(m.y), glm::cos(m.x) * glm::cos(m.y)) * 5.f;
		auto camera_dir = glm::normalize(-camera_pos);

		{ // launch compute shaders and draw to image
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glUseProgram(compute_program);

			glm::ivec2 render_screens_count = glm::ivec2(players.size() % 2, players.size() / 2 + 1);
			glm::ivec2 render_size = glm::ceil(glm::vec2(window_size) / glm::vec2(render_screens_count));
			glUniform2i(glGetUniformLocation(compute_program, "render_size"), render_size.x, render_size.y);

			glUniform1f(glGetUniformLocation(compute_program, "elapsed_time"), elapsed_time.count() / 1000.f);
			glUniform1f(glGetUniformLocation(compute_program, "delta_time"), delta_time.count() / 1000.f);
			glUniform2i(glGetUniformLocation(compute_program, "mouse_coord"), mouse.x, window_size.y - mouse.y);

			for (auto i = 0ul; i < players.size(); ++i) {
				players[i].update(delta_time);
				auto player_str = std::string("players[") + std::to_string(i) + "]";
				glUniform4f(glGetUniformLocation(compute_program, (player_str + ".pos").c_str()),
					players[i].m_pos.x, players[i].m_pos.y, players[i].m_pos.z, 1);
				glUniform4f(glGetUniformLocation(compute_program, (player_str + ".dir").c_str()),
					players[i].m_dir.x, players[i].m_dir.y, players[i].m_dir.z, 1);
				glUniform4f(glGetUniformLocation(compute_program, (player_str + ".vel").c_str()),
					players[i].m_vel.x, players[i].m_vel.y, players[i].m_vel.z, 1);
			}

			for (auto i = 0ul; i < players.size(); ++i) {
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				glm::ivec2 render_screen = glm::ivec2(i % 2, i / 2);
				glm::ivec2 render_translation = glm::vec2(render_screen) * glm::ceil(glm::vec2(window_size) * .5f);
				auto camera_pos = players[i].m_pos + glm::vec3(0, .4, 0);
				auto camera_dir = players[i].m_dir;
				glUniform2i(glGetUniformLocation(compute_program, "render_translation"), render_translation.x, render_translation.y);
				glUniform3f(glGetUniformLocation(compute_program, "camera_pos"), camera_pos.x, camera_pos.y, camera_pos.z);
				glUniform3f(glGetUniformLocation(compute_program, "camera_dir"), camera_dir.x, camera_dir.y, camera_dir.z);
				glUniform1i(glGetUniformLocation(compute_program, "camera_player"), i);
				glDispatchCompute(
					min2(render_size.x, frame_tex_size.x - render_translation.x) / 8 + 1,
					min2(render_size.y, frame_tex_size.y - render_translation.y) / 8 + 1, 1);
			}
		}

		// make sure writing to image has finished before read
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		
		{ // present image to screen
			glUseProgram(display_program);
			glUniform2i(glGetUniformLocation(display_program, "tex_size"), frame_tex_size.x, frame_tex_size.y);
			glClear(GL_COLOR_BUFFER_BIT);
			glBindVertexArray(vao);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, frame_tex_out);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
			glfwSwapBuffers(window);
			glBindVertexArray(0);
		}

		++frames;
		if (fps_print_time.count() + 1000 < elapsed_time.count()) {
			fps_print_time += elapsed_time - fps_print_time;
			std::cout << frames << " fps" << std::endl;
			frames = 0;
		}
	}

	glfwTerminate();

	return 0;
}
