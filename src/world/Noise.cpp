#include "Noise.h"
#include <cmath>
#include <cstdint>
#include <random>

Noise::Noise() {
    std::random_device rd;
    m_seed = rd();
}

float Noise::hash(int x, int z) const {
    uint32_t n = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(z) * 668265263u + m_seed;
    n = (n ^ (n >> 13)) * 1274126177u;
    n = n ^ (n >> 16);
    return (static_cast<float>(n & 0x7FFFFFFFu) / 2147483647.0f) * 2.0f - 1.0f;
}

float Noise::smoothstep(float t) const {
    return t * t * (3.0f - 2.0f * t);
}

float Noise::lerp(float a, float b, float t) const {
    return a + t * (b - a);
}

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

float Noise::temperature(float x, float z) const {
    Noise shifted(m_seed ^ 0xA5C3E291u);
    return shifted.fbm(x * 0.002f, z * 0.002f, 4);
}

float Noise::moisture(float x, float z) const {
    Noise shifted(m_seed ^ 0x5B7A1D3Fu);
    return shifted.fbm(x * 0.003f, z * 0.003f, 4);
}
