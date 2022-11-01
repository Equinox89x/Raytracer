#pragma once
#include <cmath>
#include <limits>

namespace dae
{
	/* --- CONSTANTS --- */
	constexpr auto PI = 3.14159265358979323846f;
	constexpr auto PI_DIV_2 = 1.57079632679489661923f;
	constexpr auto PI_DIV_4 = 0.785398163397448309616f;
	constexpr auto PI_2 = 6.283185307179586476925f;
	constexpr auto PI_4 = 12.56637061435917295385f;

	constexpr auto TO_DEGREES = (180.0f / PI);
	constexpr auto TO_RADIANS(PI / 180.0f);

	inline float Square(float a)
	{
		return a * a;
	}

	inline float Lerpf(float a, float b, float factor)
	{
		return ((1 - factor) * a) + (factor * b);
	}

	inline bool AreEqual(float a, float b, float epsilon = FLT_EPSILON)
	{
		return abs(a - b) < epsilon;
	}

	#pragma region Compile time sqrt
	float constexpr sqrtNewtonRaphson(float x, float curr, float prev)
	{
		return curr == prev
			? curr
			: sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
	}

	float constexpr sqrtfc(float x)
	{
		return x >= 0 && x < std::numeric_limits<float>::infinity()
			? sqrtNewtonRaphson(x, x, 0)
			: std::numeric_limits<float>::quiet_NaN();
	}
	#pragma endregion
}