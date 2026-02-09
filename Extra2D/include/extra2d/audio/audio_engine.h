#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <extra2d/core/types.h>

namespace extra2d {

class Sound;

class AudioEngine {
public:
    static AudioEngine& getInstance();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioEngine(AudioEngine&&) = delete;
    AudioEngine& operator=(AudioEngine&&) = delete;

    bool initialize();
    void shutdown();

    std::shared_ptr<Sound> loadSound(const std::string& filePath);
    std::shared_ptr<Sound> loadSound(const std::string& name, const std::string& filePath);

    std::shared_ptr<Sound> getSound(const std::string& name);
    void unloadSound(const std::string& name);
    void unloadAllSounds();

    void setMasterVolume(float volume);
    float getMasterVolume() const;

    void pauseAll();
    void resumeAll();
    void stopAll();

private:
    AudioEngine() = default;
    ~AudioEngine();

    std::unordered_map<std::string, std::shared_ptr<Sound>> sounds_;
    float masterVolume_ = 1.0f;
    bool initialized_ = false;
};

} // namespace extra2d
