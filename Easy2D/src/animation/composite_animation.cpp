#include <easy2d/animation/composite_animation.h>
#include <easy2d/animation/animation_cache.h>

namespace easy2d {

// ============================================================================
// 静态工厂
// ============================================================================

Ptr<CompositeAnimation> CompositeAnimation::create() {
    return makePtr<CompositeAnimation>();
}

Ptr<CompositeAnimation> CompositeAnimation::create(const std::string& alsFilePath) {
    auto comp = makePtr<CompositeAnimation>();
    comp->loadFromFile(alsFilePath);
    return comp;
}

// ============================================================================
// 加载 ALS 文件
// ============================================================================

bool CompositeAnimation::loadFromFile(const std::string& alsFilePath) {
    AlsParser parser;
    auto result = parser.parse(alsFilePath);
    if (!result.success) return false;

    // 清除现有图层
    for (auto& entry : layers_) {
        if (entry.node) {
            removeChild(entry.node);
        }
    }
    layers_.clear();

    // 创建每个图层
    for (const auto& layer : result.layers) {
        auto node = AnimationNode::create();
        if (node->loadFromFile(layer.aniPath)) {
            node->setPosition(layer.offset);
            addLayer(node, layer.zOrder);
        }
    }

    return !layers_.empty();
}

// ============================================================================
// 图层管理
// ============================================================================

void CompositeAnimation::addLayer(Ptr<AnimationNode> node, int zOrder) {
    if (!node) return;

    node->setZOrder(zOrder);
    addChild(node);
    layers_.push_back({node, zOrder});
}

void CompositeAnimation::removeLayer(size_t index) {
    if (index >= layers_.size()) return;

    auto& entry = layers_[index];
    if (entry.node) {
        removeChild(entry.node);
    }
    layers_.erase(layers_.begin() + static_cast<ptrdiff_t>(index));
}

Ptr<AnimationNode> CompositeAnimation::getLayer(size_t index) const {
    if (index >= layers_.size()) return nullptr;
    return layers_[index].node;
}

Ptr<AnimationNode> CompositeAnimation::getMainLayer() const {
    if (layers_.empty()) return nullptr;
    return layers_[0].node;
}

size_t CompositeAnimation::getLayerCount() const {
    return layers_.size();
}

// ============================================================================
// 统一播放控制
// ============================================================================

void CompositeAnimation::play() {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->play();
    }
}

void CompositeAnimation::pause() {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->pause();
    }
}

void CompositeAnimation::resume() {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->resume();
    }
}

void CompositeAnimation::stop() {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->stop();
    }
}

void CompositeAnimation::reset() {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->reset();
    }
}

void CompositeAnimation::setPlaybackSpeed(float speed) {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->setPlaybackSpeed(speed);
    }
}

void CompositeAnimation::setLooping(bool loop) {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->setLooping(loop);
    }
}

bool CompositeAnimation::isPlaying() const {
    auto main = getMainLayer();
    return main ? main->isPlaying() : false;
}

bool CompositeAnimation::isStopped() const {
    auto main = getMainLayer();
    return main ? main->isStopped() : true;
}

// ============================================================================
// 事件回调（绑定到主图层）
// ============================================================================

void CompositeAnimation::setKeyframeCallback(KeyframeHitCallback callback) {
    auto main = getMainLayer();
    if (main) main->setKeyframeCallback(std::move(callback));
}

void CompositeAnimation::setCompletionCallback(AnimationCompleteCallback callback) {
    auto main = getMainLayer();
    if (main) main->setCompletionCallback(std::move(callback));
}

void CompositeAnimation::addEventListener(AnimationEventCallback callback) {
    auto main = getMainLayer();
    if (main) main->addEventListener(std::move(callback));
}

// ============================================================================
// 视觉属性（应用到所有图层）
// ============================================================================

void CompositeAnimation::setTintColor(const Color& color) {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->setTintColor(color);
    }
}

void CompositeAnimation::setFlipX(bool flip) {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->setFlipX(flip);
    }
}

void CompositeAnimation::setFlipY(bool flip) {
    for (auto& entry : layers_) {
        if (entry.node) entry.node->setFlipY(flip);
    }
}

} // namespace easy2d
