#include "extra2d/audio/audio_engine.h"
#include "extra2d/audio/sound.h"
#include "extra2d/utils/logger.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

namespace extra2d {

AudioEngine &AudioEngine::getInstance() {
  static AudioEngine instance;
  return instance;
}

AudioEngine::~AudioEngine() { shutdown(); }

bool AudioEngine::initialize() {
  if (initialized_) {
    return true;
  }

  // 初始化 SDL 音频子系统
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
    E2D_LOG_ERROR("Failed to initialize SDL audio: {}", SDL_GetError());
    return false;
  }

  // 打开音频设备
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
    E2D_LOG_ERROR("Failed to open audio: {}", Mix_GetError());
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return false;
  }

  // 分配混音通道
  Mix_AllocateChannels(32);

  initialized_ = true;
  E2D_LOG_INFO("AudioEngine initialized successfully (SDL2_mixer)");
  return true;
}

void AudioEngine::shutdown() {
  if (!initialized_) {
    return;
  }

  unloadAllSounds();

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);

  initialized_ = false;
  E2D_LOG_INFO("AudioEngine shutdown");
}

std::shared_ptr<Sound> AudioEngine::loadSound(const std::string &filePath) {
  return loadSound(filePath, filePath);
}

std::shared_ptr<Sound> AudioEngine::loadSound(const std::string &name,
                                              const std::string &filePath) {
  if (!initialized_) {
    E2D_LOG_ERROR("AudioEngine not initialized");
    return nullptr;
  }

  // 检查是否已存在
  auto it = sounds_.find(name);
  if (it != sounds_.end()) {
    return it->second;
  }

  Mix_Chunk *chunk = Mix_LoadWAV(filePath.c_str());
  if (!chunk) {
    E2D_LOG_ERROR("Failed to load sound: {} ({})", filePath, Mix_GetError());
    return nullptr;
  }

  auto sound = std::shared_ptr<Sound>(new Sound(name, filePath, chunk));
  sounds_[name] = sound;

  E2D_LOG_DEBUG("Loaded sound: {}", filePath);
  return sound;
}

std::shared_ptr<Sound> AudioEngine::getSound(const std::string &name) {
  auto it = sounds_.find(name);
  if (it != sounds_.end()) {
    return it->second;
  }
  return nullptr;
}

void AudioEngine::unloadSound(const std::string &name) {
  auto it = sounds_.find(name);
  if (it != sounds_.end()) {
    sounds_.erase(it);
    E2D_LOG_DEBUG("Unloaded sound: {}", name);
  }
}

void AudioEngine::unloadAllSounds() {
  stopAll();
  sounds_.clear();
  E2D_LOG_DEBUG("Unloaded all sounds");
}

void AudioEngine::setMasterVolume(float volume) {
  masterVolume_ = volume;
  int mixVol = static_cast<int>(volume * MIX_MAX_VOLUME);
  Mix_Volume(-1, mixVol);  // 所有通道
  Mix_VolumeMusic(mixVol); // 音乐
}

float AudioEngine::getMasterVolume() const { return masterVolume_; }

void AudioEngine::pauseAll() {
  Mix_Pause(-1); // 暂停所有通道
}

void AudioEngine::resumeAll() {
  Mix_Resume(-1); // 恢复所有通道
}

void AudioEngine::stopAll() {
  for (auto &pair : sounds_) {
    if (pair.second) {
      pair.second->stop();
    }
  }
}

} // namespace extra2d
