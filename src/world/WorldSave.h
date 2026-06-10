#pragma once
#include <string>
#include <vector>
#include "World.h"

namespace WorldSave {
    bool saveWorld(const std::string& name, const World& world, int worldSize, const std::string& seed);
    bool loadWorld(const std::string& name, World& world, int& worldSize, std::string& seed);
    bool saveExists(const std::string& name);
    std::vector<std::string> listSaves();
    bool deleteSave(const std::string& name);
}
