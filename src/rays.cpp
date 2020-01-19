#include "rays.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include "misc.hpp"

/*
 * this class is basically a mirror to the shader, to make equivalent calls
 */

#define pi glm::pi<float>()

using namespace glm;

mat3 Rays::look_at(vec3 d)
{
	vec3 u = vec3(0, 1, 0);
	vec3 r = normalize(cross(d, u));
	u = normalize(cross(r, d));
	return mat3(r, u, d);
}

mat2 Rays::rotateXY(float a)
{
	return mat2(
		cos(a), -sin(a),
		sin(a), cos(a)
	);
}

float Rays::sphere(vec3 p, float r)
{
	return length(p) - r;
}

float Rays::roundcube(vec3 p, vec2 r)
{
	return length(max(abs(p) - (r.x - r.y), vec3(0))) - r.y;
}

float Rays::roundcube(vec3 p, vec4 r)
{
	return length(max(abs(p) - (xyz(r) - r.w), vec3(0))) - r.w;
}

float Rays::cube(vec3 p, float r)
{
	return roundcube(p, vec2(r, .0));
}

float Rays::cube(vec3 p, vec3 r)
{
	return roundcube(p, vec4(r, 0));
}

float Rays::quickcube(vec3 p, float r)
{
	p = abs(p);
	return max(p.x, max(p.y, p.z)) - r;
}

float Rays::quickcube(vec3 p, vec3 r)
{
	p = abs(p);
	return max(p.x - r.x, max(p.y - r.y, p.z - r.z));
}

float Rays::plane(vec3 p, vec3 n, float r)
{
	return dot(p, n) - r;
}

float Rays::line(vec3 p, vec3 a, vec3 b, float r)
{
	vec3 ab = b - a;
	vec3 ap = p - a;
	float h = clamp(dot(ap, ab) / dot(ab, ab), 0.f, 1.f);
	return length(ap - h * ab) - r;
}

float Rays::torus(vec3 p, vec2 r)
{
	vec3 p0 = normalize(vec3(xy(p), 0.f)) * r.x;
	return length(p0 - p) - r.y;
}

float Rays::onion(float d, float thickness)
{
	return abs(d + thickness) - thickness;
}

vec3 Rays::alongate(vec3 p, vec3 a, vec3 b)
{
	return max(min(p, a), p - b);
}

float Rays::scene(vec3 p)
{
	float c0 = 100.;
	for (int i = 0; i < 4; ++i) {
		if (camera_player != i && players[i].pos.w == 1) {
			vec3 d = vec3(players[i].dir.x, 0, players[i].dir.z) * .5f;
			float dl = .5 - clamp(-players[i].dir.y * .7f, 0.f, .5f);
			vec3 cp = inverse(look_at(normalize(d))) * (p - xyz(players[i].pos));
			// cp.x += length(players[i].vel) * 10. * sin(cp.z * 5. + elapsed_time) * .05;
			c0 = min(c0, roundcube(cp, vec4(max(.05f, dl), .5, .5, .05)));
		}
	}

	float c1 = quickcube(p, vec3(5, .5, 5));
	c1 = max(-c1, -plane(p, normalize(vec3(0, -1, 0)), .0));

	return min(c0, c1) * .5;
}

bool Rays::march(vec3 ro, vec3 rd, vec3* p, float* steps)
{
	*p = ro;
	float ol = 0.;

	for (int i = 0; i < 200; ++i) {
		float l = scene(*p);
		ol += l;
		*p = ro + rd * ol;
		*steps = float(i) / 100.;

		if (l < .01)
			return true;
		if (ol > 20.)
			return false;
	}

	return false;
}

vec3 Rays::normal(vec3 p)
{
	float l = scene(p);
	vec2 e = vec2(0, .001);

	return normalize(
		l - vec3(
			scene(p - vec3(e.y, e.x, e.x)),
			scene(p - vec3(xy(e), e.x)),
			scene(p - vec3(e.x, xy(e)))
		)
	);
}
