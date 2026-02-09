#pragma once

#include <easy2d/scene/node.h>
#include <easy2d/animation/animation_clip.h>
#include <easy2d/animation/animation_controller.h>
#include <easy2d/animation/animation_cache.h>
#include <easy2d/animation/frame_renderer.h>
#include <easy2d/animation/animation_event.h>
#include <vector>

namespace easy2d {

// ============================================================================
// AnimationNode - 动画节点（继承 Node）
// 使用 FrameRenderer 单渲染器策略，不依赖 Sprite 基类
// 适用于需要独立渲染控制的动画（如特效、复合动画图层）
// ============================================================================
class AnimationNode : public Node {
public:
    AnimationNode();
    ~AnimationNode() override = default;

    // ------ 静态工厂（Cocos 风格）------
    static Ptr<AnimationNode> create();
    static Ptr<AnimationNode> create(Ptr<AnimationClip> clip);
    static Ptr<AnimationNode> create(const std::string& aniFilePath);

    // ------ 动画数据 ------
    void setClip(Ptr<AnimationClip> clip);
    Ptr<AnimationClip> getClip() const;
    bool loadFromFile(const std::string& aniFilePath);

    // ------ 播放控制 ------
    void play();
    void pause();
    void resume();
    void stop();
    void reset();

    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    void setPlaybackSpeed(float speed);
    float getPlaybackSpeed() const;
    void setLooping(bool loop);
    bool isLooping() const;

    // ------ 帧控制 ------
    void setFrameIndex(size_t index);
    size_t getCurrentFrameIndex() const;
    size_t getTotalFrames() const;

    // ------ 事件回调 ------
    void setKeyframeCallback(KeyframeHitCallback callback);
    void setCompletionCallback(AnimationCompleteCallback callback);
    void setFrameChangeCallback(AnimationController::FrameChangeCallback callback);
    void addEventListener(AnimationEventCallback callback);

    // ------ 视觉属性 ------
    void setTintColor(const Color& color);
    Color getTintColor() const { return tintColor_; }
    void setFlipX(bool flip) { flipX_ = flip; }
    void setFlipY(bool flip) { flipY_ = flip; }
    bool isFlipX() const { return flipX_; }
    bool isFlipY() const { return flipY_; }

    // ------ 自动播放 ------
    void setAutoPlay(bool autoPlay) { autoPlay_ = autoPlay; }
    bool isAutoPlay() const { return autoPlay_; }

    // ------ 碰撞盒访问 ------
    const std::vector<std::array<int32_t, 6>>& getCurrentDamageBoxes() const;
    const std::vector<std::array<int32_t, 6>>& getCurrentAttackBoxes() const;

    // ------ 查询 ------
    Size getMaxFrameSize() const;
    Rect getBoundingBox() const override;

    // ------ 直接访问 ------
    AnimationController& getController() { return controller_; }
    const AnimationController& getController() const { return controller_; }
    FrameRenderer& getFrameRenderer() { return frameRenderer_; }
    const FrameRenderer& getFrameRenderer() const { return frameRenderer_; }

protected:
    void onUpdate(float dt) override;
    void onDraw(RenderBackend& renderer) override;
    void onEnter() override;
    void onExit() override;

private:
    AnimationController controller_;
    FrameRenderer       frameRenderer_;
    Color tintColor_ = Colors::White;
    bool  flipX_ = false;
    bool  flipY_ = false;
    bool  autoPlay_ = false;
    std::vector<AnimationEventCallback> eventListeners_;

    static const std::vector<std::array<int32_t, 6>> emptyBoxes_;

    void setupControllerCallbacks();
    void dispatchEvent(const AnimationEvent& event);
};

} // namespace easy2d
