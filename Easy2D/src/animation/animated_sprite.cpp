#include <easy2d/animation/animated_sprite.h>
#include <easy2d/animation/interpolation_engine.h>

namespace easy2d {

const std::vector<std::array<int32_t, 6>> AnimatedSprite::emptyBoxes_;
const std::string AnimatedSprite::emptyString_;

AnimatedSprite::AnimatedSprite() {
    controller_.setFrameChangeCallback(
        [this](size_t oldIdx, size_t newIdx, const AnimationFrame& frame) {
            onFrameChanged(oldIdx, newIdx, frame);
        });
}

// ------ 静态工厂 ------

Ptr<AnimatedSprite> AnimatedSprite::create() {
    return makePtr<AnimatedSprite>();
}

Ptr<AnimatedSprite> AnimatedSprite::create(Ptr<AnimationClip> clip) {
    auto sprite = makePtr<AnimatedSprite>();
    sprite->setAnimationClip(std::move(clip));
    return sprite;
}

Ptr<AnimatedSprite> AnimatedSprite::create(const std::string& aniFilePath) {
    auto sprite = makePtr<AnimatedSprite>();
    sprite->loadAnimation(aniFilePath);
    return sprite;
}

// ------ 动画绑定 ------

void AnimatedSprite::setAnimationClip(Ptr<AnimationClip> clip) {
    controller_.setClip(clip);
    if (clip && !clip->empty()) {
        applyFrame(clip->getFrame(0));
    }
}

void AnimatedSprite::loadAnimation(const std::string& aniFilePath) {
    auto clip = AnimationCache::getInstance().loadClip(aniFilePath);
    if (clip) {
        setAnimationClip(clip);
    }
}

Ptr<AnimationClip> AnimatedSprite::getAnimationClip() const {
    return controller_.getClip();
}

// ------ 动画字典 ------

void AnimatedSprite::addAnimation(const std::string& name, Ptr<AnimationClip> clip) {
    if (clip) {
        animations_[name] = std::move(clip);
    }
}

void AnimatedSprite::play(const std::string& name, bool loop) {
    auto it = animations_.find(name);
    if (it == animations_.end()) return;

    currentAnimationName_ = name;
    // 精灵图动画不应覆盖节点的 position/scale/rotation
    applyFrameTransform_ = false;
    setAnimationClip(it->second);
    setLooping(loop);
    play();
}

bool AnimatedSprite::hasAnimation(const std::string& name) const {
    return animations_.find(name) != animations_.end();
}

Ptr<AnimationClip> AnimatedSprite::getAnimation(const std::string& name) const {
    auto it = animations_.find(name);
    if (it != animations_.end()) return it->second;
    return nullptr;
}

const std::string& AnimatedSprite::getCurrentAnimationName() const {
    return currentAnimationName_;
}

// ------ 播放控制 ------

void AnimatedSprite::play() { controller_.play(); }
void AnimatedSprite::pause() { controller_.pause(); }
void AnimatedSprite::resume() { controller_.resume(); }
void AnimatedSprite::stop() { controller_.stop(); }
void AnimatedSprite::reset() { controller_.reset(); }
bool AnimatedSprite::isPlaying() const { return controller_.isPlaying(); }
bool AnimatedSprite::isPaused() const { return controller_.isPaused(); }
bool AnimatedSprite::isStopped() const { return controller_.isStopped(); }

// ------ 属性控制 ------

void AnimatedSprite::setLooping(bool loop) { controller_.setLooping(loop); }
bool AnimatedSprite::isLooping() const { return controller_.isLooping(); }
void AnimatedSprite::setPlaybackSpeed(float speed) { controller_.setPlaybackSpeed(speed); }
float AnimatedSprite::getPlaybackSpeed() const { return controller_.getPlaybackSpeed(); }

// ------ 帧控制 ------

void AnimatedSprite::setFrameIndex(size_t index) { controller_.setFrameIndex(index); }
size_t AnimatedSprite::getCurrentFrameIndex() const { return controller_.getCurrentFrameIndex(); }
size_t AnimatedSprite::getTotalFrames() const { return controller_.getTotalFrames(); }
void AnimatedSprite::nextFrame() { controller_.nextFrame(); }
void AnimatedSprite::prevFrame() { controller_.prevFrame(); }

// ------ 帧范围限制 ------

void AnimatedSprite::setFrameRange(int start, int end) {
    frameRangeStart_ = start;
    frameRangeEnd_ = end;
    
    // 确保当前帧在新的范围内
    auto clip = controller_.getClip();
    if (clip && !clip->empty()) {
        size_t currentFrame = controller_.getCurrentFrameIndex();
        size_t minFrame = static_cast<size_t>(start);
        size_t maxFrame = (end < 0) ? clip->getFrameCount() - 1 : static_cast<size_t>(end);
        
        if (currentFrame < minFrame || currentFrame > maxFrame) {
            controller_.setFrameIndex(minFrame);
        }
    }
}

std::pair<int, int> AnimatedSprite::getFrameRange() const {
    return {frameRangeStart_, frameRangeEnd_};
}

void AnimatedSprite::clearFrameRange() {
    frameRangeStart_ = 0;
    frameRangeEnd_ = -1;
}

bool AnimatedSprite::hasFrameRange() const {
    return frameRangeEnd_ >= 0;
}

// ------ 回调 ------

void AnimatedSprite::setCompletionCallback(AnimationController::CompletionCallback cb) {
    controller_.setCompletionCallback(std::move(cb));
}

void AnimatedSprite::setKeyframeCallback(AnimationController::KeyframeCallback cb) {
    controller_.setKeyframeCallback(std::move(cb));
}

void AnimatedSprite::setSoundTriggerCallback(AnimationController::SoundTriggerCallback cb) {
    controller_.setSoundTriggerCallback(std::move(cb));
}

// ------ 碰撞盒访问 ------

const std::vector<std::array<int32_t, 6>>& AnimatedSprite::getCurrentDamageBoxes() const {
    auto clip = controller_.getClip();
    if (!clip || clip->empty()) return emptyBoxes_;
    return clip->getFrame(controller_.getCurrentFrameIndex()).damageBoxes;
}

const std::vector<std::array<int32_t, 6>>& AnimatedSprite::getCurrentAttackBoxes() const {
    auto clip = controller_.getClip();
    if (!clip || clip->empty()) return emptyBoxes_;
    return clip->getFrame(controller_.getCurrentFrameIndex()).attackBoxes;
}

// ------ 生命周期 ------

void AnimatedSprite::onEnter() {
    Sprite::onEnter();
    if (autoPlay_ && controller_.getClip() && !controller_.getClip()->empty()) {
        play();
    }
}

void AnimatedSprite::onUpdate(float dt) {
    Sprite::onUpdate(dt);
    
    // 保存更新前的帧索引
    size_t prevFrameIdx = controller_.getCurrentFrameIndex();
    
    controller_.update(dt);
    
    // 应用帧范围限制
    if (hasFrameRange() && controller_.isPlaying()) {
        size_t currentFrame = controller_.getCurrentFrameIndex();
        size_t minFrame = static_cast<size_t>(frameRangeStart_);
        size_t maxFrame = static_cast<size_t>(frameRangeEnd_);
        
        auto clip = controller_.getClip();
        if (clip && !clip->empty()) {
            // 确保范围有效
            if (maxFrame >= clip->getFrameCount()) {
                maxFrame = clip->getFrameCount() - 1;
            }
            
            // 如果超出范围，回到起始帧
            if (currentFrame < minFrame || currentFrame > maxFrame) {
                controller_.setFrameIndex(minFrame);
            }
        }
    }

    // 插值处理
    if (controller_.isInterpolating()) {
        auto clip = controller_.getClip();
        if (clip) {
            size_t idx = controller_.getCurrentFrameIndex();
            if (idx + 1 < clip->getFrameCount()) {
                auto props = InterpolationEngine::interpolate(
                    clip->getFrame(idx),
                    clip->getFrame(idx + 1),
                    controller_.getInterpolationFactor());
                // 仅更新插值属性，不覆盖帧纹理
                Sprite::setColor(props.color);
            }
        }
    }
}

// ------ 内部方法 ------

void AnimatedSprite::onFrameChanged(size_t /*oldIdx*/, size_t /*newIdx*/,
                                     const AnimationFrame& frame) {
    applyFrame(frame);
}

void AnimatedSprite::applyFrame(const AnimationFrame& frame) {
    // 更新纹理
    if (frame.spriteFrame && frame.spriteFrame->isValid()) {
        Sprite::setTexture(frame.spriteFrame->getTexture());
        Sprite::setTextureRect(frame.spriteFrame->getRect());
    }

    // 帧变换仅在 ANI 动画模式下应用；精灵图模式跳过，避免覆盖节点世界坐标
    if (applyFrameTransform_) {
        // 应用帧偏移（作为精灵位置）
        Node::setPosition(frame.offset);

        // 应用缩放
        Vec2 scale = frame.getEffectiveScale();
        Node::setScale(scale);

        // 应用旋转
        float rotation = frame.getEffectiveRotation();
        Node::setRotation(rotation);

        // 应用翻转
        auto flipType = frame.properties.get<int>(FramePropertyKey::FlipType);
        if (flipType.has_value()) {
            int flip = flipType.value();
            Sprite::setFlipX(flip == 1 || flip == 3);
            Sprite::setFlipY(flip == 2 || flip == 3);
        } else {
            Sprite::setFlipX(false);
            Sprite::setFlipY(false);
        }
    }

    // 颜色始终应用
    Color color = frame.getEffectiveColor();
    Sprite::setColor(color);
}

} // namespace easy2d
