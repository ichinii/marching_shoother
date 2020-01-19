#pragma once

#include <ostream>
#include <chrono>
#include <glm/glm.hpp>

template <typename CharT, typename Traits, glm::length_t L, typename T, glm::qualifier Q>
auto& operator << (std::basic_ostream<CharT, Traits>& os, glm::vec<L, T, Q>& vec)
{
	if (L == 0) return os << "()";
	os << '(' << vec[0];
	for (glm::length_t i = 1; i < L; ++i) os << '|' << vec[i];
	return os << ')';
}

template <typename CharT, typename Traits, typename Rep, typename Period>
auto& operator << (std::basic_ostream<CharT, Traits>& os, std::chrono::duration<Rep, Period> duration)
{
	return os << duration.count();
}

template <typename A, typename B>
auto&& min2(A&& a, B&& b)
{
	using T = decltype(a + b);
	T c = std::forward<A>(a);
	T d = std::forward<B>(b);
	return std::move(c < d ? c : d);
}

template <glm::length_t L, typename T, glm::qualifier Q>
auto right(const glm::vec<L, T, Q>& dir)
{
	if constexpr (L == 4)
		return glm::vec4(glm::cross(glm::vec3(dir.x, dir.y, dir.z), glm::vec3(0, 1, 0)), 0.);
	else
		return glm::cross(glm::vec3(dir.x, dir.y, dir.z), glm::vec3(0, 1, 0));
}

template <glm::length_t L, typename T, glm::qualifier Q>
auto xyz(const glm::vec<L, T, Q>& v)
{
	static_assert(L >= 3);
	return glm::vec3(v[0], v[1], v[2]);
}

template <glm::length_t L, typename T, glm::qualifier Q>
auto xy(const glm::vec<L, T, Q>& v)
{
	static_assert(L >= 2);
	return glm::vec2(v[0], v[1]);
}

template <glm::length_t L, typename T, glm::qualifier Q>
auto xz(const glm::vec<L, T, Q>& v)
{
	static_assert(L >= 3);
	return glm::vec2(v[0], v[2]);
}

template <glm::length_t L, typename T, glm::qualifier Q>
auto yz(const glm::vec<L, T, Q>& v)
{
	static_assert(L >= 3);
	return glm::vec2(v[1], v[2]);
}
