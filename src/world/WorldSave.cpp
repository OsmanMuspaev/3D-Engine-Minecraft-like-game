#include "WorldSave.h"
#include "Chunk.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstring>

namespace fs = std::filesystem;

static const char MAGIC[6] = {'Q','U','A','D','R','O'};
static constexpr int HEADER_SIZE = 6 + sizeof(int) + sizeof(uint32_t);

namespace WorldSave {

bool saveWorld(const std::string& name, const World& world, int worldSize, const std::string& seed) {
    fs::path saveDir = fs::path("saves") / name;
    fs::create_directories(saveDir);

    fs::path filePath = saveDir / "world.bin";
    std::ofstream out(filePath, std::ios::binary);
    if (!out.is_open()) return false;

    out.write(MAGIC, 6);

    int ws = worldSize;
    out.write(reinterpret_cast<const char*>(&ws), sizeof(int));

    uint32_t seedLen = static_cast<uint32_t>(seed.size());
    out.write(reinterpret_cast<const char*>(&seedLen), sizeof(uint32_t));
    out.write(seed.c_str(), seedLen);

    const auto& chunks = world.getChunks();
    int chunkCount = static_cast<int>(chunks.size());
    out.write(reinterpret_cast<const char*>(&chunkCount), sizeof(int));

    for (auto& [key, chunk] : chunks) {
        auto [cx, cy, cz] = key;
        out.write(reinterpret_cast<const char*>(&cx), sizeof(int));
        out.write(reinterpret_cast<const char*>(&cy), sizeof(int));
        out.write(reinterpret_cast<const char*>(&cz), sizeof(int));

        for (int ly = 0; ly < Chunk::SIZE; ly++) {
            for (int lz = 0; lz < Chunk::SIZE; lz++) {
                for (int lx = 0; lx < Chunk::SIZE; lx++) {
                    Block block = chunk->getBlock(lx, ly, lz);
                    unsigned int bt = static_cast<unsigned int>(block.type);
                    out.write(reinterpret_cast<const char*>(&bt), sizeof(unsigned int));
                }
            }
        }
    }

    return out.good();
}

bool loadWorld(const std::string& name, World& world, int& worldSize, std::string& seed) {
    fs::path filePath = fs::path("saves") / name / "world.bin";
    if (!fs::exists(filePath)) return false;

    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) return false;

    char magic[6];
    in.read(magic, 6);
    if (std::memcmp(magic, MAGIC, 6) != 0) return false;

    int ws;
    in.read(reinterpret_cast<char*>(&ws), sizeof(int));
    worldSize = ws;

    uint32_t seedLen;
    in.read(reinterpret_cast<char*>(&seedLen), sizeof(uint32_t));
    seed.resize(seedLen);
    in.read(&seed[0], seedLen);

    int chunkCount;
    in.read(reinterpret_cast<char*>(&chunkCount), sizeof(int));

    for (int i = 0; i < chunkCount; i++) {
        int cx, cy, cz;
        in.read(reinterpret_cast<char*>(&cx), sizeof(int));
        in.read(reinterpret_cast<char*>(&cy), sizeof(int));
        in.read(reinterpret_cast<char*>(&cz), sizeof(int));

        for (int ly = 0; ly < Chunk::SIZE; ly++) {
            for (int lz = 0; lz < Chunk::SIZE; lz++) {
                for (int lx = 0; lx < Chunk::SIZE; lx++) {
                    unsigned int bt;
                    in.read(reinterpret_cast<char*>(&bt), sizeof(unsigned int));
                    int wx = cx * Chunk::SIZE + lx;
                    int wy = cy * Chunk::SIZE + ly;
                    int wz = cz * Chunk::SIZE + lz;
                    world.setBlock(wx, wy, wz, static_cast<BlockType>(bt));
                }
            }
        }
    }

    return in.good() || in.eof();
}

bool saveExists(const std::string& name) {
    fs::path filePath = fs::path("saves") / name / "world.bin";
    return fs::exists(filePath);
}

std::vector<std::string> listSaves() {
    std::vector<std::string> saves;
    fs::path savesDir("saves");
    if (!fs::exists(savesDir) || !fs::is_directory(savesDir)) return saves;

    for (auto& entry : fs::directory_iterator(savesDir)) {
        if (entry.is_directory()) {
            fs::path worldFile = entry.path() / "world.bin";
            if (fs::exists(worldFile)) {
                saves.push_back(entry.path().filename().string());
            }
        }
    }

    std::sort(saves.begin(), saves.end());
    return saves;
}

bool deleteSave(const std::string& name) {
    fs::path saveDir = fs::path("saves") / name;
    if (!fs::exists(saveDir)) return false;

    std::error_code ec;
    fs::remove_all(saveDir, ec);
    return !ec;
}

}
