#pragma once

#include "data.h"
#include <extra2d/extra2d.h>

namespace pushbox {

// ============================================================================
// 全局音频管理器 - 单例模式，不依赖场景生命周期
// ============================================================================
class AudioManager {
public:
    // 获取单例实例
    static AudioManager& instance();

    // 初始化音频资源
    void init();

    // 启用/禁用音效
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }

    // 播放音效
    void playManMove();
    void playBoxMove();

    // 背景音乐控制
    void playBackground();
    void pauseBackground();
    void resumeBackground();
    void stopBackground();

private:
    AudioManager() = default;
    ~AudioManager() = default;
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool initialized_ = false;
    bool enabled_ = true;

    extra2d::Ptr<extra2d::Sound> background_;
    extra2d::Ptr<extra2d::Sound> manMove_;
    extra2d::Ptr<extra2d::Sound> boxMove_;
};

} // namespace pushbox
