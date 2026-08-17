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
        //Master volume as a multiplier in [0, 1] and the mute flag, both driven
        //by the options menu via applySettings()
        float master = 1.0f;
        bool muted = false;
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

        // Master volume as a percentage in [0, 100]
        void setMasterVolume(int percent);

        // Silences playback and stops anything currently sounding
        void setMuted(bool mute);

        // Pulls volume and mute straight from the saved settings
        void applySettings();

        ~AudioManager();
    };
} // namespace audio

#define SFX audio::AudioManager::get()
#define SNDSRC audio::AudioManager::get()


#endif //RIVERRAID3D_AUDIO_H
