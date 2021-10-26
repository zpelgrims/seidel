#include "precomp.h"

int Random::seed = 1341;

int Random::rndInt()
{
	seed ^= seed << 13, seed ^= seed >> 17, seed ^= seed << 5;
	return seed;
}

float Random::rnd()
{
	return rndInt() * 2.3283064365387e-10f + 0.5f;
}
