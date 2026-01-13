#ifndef NOISE_H
#define NOISE_H
#include <array>

namespace rng {
	//Array that represents a random permutation of 0 -> 255
	typedef std::array<int, 256> permutation256;
	void createPermutation(permutation256 &p, int seed);
}

namespace perlin {
	//Assume that 0.0 <= x <= 1.0
	float interpolate(float a, float b, float x);
	float noise(float x, float y, const rng::permutation256 &p);
	float noise(float x, float y, int repeat, const rng::permutation256 &p);	
}

#endif
