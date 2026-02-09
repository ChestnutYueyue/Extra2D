#pragma once

#include <easy2d/scene/sprite.h>
#include <easy2d/animation/animation_controller.h>
#include <easy2d/animation/animation_cache.h>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace easy2d {

// ============================================================================
// AnimatedSprite - 动画精灵节点
// 将 AnimationController 与 Sprite 渲染桥接，接入场景图
// ============================================================================
class AnimatedSprite : public Sprite {
public:
    AnimatedSprite();
    ~AnimatedSprite() override = default;

    // ------ 静态工厂 ------
    static Ptr<AnimatedSprite> create();
    static Ptr<AnimatedSprite> create(Ptr<AnimationClip> clip);
    static Ptr<AnimatedSprite> create(const std::string& aniFilePath);

    // ------ 动画绑定 ------
    void setAnimationClip(Ptr<AnimationClip> clip);
    void loadAnimation(const std::string& aniFilePath);
    Ptr<AnimationClip> getAnimationClip() const;

    // ------ 动画字典 ------
    void addAnimation(const std::string& name, Ptr<AnimationClip> clip);
    void play(const std::string& name, bool loop = true);
    bool hasAnimation(const std::string& name) const;
    Ptr<AnimationClip> getAnimation(const std::string& name) const;
    const std::string& getCurrentAnimationName() const;

    // ------ 播放控制（委托 controller_）------
    void play();
    void pause();
    void resume();
    void stop();
    void reset();
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    // ------ 属性控制 ------
    void setLooping(bool loop);
    bool isLooping() const;
    void setPlaybackSpeed(float speed);
    float getPlaybackSpeed() const;

    // ------ 帧控制 ------
    void setFrameIndex(size_t index);
    size_t getCurrentFrameIndex() const;
    size_t getTotalFrames() const;
    void nextFrame();
    void prevFrame();

    // ------ 帧范围限制 ------
    /// 设置帧播放范围（用于精灵图动画，限制在指定范围内循环）
    /// @param start 起始帧索引（包含）
    /// @param end 结束帧索引（包含），-1表示不限制
    void setFrameRange(int start, int end = -1);
    
    /// 获取当前帧范围
    /// @return pair<起始帧, 结束帧>，结束帧为-1表示不限制
    std::pair<int, int> getFrameRange() const;
    
    /// 清除帧范围限制（恢复播放所有帧）
    void clearFrameRange();
    
    /// 检查是否设置了帧范围限制
    bool hasFrameRange() const;

    // ------ 回调 ------
    void setCompletionCallback(AnimationController::CompletionCallback cb);
    void setKeyframeCallback(AnimationController::KeyframeCallback cb);
    void setSoundTriggerCallback(AnimationController::SoundTriggerCallback cb);

    // ------ 碰撞盒访问（当前帧）------
    const std::vector<std::array<int32_t, 6>>& getCurrentDamageBoxes() const;
    const std::vector<std::array<int32_t, 6>>& getCurrentAttackBoxes() const;

    // ------ 帧变换控制 ------
    /// 设置是否由动画帧数据覆盖节点的 position/scale/rotation
    /// ANI 动画需要开启（默认），精灵图动画应关闭
    void setApplyFrameTransform(bool apply) { applyFrameTransform_ = apply; }
    bool isApplyFrameTransform() const { return applyFrameTransform_; }

    // ------ 自动播放 ------
    void setAutoPlay(bool autoPlay) { autoPlay_ = autoPlay; }
    bool isAutoPlay() const { return autoPlay_; }

    // ------ 直接控制器访问 ------
    AnimationController& getController() { return controller_; }
    const AnimationController& getController() const { return controller_; }

protected:
    void onUpdate(float dt) override;
    void onEnter() override;

private:
    AnimationController controller_;
    bool autoPlay_ = false;
    bool applyFrameTransform_ = true;

    // 动画字典
    std::unordered_map<std::string, Ptr<AnimationClip>> animations_;
    std::string currentAnimationName_;
    static const std::string emptyString_;

    // 帧范围限制（用于精灵图动画）
    int frameRangeStart_ = 0;   // 起始帧索引
    int frameRangeEnd_ = -1;    // 结束帧索引，-1表示不限制

    // 空碰撞盒列表（用于无帧时返回引用）
    static const std::vector<std::array<int32_t, 6>> emptyBoxes_;

    void applyFrame(const AnimationFrame& frame);
    void onFrameChanged(size_t oldIdx, size_t newIdx, const AnimationFrame& frame);
};

} // namespace easy2d
