#version 450 core

layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform restrict writeonly image2D output_image;

uniform ivec2 render_translation;
uniform ivec2 render_size;
uniform float elapsed_time;
uniform float delta_time;
uniform ivec2 mouse_coord;
uniform vec3 camera_pos;
uniform vec3 camera_dir;
uniform int camera_player;

struct Player {
	vec4 pos;
	vec4 dir;
	vec4 vel;
};

uniform Player players[4];

#define pi 3.141

mat3 look_at(vec3 d)
{
	vec3 u = vec3(0, 1, 0);
	vec3 r = normalize(cross(d, u));
	u = normalize(cross(r, d));
	return mat3(r, u, d);
}

mat2 rotateXY(float a)
{
	return mat2(
		cos(a), -sin(a),
		sin(a), cos(a)
	);
}

float sphere(vec3 p, float r)
{
	return length(p) - r;
}

float roundcube(vec3 p, vec2 r)
{
	return length(max(abs(p) - (r.x - r.y), vec3(0))) - r.y;
}

float roundcube(vec3 p, vec4 r)
{
	return length(max(abs(p) - (r.xyz - r.w), vec3(0))) - r.w;
}

float cube(vec3 p, float r)
{
	return roundcube(p, vec2(r, .0));
}

float cube(vec3 p, vec3 r)
{
	return roundcube(p, vec4(r, 0));
}

float quickcube(vec3 p, float r)
{
	p = abs(p);
	return max(p.x, max(p.y, p.z)) - r;
}

float quickcube(vec3 p, vec3 r)
{
	p = abs(p);
	return max(p.x - r.x, max(p.y - r.y, p.z - r.z));
}

float plane(vec3 p, vec3 n, float r)
{
	return dot(p, n) - r;
}

float line(vec3 p, vec3 a, vec3 b, float r)
{
	vec3 ab = b - a;
	vec3 ap = p - a;
	float h = clamp(dot(ap, ab) / dot(ab, ab), 0., 1.);
	return length(ap - h * ab) - r;
}

float torus(vec3 p, vec2 r)
{
	vec3 p0 = normalize(vec3(p.xy, 0)) * r.x;
	return length(p0 - p) - r.y;
}

float onion(float d, float thickness)
{
	return abs(d + thickness) - thickness;
}

vec3 alongate(vec3 p, vec3 a, vec3 b)
{
	return max(min(p, a), p - b);
}

float pickerick(vec3 p)
{
	// p.xz *= sin(p.y - elapsed_time) * .2 + 1.;
	p.xz -= cos(p.y * 1.5) * .06;
	float pickle = line(p, vec3(0, -1, 0), vec3(0, 1, 0), .5);
	float eyeball = sphere(vec3(abs(p.x), p.yz) - vec3(.17, .7, .45), .17);
	eyeball = max(eyeball, sphere(vec3(abs(p.x), p.yz) - vec3(.17, .53, .43), .3));
	eyeball = max(eyeball, sphere(vec3(abs(p.x), p.yz) - vec3(.17, .87, .43), .3));
	eyeball = max(eyeball, -sphere(vec3(abs(p.x), p.yz) - vec3(.16, .67, .6), .03));
	float eyebrow = line(vec3(abs(p.x), p.yz), vec3(0, .85, .5), vec3(.2, .92, .47), .04);
	float mouth = line(vec3(abs(p.y), p.yz), vec3(0, .4, .5), vec3(.4, .4 - cos(abs(p.x) * 6.) * .1, .5), .15 - cos(abs(p.x) * 5.) * .05);

	return min(max(pickle, -mouth), min(eyeball, eyebrow)) * .8;
}

float scene(vec3 p)
{
	float c0 = 100.;
	for (int i = 0; i < 4; ++i) {
		if (camera_player != i && players[i].pos.w == 1) {
			vec3 d = vec3(players[i].dir.x, 0, players[i].dir.z) * .5;
			float dl = .5 - clamp(-players[i].dir.y * .7, 0, .5);
			vec3 cp = inverse(look_at(normalize(d))) * (p - players[i].pos.xyz);
			// cp.x += length(players[i].vel) * 10. * sin(cp.z * 5. + elapsed_time) * .05;
			c0 = min(c0, roundcube(cp, vec4(max(.05, dl), .5, .5, .05)));
		}
	}

	float c1 = quickcube(p, vec3(5, .5, 5));
	c1 = max(-c1, -plane(p, normalize(vec3(0, -1, 0)), .0));

	return min(c0, c1) * .5;

	// return pickerick(p);

	// p.xz = rotateXY(elapsed_time) * p.xz;
	// vec3 pt = alongate(p, vec3(.1, .3, .5), vec3(.3, .3, .3));
	// pt = look_at(normalize(-vec3(1, 1, 1))) * pt;
	// float t0 = torus(pt, vec2(.5, .2));
	// t0 = onion(t0, .03);
	// t0 = onion(t0, .01);
	// /* t0 = onion(t0, .01); */
	// float p0 = plane(p, normalize(-vec3(1, 0, 0)), 0.);
	// float s0 = sphere(p, .6);
	// p0 = max(p0, -s0);
	// t0 = max(t0, -p0);
	// return t0;
}

bool march(vec3 ro, vec3 rd, out vec3 p, out float steps)
{
	p = ro;
	float ol = 0.;

	for (int i = 0; i < 200; ++i) {
		float l = scene(p);
		ol += l;
		p = ro + rd * ol;
		steps = float(i) / 100.;

		if (l < .01)
			return true;
		if (ol > 20.)
			return false;
	}

	return false;
}

vec3 normal(vec3 p)
{
	float l = scene(p);
	vec2 e = vec2(0, .001);

	return normalize(
		l - vec3(
			scene(p - e.yxx),
			scene(p - e.xyx),
			scene(p - e.xxy)
		)
	);
}

void main() {
	vec2 output_size = min(render_size, vec2(imageSize(output_image) - render_translation));
  vec2 output_coord = gl_GlobalInvocationID.xy;
	if (output_coord.x >= output_size.x || output_coord.y >= output_size.y) return;

	vec2 uv = (output_coord - output_size * .5) / output_size.y;
	vec3 c = vec3(0);
  vec2 m = vec2(mouse_coord - output_size * .5) / output_size.y;

	// m *= -pi;
	// vec3 ro = vec3(sin(m.x) * cos(m.y), sin(m.y), cos(m.x) * cos(m.y)) * 3.;
	// vec3 rd = look_at(normalize(-ro)) * normalize(vec3(uv, 1));
	vec3 ro = camera_pos;
	vec3 rd = look_at(camera_dir) * normalize(vec3(uv, 1));

	vec3 p;
	float steps;
	bool hit = march(ro, rd, p, steps);
	vec3 n = normal(p);
	/* c.r += hit ? .9 : .0; */
	c.g += hit ? dot(rd, -n) : 0.;
	c.b += steps;
	c.g += hit ? 0. : 1. - steps;
	/* c.g += hit ? 0. : steps * steps; */
	// if (hit)
	// 	c *= march(p + n * .003, normalize(vec3(5, 4, 3) - p), p, steps) ? .6 : 1.;

	imageStore(output_image, render_translation + ivec2(output_coord), vec4(c, 1));
}
