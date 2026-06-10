#pragma once
#include <cmath>

class Noise {
public:
    Noise();
    Noise(unsigned int seed) : m_seed(seed) {}

    unsigned int getSeed() const { return m_seed; }
    void setSeed(unsigned int seed) { m_seed = seed; }

    // Returns 2D value noise at the given coordinates.
    float noise2D(float x, float z) const;

    // Fractal Brownian Motion: sum of octaves.
    float fbm(float x, float z, int octaves = 6, float lacunarity = 2.0f, float persistence = 0.5f) const;

    // Biome temperature noise.
    float temperature(float x, float z) const;
    // Biome moisture noise.
    float moisture(float x, float z) const;

private:
    unsigned int m_seed;

    // Hashes integer coordinates to a float in [-1, 1].
    float hash(int x, int z) const;
    // Smoothstep interpolation: 3t^2 - 2t^3.
    float smoothstep(float t) const;
    // Linear interpolation between a and b.
    float lerp(float a, float b, float t) const;
};
