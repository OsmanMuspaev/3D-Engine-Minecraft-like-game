#pragma once
#include <cmath>

// Simple 2D value noise generator with fractal Brownian motion support
class Noise {
public:
    Noise();
    Noise(unsigned int seed) : m_seed(seed) {}

    // Returns 2D value noise at the given coordinates
    float noise2D(float x, float z) const;

    // Fractal Brownian Motion: sum of octaves
    float fbm(float x, float z, int octaves = 6, float lacunarity = 2.0f, float persistence = 0.5f) const;

    // Biome temperature noise
    float temperature(float x, float z) const;
    // Biome moisture noise
    float moisture(float x, float z) const;

private:
    unsigned int m_seed;

    // Hash function for integer coordinates
    float hash(int x, int z) const;
    // Smooth interpolation curve
    float smoothstep(float t) const;
    // Linear interpolation
    float lerp(float a, float b, float t) const;
};
