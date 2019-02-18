#include "random.hh"
#include <random>

Random::Random()
{
	// Seed Mersenne Twister with random device.
	this->generator = std::mt19937(std::random_device()());
	// Setup random distribution.
	this->distribution = std::uniform_real_distribution<double>(0.0, 1.0);
}

int Random::nextInt()
{
	return next() * 100000;
}

int Random::nextInt(int upper)
{
	return nextInt() % upper;
}

int Random::nextInt(int lower, int upper)
{
	return (nextInt() + lower) % upper;
}

double Random::nextDouble()
{
	return next();
}

double Random::nextDouble(double upper)
{
	return next() * upper;
}

double Random::nextDouble(double lower, double upper)
{
	return 0.0;
}

double Random::next()
{
	return distribution(generator);
}