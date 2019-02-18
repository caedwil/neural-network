#ifndef RANDOM_HH
#define RANDOM_HH

#include <random>

class Random
{
public:
	Random();

	int nextInt();
	int nextInt(int upper);
	int nextInt(int lower, int upper);

	double nextDouble();
	double nextDouble(double upper);
	double nextDouble(double lower, double upper);
private:
	std::mt19937 generator;
	std::uniform_real_distribution<double> distribution;

	double next();
};

#endif