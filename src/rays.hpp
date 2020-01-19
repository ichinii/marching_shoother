#pragma once

#include <glm/glm.hpp>

class Rays {
public:
	glm::mat3 look_at(glm::vec3 d);
	glm::mat2 rotateXY(float a);
	float sphere(glm::vec3 p, float r);
	float roundcube(glm::vec3 p, glm::vec2 r);
	float roundcube(glm::vec3 p, glm::vec4 r);
	float cube(glm::vec3 p, float r);
	float cube(glm::vec3 p, glm::vec3 r);
	float quickcube(glm::vec3 p, float r);
	float quickcube(glm::vec3 p, glm::vec3 r);
	float plane(glm::vec3 p, glm::vec3 n, float r);
	float line(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r);
	float torus(glm::vec3 p, glm::vec2 r);
	float onion(float d, float thickness);
	glm::vec3 alongate(glm::vec3 p, glm::vec3 a, glm::vec3 b);
	float scene(glm::vec3 p);
	bool march(glm::vec3 ro, glm::vec3 rd, glm::vec3* p, float* steps);
	glm::vec3 normal(glm::vec3 p);

	glm::ivec2 render_translation;
	glm::ivec2 render_size;
	float elapsed_time;
	float delta_time;
	glm::ivec2 mouse_coord;
	glm::vec3 camera_pos;
	glm::vec3 camera_dir;
	int camera_player;

	struct Player {
		glm::vec4 pos;
		glm::vec4 dir;
		glm::vec4 vel;
	};

	Player players[4];

private:
};
