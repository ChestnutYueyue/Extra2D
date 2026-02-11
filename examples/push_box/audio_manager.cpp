#include "audio_manager.h"

#include "storage.h"

namespace pushbox {

// ============================================================================
// 单例实现
// ============================================================================
AudioManager& AudioManager::instance() {
    static AudioManager instance;
    return instance;
}

// ============================================================================
// 初始化音频资源
// ============================================================================
void AudioManager::init() {
    if (initialized_) {
        return;
    }

    auto& resources = extra2d::Application::instance().resources();

    // 加载音效资源
    background_ = resources.loadSound("pushbox_bg", "assets/audio/background.wav");
    manMove_ = resources.loadSound("pushbox_manmove", "assets/audio/manmove.wav");
    boxMove_ = resources.loadSound("pushbox_boxmove", "assets/audio/boxmove.wav");

    // 设置背景音乐循环播放
    if (background_) {
        background_->setLooping(true);
    }

    // 从存储中读取音效设置
    enabled_ = g_SoundOpen;

    initialized_ = true;

    // 如果音效开启，播放背景音乐
    if (enabled_ && background_) {
        background_->play();
    }
}

// ============================================================================
// 启用/禁用音效
// ============================================================================
void AudioManager::setEnabled(bool enabled) {
    enabled_ = enabled;
    g_SoundOpen = enabled;
    saveSoundOpen(enabled);

    if (!background_) {
        return;
    }

    if (enabled_) {
        background_->resume();
    } else {
        background_->pause();
    }
}

// ============================================================================
// 播放角色移动音效
// ============================================================================
void AudioManager::playManMove() {
    if (!enabled_ || !manMove_) {
        return;
    }
    manMove_->play();
}

// ============================================================================
// 播放箱子移动音效
// ============================================================================
void AudioManager::playBoxMove() {
    if (!enabled_ || !boxMove_) {
        return;
    }
    boxMove_->play();
}

// ============================================================================
// 背景音乐控制
// ============================================================================
void AudioManager::playBackground() {
    if (background_) {
        background_->play();
    }
}

void AudioManager::pauseBackground() {
    if (background_) {
        background_->pause();
    }
}

void AudioManager::resumeBackground() {
    if (background_) {
        background_->resume();
    }
}

void AudioManager::stopBackground() {
    if (background_) {
        background_->stop();
    }
}

} // namespace pushbox
