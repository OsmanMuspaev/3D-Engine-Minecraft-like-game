#pragma once
#include <string>
#include <vector>
#include "World.h"

namespace WorldSave {
    // Save world data to saves/<name>/world.bin
    bool saveWorld(const std::string& name, const World& world, int worldSize, const std::string& seed);

    // Load world data from saves/<name>/world.bin
    bool loadWorld(const std::string& name, World& world, int& worldSize, std::string& seed);

    // Check if a save exists
    bool saveExists(const std::string& name);

    // Get list of all saved worlds
    std::vector<std::string> listSaves();

    // Delete a save
    bool deleteSave(const std::string& name);
}
