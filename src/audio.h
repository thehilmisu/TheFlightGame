#ifndef RIVERRAID3D_AUDIO_H
#define RIVERRAID3D_AUDIO_H

#pragma once
#include <map>
#include <string>
#include <SDL2/SDL_mixer.h>
#include <glm/glm.hpp>

namespace audio {
    class AudioManager {
        std::map<std::string, Mix_Chunk *> sounds;
        bool initialized = false;
        AudioManager();

    public:
        AudioManager(const AudioManager &) = delete;

        AudioManager &operator=(const AudioManager &) = delete;

        static AudioManager *get();

        // Load sounds from a packed sfx.impfile (e.g. "sfx.impfile")
        void importFromFile(const char *path);

        void playid(const std::string &name,
                    const glm::vec3 &position = glm::vec3(0.0f),
                    float volumeScale = 1.0f);

        ~AudioManager();
    };
} // namespace audio

#define SFX audio::AudioManager::get()
#define SNDSRC audio::AudioManager::get()


#endif //RIVERRAID3D_AUDIO_H
