#include "settings.h"
#include "logger.h"
#include <SDL.h>
#include <algorithm>
#include <fstream>

constexpr int MIN_VOLUME = 0;
constexpr int MAX_VOLUME = 100;

Settings &Settings::getInstance() {
    static Settings instance;
    return instance;
}

std::string Settings::filePath() const {
    //SDL resolves this to the correct per-user location on every platform we
    //ship on (~/Library/Application Support, %APPDATA%, ~/.local/share)
    char *prefpath = SDL_GetPrefPath("thehilmisu", "RiverRaid3D");
    if (!prefpath) {
        WARN("Could not resolve preferences directory, using working directory");
        return "settings.cfg";
    }
    std::string path = std::string(prefpath) + "settings.cfg";
    SDL_free(prefpath);
    return path;
}

void Settings::setSelectedPlane(int index) {
    //The plane list lives on Player, so the real upper bound is checked there;
    //here we only guard against a negative index from a hand-edited file
    selectedPlane = std::max(0, index);
}

void Settings::setVolume(int v) {
    volume = std::clamp(v, MIN_VOLUME, MAX_VOLUME);
}

void Settings::load() {
    const std::string path = filePath();
    std::ifstream file(path);
    if (!file) {
        INFO("No settings file at %s, starting from defaults", path.c_str());
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        const size_t separator = line.find('=');
        if (separator == std::string::npos)
            continue;

        const std::string key = line.substr(0, separator);
        const std::string value = line.substr(separator + 1);

        //A corrupt or hand-edited file must not take the game down with it
        try {
            if (key == "selectedPlane")
                setSelectedPlane(std::stoi(value));
            else if (key == "soundEnabled")
                setSoundEnabled(std::stoi(value) != 0);
            else if (key == "volume")
                setVolume(std::stoi(value));
            else if (key == "highScore")
                highScore = static_cast<unsigned int>(std::stoul(value));
            else
                WARN("Unknown setting '%s', ignoring", key.c_str());
        } catch (const std::exception &) {
            WARN("Malformed value for setting '%s', keeping default", key.c_str());
        }
    }

    INFO("Loaded settings from %s", path.c_str());
}

bool Settings::save() {
    const std::string path = filePath();
    std::ofstream file(path, std::ios::trunc);
    if (!file) {
        ERROR("Could not open %s for writing", path.c_str());
        return false;
    }

    file << "# River Raid 3D settings\n";
    file << "selectedPlane=" << selectedPlane << "\n";
    file << "soundEnabled=" << (soundEnabled ? 1 : 0) << "\n";
    file << "volume=" << volume << "\n";
    file << "highScore=" << highScore << "\n";

    if (!file.good()) {
        ERROR("Failed while writing %s", path.c_str());
        return false;
    }
    return true;
}

bool Settings::submitScore(unsigned int score) {
    if (score <= highScore)
        return false;

    highScore = score;
    save();
    return true;
}
