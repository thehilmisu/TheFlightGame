#ifndef RIVERRAID3D_SETTINGS_H
#define RIVERRAID3D_SETTINGS_H

#include <string>

//Player settings and profile that survive between sessions.
//These are stored as a small key=value file in the per-user preferences
//directory (see SDL_GetPrefPath), so the game never writes next to its
//executable - that location is not writable in an installed build.
class Settings {
public:
    static Settings &getInstance();

    Settings(const Settings &) = delete;
    Settings &operator=(const Settings &) = delete;

    //Reads the settings file. Missing files and malformed values are not
    //errors, anything that cannot be read keeps its default.
    void load();

    //Writes the settings file, returns false if it could not be written
    bool save();

    [[nodiscard]] int getSelectedPlane() const { return selectedPlane; }
    void setSelectedPlane(int index);

    [[nodiscard]] bool soundIsEnabled() const { return soundEnabled; }
    void setSoundEnabled(bool enabled) { soundEnabled = enabled; }

    //Volume is a percentage in the range [0, 100]
    [[nodiscard]] int getVolume() const { return volume; }
    void setVolume(int v);

    [[nodiscard]] unsigned int getHighScore() const { return highScore; }

    //Records 'score' if it beats the stored best and saves immediately so that
    //a crash on exit cannot lose it. Returns true if it was a new best.
    bool submitScore(unsigned int score);

private:
    Settings() = default;
    ~Settings() = default;

    //Full path of the settings file, including the file name
    [[nodiscard]] std::string filePath() const;

    int selectedPlane = 0;
    bool soundEnabled = true;
    int volume = 50;
    unsigned int highScore = 0;
};

#define SETTINGS Settings::getInstance()

#endif //RIVERRAID3D_SETTINGS_H
