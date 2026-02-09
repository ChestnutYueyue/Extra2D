#pragma once

#include <easy2d/scene/node.h>
#include <easy2d/animation/animation_node.h>
#include <easy2d/animation/als_parser.h>
#include <easy2d/animation/animation_event.h>
#include <vector>

namespace easy2d {

// ============================================================================
// CompositeAnimation - ALS 多层复合动画节点
// 管理多个 AnimationNode 图层，统一控制播放
// 对应 DNF 的 ALS 格式（多层动画叠加）
// ============================================================================
class CompositeAnimation : public Node {
public:
    CompositeAnimation() = default;
    ~CompositeAnimation() override = default;

    // ------ 静态工厂 ------
    static Ptr<CompositeAnimation> create();
    static Ptr<CompositeAnimation> create(const std::string& alsFilePath);

    // ------ 加载 ------
    bool loadFromFile(const std::string& alsFilePath);

    // ------ 图层管理 ------
    void addLayer(Ptr<AnimationNode> node, int zOrder = 0);
    void removeLayer(size_t index);
    Ptr<AnimationNode> getLayer(size_t index) const;
    Ptr<AnimationNode> getMainLayer() const;
    size_t getLayerCount() const;

    // ------ 统一播放控制 ------
    void play();
    void pause();
    void resume();
    void stop();
    void reset();
    void setPlaybackSpeed(float speed);
    void setLooping(bool loop);

    bool isPlaying() const;
    bool isStopped() const;

    // ------ 事件回调（绑定到主图层）------
    void setKeyframeCallback(KeyframeHitCallback callback);
    void setCompletionCallback(AnimationCompleteCallback callback);
    void addEventListener(AnimationEventCallback callback);

    // ------ 视觉属性（应用到所有图层）------
    void setTintColor(const Color& color);
    void setFlipX(bool flip);
    void setFlipY(bool flip);

private:
    struct LayerEntry {
        Ptr<AnimationNode> node;
        int zOrder = 0;
    };
    std::vector<LayerEntry> layers_;
};

} // namespace easy2d
