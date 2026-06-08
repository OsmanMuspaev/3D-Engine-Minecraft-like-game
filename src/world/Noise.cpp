#include "Noise.h"
#include <cmath>
#include <cstdint>
#include <random>

// Initializes with a random seed from the system
Noise::Noise() {
    std::random_device rd;
    m_seed = rd();
}

// Hashes integer coordinates to a float in [-1, 1]
float Noise::hash(int x, int z) const {
    uint32_t n = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(z) * 668265263u + m_seed;
    n = (n ^ (n >> 13)) * 1274126177u;
    n = n ^ (n >> 16);
    return (static_cast<float>(n & 0x7FFFFFFFu) / 2147483647.0f) * 2.0f - 1.0f;
}

// Smoothstep interpolation: 3t^2 - 2t^3
float Noise::smoothstep(float t) const {
    return t * t * (3.0f - 2.0f * t);
}

// Linear interpolation between a and b
float Noise::lerp(float a, float b, float t) const {
    return a + t * (b - a);
}

// Bilinear interpolation of noise at fractional coordinates
float Noise::noise2D(float x, float z) const {
    int ix = static_cast<int>(std::floor(x));
    int iz = static_cast<int>(std::floor(z));
    float fx = x - ix;
    float fz = z - iz;

    float sx = smoothstep(fx);
    float sz = smoothstep(fz);

    float v00 = hash(ix, iz);
    float v10 = hash(ix + 1, iz);
    float v01 = hash(ix, iz + 1);
    float v11 = hash(ix + 1, iz + 1);

    float a = lerp(v00, v10, sx);
    float b = lerp(v01, v11, sx);
    return lerp(a, b, sz);
}

// Accumulates multiple octaves of noise for natural-looking terrain
float Noise::fbm(float x, float z, int octaves, float lacunarity, float persistence) const {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {
        value += noise2D(x * frequency, z * frequency) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return value / maxAmplitude;
}

// Temperature noise using a seed-shifted noise instance
float Noise::temperature(float x, float z) const {
    Noise shifted(m_seed ^ 0xA5C3E291u);
    return shifted.fbm(x * 0.002f, z * 0.002f, 4);
}

// Moisture noise using a seed-shifted noise instance
float Noise::moisture(float x, float z) const {
    Noise shifted(m_seed ^ 0x5B7A1D3Fu);
    return shifted.fbm(x * 0.003f, z * 0.003f, 4);
}
