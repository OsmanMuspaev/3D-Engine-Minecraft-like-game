#pragma once
#include <string>
#include <filesystem>
#include <vector>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace AssetPath {

inline std::string getBundleResourcesPath() {
#ifdef __APPLE__
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle) {
        CFURLRef url = CFBundleCopyResourcesDirectoryURL(bundle);
        if (url) {
            char path[1024];
            if (CFURLGetFileSystemRepresentation(url, true, reinterpret_cast<UInt8*>(path), sizeof(path))) {
                CFRelease(url);
                return std::string(path);
            }
            CFRelease(url);
        }
    }
#endif
    return "";
}

inline std::string resolve(const std::string& filename) {
    std::string bundlePath = getBundleResourcesPath();
    if (!bundlePath.empty()) {
        std::string bp = bundlePath + "/assets/" + filename;
        if (std::filesystem::exists(bp)) {
            return bp;
        }
    }

    std::vector<std::string> paths = {
        "assets/" + filename,
        "../Resources/assets/" + filename,
        "3D-game.app/Contents/Resources/assets/" + filename,
    };

    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return "assets/" + filename;
}

}
