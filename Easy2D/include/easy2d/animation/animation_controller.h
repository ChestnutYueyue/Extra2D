#pragma once

#include <easy2d/core/types.h>
#include <easy2d/animation/animation_clip.h>
#include <easy2d/animation/interpolation_engine.h>
#include <functional>
#include <string>

namespace easy2d {

// ============================================================================
// 动画播放状态
// ============================================================================
enum class AnimPlayState : uint8 {
    Stopped,
    Playing,
    Paused
};

// ============================================================================
// AnimationController - 动画播放控制器
// 借鉴 Cocos Creator 的 AnimationState：纯播放逻辑，不持有渲染资源
// ============================================================================
class AnimationController {
public:
    // 回调类型定义
    using FrameChangeCallback  = std::function<void(size_t oldIdx, size_t newIdx,
                                                     const AnimationFrame& frame)>;
    using KeyframeCallback     = std::function<void(int flagIndex)>;
    using SoundTriggerCallback = std::function<void(const std::string& path)>;
    using CompletionCallback   = std::function<void()>;

    AnimationController() = default;

    // ------ 绑定动画数据 ------
    void setClip(Ptr<AnimationClip> clip);
    Ptr<AnimationClip> getClip() const { return clip_; }

    // ------ 播放控制 ------
    void play();
    void pause();
    void resume();
    void stop();
    void reset();

    // ------ 帧控制 ------
    void setFrameIndex(size_t index);
    void nextFrame();
    void prevFrame();

    // ------ 核心更新（每帧调用）------
    void update(float dt);

    // ------ 状态查询 ------
    AnimPlayState getState() const { return state_; }
    bool isPlaying() const { return state_ == AnimPlayState::Playing; }
    bool isPaused()  const { return state_ == AnimPlayState::Paused; }
    bool isStopped() const { return state_ == AnimPlayState::Stopped; }

    size_t getCurrentFrameIndex() const { return currentFrameIndex_; }
    size_t getTotalFrames() const;
    const AnimationFrame& getCurrentFrame() const;

    float getPlaybackSpeed() const { return playbackSpeed_; }
    void  setPlaybackSpeed(float speed) { playbackSpeed_ = speed; }

    bool isLooping() const;
    void setLooping(bool loop);

    // ------ 插值状态 ------
    float getInterpolationFactor() const { return interpolationFactor_; }
    bool  isInterpolating() const { return interpolating_; }

    // ------ 回调注册 ------
    void setFrameChangeCallback(FrameChangeCallback cb) { onFrameChange_ = std::move(cb); }
    void setKeyframeCallback(KeyframeCallback cb) { onKeyframe_ = std::move(cb); }
    void setSoundTriggerCallback(SoundTriggerCallback cb) { onSoundTrigger_ = std::move(cb); }
    void setCompletionCallback(CompletionCallback cb) { onComplete_ = std::move(cb); }

private:
    Ptr<AnimationClip> clip_;
    AnimPlayState state_ = AnimPlayState::Stopped;

    size_t currentFrameIndex_ = 0;
    float  accumulatedTime_   = 0.0f;   // 当前帧已累积时间 (ms)
    float  playbackSpeed_     = 1.0f;
    bool   loopOverride_      = false;  // 外部循环覆盖值
    bool   hasLoopOverride_   = false;  // 是否使用外部循环覆盖

    // 插值状态
    bool  interpolating_       = false;
    float interpolationFactor_ = 0.0f;

    // 回调
    FrameChangeCallback  onFrameChange_;
    KeyframeCallback     onKeyframe_;
    SoundTriggerCallback onSoundTrigger_;
    CompletionCallback   onComplete_;

    // 内部方法
    void advanceFrame(size_t newIndex);
    void processFrameProperties(const AnimationFrame& frame);
    void updateInterpolation();
};

} // namespace easy2d
