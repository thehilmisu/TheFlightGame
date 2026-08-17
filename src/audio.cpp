#include "audio.h"
#include "logger.h"
#include "assetloader.h"
#include "importfile.h"
#include "settings.h"
#include <algorithm>

namespace audio {
    AudioManager::AudioManager() {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            ERROR("SDL_mixer: %s", Mix_GetError());
            return;
        }
        Mix_AllocateChannels(16);
        // allow 16 simultaneous sounds
        initialized = true;
    }

    AudioManager::~AudioManager() {
        for (auto &[name, chunk]: sounds)
            Mix_FreeChunk(chunk);
        if (initialized)
            Mix_CloseAudio();
    }

    AudioManager *AudioManager::get() {
        static auto *audiomanager = new AudioManager;
        return audiomanager;
    }

    void AudioManager::importFromFile(const char *path) {
        if (!initialized) return;
        AssetLoader &loader = AssetLoader::getInstance();
        std::string content = loader.getAssetString(path);
        const auto entries = impfile::parseBin(content);

        for (const auto &entry: entries) {
            std::string wavPath = entry.getVar("path");
            const float gain = std::stof(entry.getVar("gain"));

            auto wavData = loader.getAssetData(wavPath);
            SDL_RWops *rw = SDL_RWFromConstMem(wavData.data(), (int) wavData.size());
            if (Mix_Chunk *chunk = Mix_LoadWAV_RW(rw, 1)) {
                // Map gain to SDL volume (0-128). Cap at MIX_MAX_VOLUME.
                const int vol = std::min(MIX_MAX_VOLUME, static_cast<int>(gain * 1.33f));
                Mix_VolumeChunk(chunk, vol);
                sounds[entry.name] = chunk;
            }
        }
    }


    void AudioManager::playid(const std::string &name,
                              const glm::vec3 & /*position*/,
                              float volumeScale) {
        if (!initialized || muted) return;
        const auto it = sounds.find(name);
        if (it == sounds.end()) return;

        const int channel = Mix_PlayChannel(-1, it->second, 0);
        if (channel == -1) return;

        //Fold the master volume and the per-call scale into the channel volume.
        //Mix_VolumeChunk(chunk, -1) queries without changing it, so the gain
        //each sound was imported with from sfx.impfile is preserved.
        const auto chunkvolume = static_cast<float>(Mix_VolumeChunk(it->second, -1));
        const int volume = static_cast<int>(chunkvolume * master * volumeScale);
        Mix_Volume(channel, std::clamp(volume, 0, MIX_MAX_VOLUME));
    }

    void AudioManager::setMasterVolume(int percent) {
        master = static_cast<float>(std::clamp(percent, 0, 100)) / 100.0f;
    }

    void AudioManager::setMuted(bool mute) {
        muted = mute;
        //Cut anything already sounding, otherwise a long sample keeps playing
        //after the player mutes
        if (muted && initialized)
            Mix_HaltChannel(-1);
    }

    void AudioManager::applySettings() {
        setMasterVolume(SETTINGS.getVolume());
        setMuted(!SETTINGS.soundIsEnabled());
    }
}
