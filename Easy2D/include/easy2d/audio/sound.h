#pragma once

#include <string>
#include <easy2d/core/types.h>

struct Mix_Chunk;

namespace easy2d {

class AudioEngine;

class Sound {
public:
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    bool play();
    void pause();
    void resume();
    void stop();

    bool isPlaying() const;
    bool isPaused() const;

    void setVolume(float volume);
    float getVolume() const { return volume_; }

    void setLooping(bool looping);
    bool isLooping() const { return looping_; }

    void setPitch(float pitch);
    float getPitch() const { return pitch_; }

    float getDuration() const;
    float getCursor() const;
    void setCursor(float seconds);

    const std::string& getFilePath() const { return filePath_; }
    const std::string& getName() const { return name_; }

private:
    friend class AudioEngine;

    Sound(const std::string& name, const std::string& filePath, Mix_Chunk* chunk);

    std::string name_;
    std::string filePath_;
    Mix_Chunk* chunk_ = nullptr;
    int channel_ = -1;   // SDL_mixer 分配的通道，-1 表示未播放
    float volume_ = 1.0f;
    float pitch_ = 1.0f;
    bool looping_ = false;
};

} // namespace easy2d
