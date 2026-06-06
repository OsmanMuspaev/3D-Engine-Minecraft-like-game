#pragma once
#include <cmath>

class Noise {
public:
    Noise();
    Noise(unsigned int seed) : m_seed(seed) {}

    float noise2D(float x, float z) const;

    // Fractal Brownian Motion — сумма октав
    float fbm(float x, float z, int octaves = 6, float lacunarity = 2.0f, float persistence = 0.5f) const;

    float temperature(float x, float z) const;
    float moisture(float x, float z) const;

private:
    unsigned int m_seed;

    float hash(int x, int z) const;
    float smoothstep(float t) const;
    float lerp(float a, float b, float t) const;
};
