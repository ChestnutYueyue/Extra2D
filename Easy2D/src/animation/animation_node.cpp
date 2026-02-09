#include <easy2d/animation/animation_node.h>
#include <easy2d/animation/interpolation_engine.h>

namespace easy2d {

const std::vector<std::array<int32_t, 6>> AnimationNode::emptyBoxes_;

// ============================================================================
// 构造
// ============================================================================

AnimationNode::AnimationNode() {
    setupControllerCallbacks();
}

// ============================================================================
// 静态工厂
// ============================================================================

Ptr<AnimationNode> AnimationNode::create() {
    return makePtr<AnimationNode>();
}

Ptr<AnimationNode> AnimationNode::create(Ptr<AnimationClip> clip) {
    auto node = makePtr<AnimationNode>();
    node->setClip(std::move(clip));
    return node;
}

Ptr<AnimationNode> AnimationNode::create(const std::string& aniFilePath) {
    auto node = makePtr<AnimationNode>();
    node->loadFromFile(aniFilePath);
    return node;
}

// ============================================================================
// 动画数据
// ============================================================================

void AnimationNode::setClip(Ptr<AnimationClip> clip) {
    controller_.setClip(clip);
    if (clip && !clip->empty()) {
        // 预加载所有帧的 SpriteFrame
        std::vector<AnimationFrame> frames;
        frames.reserve(clip->getFrameCount());
        for (size_t i = 0; i < clip->getFrameCount(); ++i) {
            frames.push_back(clip->getFrame(i));
        }
        frameRenderer_.preloadFrames(frames);
    } else {
        frameRenderer_.releaseFrames();
    }
}

Ptr<AnimationClip> AnimationNode::getClip() const {
    return controller_.getClip();
}

bool AnimationNode::loadFromFile(const std::string& aniFilePath) {
    auto clip = AnimationCache::getInstance().loadClip(aniFilePath);
    if (clip) {
        setClip(clip);
        return true;
    }
    return false;
}

// ============================================================================
// 播放控制
// ============================================================================

void AnimationNode::play()   { controller_.play(); }
void AnimationNode::pause()  { controller_.pause(); }
void AnimationNode::resume() { controller_.resume(); }
void AnimationNode::stop()   { controller_.stop(); }
void AnimationNode::reset()  { controller_.reset(); }

bool AnimationNode::isPlaying() const { return controller_.isPlaying(); }
bool AnimationNode::isPaused()  const { return controller_.isPaused(); }
bool AnimationNode::isStopped() const { return controller_.isStopped(); }

void AnimationNode::setPlaybackSpeed(float speed) { controller_.setPlaybackSpeed(speed); }
float AnimationNode::getPlaybackSpeed() const { return controller_.getPlaybackSpeed(); }
void AnimationNode::setLooping(bool loop) { controller_.setLooping(loop); }
bool AnimationNode::isLooping() const { return controller_.isLooping(); }

// ============================================================================
// 帧控制
// ============================================================================

void AnimationNode::setFrameIndex(size_t index) { controller_.setFrameIndex(index); }
size_t AnimationNode::getCurrentFrameIndex() const { return controller_.getCurrentFrameIndex(); }
size_t AnimationNode::getTotalFrames() const { return controller_.getTotalFrames(); }

// ============================================================================
// 事件回调
// ============================================================================

void AnimationNode::setKeyframeCallback(KeyframeHitCallback callback) {
    controller_.setKeyframeCallback([this, cb = std::move(callback)](int flagIndex) {
        if (cb) cb(flagIndex);
        AnimationEvent evt;
        evt.type = AnimationEventType::KeyframeHit;
        evt.frameIndex = controller_.getCurrentFrameIndex();
        evt.keyframeFlag = flagIndex;
        evt.source = this;
        dispatchEvent(evt);
    });
}

void AnimationNode::setCompletionCallback(AnimationCompleteCallback callback) {
    controller_.setCompletionCallback([this, cb = std::move(callback)]() {
        if (cb) cb();
        AnimationEvent evt;
        evt.type = AnimationEventType::AnimationEnd;
        evt.frameIndex = controller_.getCurrentFrameIndex();
        evt.source = this;
        dispatchEvent(evt);
    });
}

void AnimationNode::setFrameChangeCallback(AnimationController::FrameChangeCallback callback) {
    // 保存外部回调，在 setupControllerCallbacks 中已设置内部回调
    // 需要重新绑定，将两者合并
    controller_.setFrameChangeCallback(
        [this, cb = std::move(callback)](size_t oldIdx, size_t newIdx, const AnimationFrame& frame) {
            if (cb) cb(oldIdx, newIdx, frame);
            AnimationEvent evt;
            evt.type = AnimationEventType::FrameChanged;
            evt.frameIndex = newIdx;
            evt.previousFrameIndex = oldIdx;
            evt.source = this;
            dispatchEvent(evt);
        });
}

void AnimationNode::addEventListener(AnimationEventCallback callback) {
    eventListeners_.push_back(std::move(callback));
}

// ============================================================================
// 视觉属性
// ============================================================================

void AnimationNode::setTintColor(const Color& color) {
    tintColor_ = color;
}

// ============================================================================
// 碰撞盒
// ============================================================================

const std::vector<std::array<int32_t, 6>>& AnimationNode::getCurrentDamageBoxes() const {
    auto clip = controller_.getClip();
    if (!clip || clip->empty()) return emptyBoxes_;
    return clip->getFrame(controller_.getCurrentFrameIndex()).damageBoxes;
}

const std::vector<std::array<int32_t, 6>>& AnimationNode::getCurrentAttackBoxes() const {
    auto clip = controller_.getClip();
    if (!clip || clip->empty()) return emptyBoxes_;
    return clip->getFrame(controller_.getCurrentFrameIndex()).attackBoxes;
}

// ============================================================================
// 查询
// ============================================================================

Size AnimationNode::getMaxFrameSize() const {
    return frameRenderer_.getMaxFrameSize();
}

Rect AnimationNode::getBoundingBox() const {
    Size size = frameRenderer_.getMaxFrameSize();
    Vec2 pos = getPosition();
    Vec2 anchor = getAnchor();
    return Rect{
        {pos.x - size.width * anchor.x, pos.y - size.height * anchor.y},
        size
    };
}

// ============================================================================
// 生命周期
// ============================================================================

void AnimationNode::onEnter() {
    Node::onEnter();
    if (autoPlay_ && controller_.getClip() && !controller_.getClip()->empty()) {
        play();
    }
}

void AnimationNode::onExit() {
    Node::onExit();
}

void AnimationNode::onUpdate(float dt) {
    Node::onUpdate(dt);
    controller_.update(dt);
}

void AnimationNode::onDraw(RenderBackend& renderer) {
    auto clip = controller_.getClip();
    if (!clip || clip->empty()) return;

    size_t idx = controller_.getCurrentFrameIndex();
    const auto& frame = clip->getFrame(idx);
    Vec2 pos = getPosition();

    if (controller_.isInterpolating() && idx + 1 < clip->getFrameCount()) {
        auto props = InterpolationEngine::interpolate(
            frame,
            clip->getFrame(idx + 1),
            controller_.getInterpolationFactor());

        frameRenderer_.renderInterpolated(renderer, frame, idx, props,
                                           pos, getOpacity(), tintColor_,
                                           flipX_, flipY_);
    } else {
        frameRenderer_.renderFrame(renderer, frame, idx,
                                    pos, getOpacity(), tintColor_,
                                    flipX_, flipY_);
    }
}

// ============================================================================
// 内部方法
// ============================================================================

void AnimationNode::setupControllerCallbacks() {
    controller_.setFrameChangeCallback(
        [this](size_t oldIdx, size_t newIdx, const AnimationFrame& frame) {
            AnimationEvent evt;
            evt.type = AnimationEventType::FrameChanged;
            evt.frameIndex = newIdx;
            evt.previousFrameIndex = oldIdx;
            evt.source = this;
            dispatchEvent(evt);
        });

    controller_.setSoundTriggerCallback(
        [this](const std::string& path) {
            AnimationEvent evt;
            evt.type = AnimationEventType::SoundTrigger;
            evt.frameIndex = controller_.getCurrentFrameIndex();
            evt.soundPath = path;
            evt.source = this;
            dispatchEvent(evt);
        });
}

void AnimationNode::dispatchEvent(const AnimationEvent& event) {
    for (auto& listener : eventListeners_) {
        if (listener) listener(event);
    }
}

} // namespace easy2d
