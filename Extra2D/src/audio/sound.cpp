#include <SDL2/SDL_mixer.h>
#include <extra2d/audio/sound.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

Sound::Sound(const std::string &name, const std::string &filePath,
             Mix_Chunk *chunk)
    : name_(name), filePath_(filePath), chunk_(chunk) {}

Sound::~Sound() {
  if (channel_ >= 0) {
    Mix_HaltChannel(channel_);
    channel_ = -1;
  }
  if (chunk_) {
    Mix_FreeChunk(chunk_);
    chunk_ = nullptr;
  }
}

bool Sound::play() {
  if (!chunk_) {
    E2D_LOG_WARN("Sound::play() failed: chunk is null for {}", name_);
    return false;
  }

  int loops = looping_ ? -1 : 0;
  int newChannel = Mix_PlayChannel(-1, chunk_, loops);

  if (newChannel < 0) {
    E2D_LOG_WARN("Sound::play() failed: no free channel for {} ({})", name_, Mix_GetError());
    return false;
  }

  channel_ = newChannel;

  int mixVol = static_cast<int>(volume_ * MIX_MAX_VOLUME);
  Mix_Volume(channel_, mixVol);

  return true;
}

void Sound::pause() {
  if (channel_ >= 0) {
    Mix_Pause(channel_);
  }
}

void Sound::resume() {
  if (channel_ >= 0) {
    Mix_Resume(channel_);
  }
}

void Sound::stop() {
  if (channel_ >= 0) {
    Mix_HaltChannel(channel_);
    channel_ = -1;
  }
}

bool Sound::isPlaying() const {
  if (channel_ < 0) {
    return false;
  }
  return Mix_Playing(channel_) && !Mix_Paused(channel_);
}

bool Sound::isPaused() const {
  if (channel_ < 0) {
    return false;
  }
  return Mix_Paused(channel_) != 0;
}

void Sound::setVolume(float volume) {
  volume_ = volume;
  if (channel_ >= 0) {
    int mixVol = static_cast<int>(volume * MIX_MAX_VOLUME);
    Mix_Volume(channel_, mixVol);
  }
}

void Sound::setLooping(bool looping) {
  looping_ = looping;
  // SDL_mixer 的循环在播放时设置，运行中无法更改
  // 如果需要即时生效，需要重新播放
}

void Sound::setPitch(float pitch) {
  pitch_ = pitch;
  // SDL2_mixer 不直接支持变速播放
  // 需要更高级的音频处理才能实现
}

float Sound::getDuration() const {
  // SDL2_mixer 没有直接获取时长的 API
  // 返回 0 表示不支持
  return 0.0f;
}

float Sound::getCursor() const {
  // SDL2_mixer 没有直接获取播放位置的 API
  return 0.0f;
}

void Sound::setCursor(float /*seconds*/) {
  // SDL2_mixer 不支持 seek 到特定位置 (对 chunks)
}

} // namespace extra2d
