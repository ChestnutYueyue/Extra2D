#include <easy2d/animation/animation_controller.h>
#include <cassert>

namespace easy2d {

void AnimationController::setClip(Ptr<AnimationClip> clip) {
    clip_ = std::move(clip);
    currentFrameIndex_ = 0;
    accumulatedTime_ = 0.0f;
    interpolating_ = false;
    interpolationFactor_ = 0.0f;
    state_ = AnimPlayState::Stopped;
}

void AnimationController::play() {
    if (!clip_ || clip_->empty()) return;
    state_ = AnimPlayState::Playing;
}

void AnimationController::pause() {
    if (state_ == AnimPlayState::Playing) {
        state_ = AnimPlayState::Paused;
    }
}

void AnimationController::resume() {
    if (state_ == AnimPlayState::Paused) {
        state_ = AnimPlayState::Playing;
    }
}

void AnimationController::stop() {
    state_ = AnimPlayState::Stopped;
    accumulatedTime_ = 0.0f;
    interpolating_ = false;
    interpolationFactor_ = 0.0f;
}

void AnimationController::reset() {
    stop();
    if (clip_ && !clip_->empty()) {
        advanceFrame(0);
    }
}

void AnimationController::setFrameIndex(size_t index) {
    if (!clip_ || index >= clip_->getFrameCount()) return;
    accumulatedTime_ = 0.0f;
    advanceFrame(index);
}

void AnimationController::nextFrame() {
    if (!clip_ || clip_->empty()) return;
    size_t next = currentFrameIndex_ + 1;
    if (next >= clip_->getFrameCount()) {
        if (isLooping()) {
            next = 0;
        } else {
            return;
        }
    }
    accumulatedTime_ = 0.0f;
    advanceFrame(next);
}

void AnimationController::prevFrame() {
    if (!clip_ || clip_->empty()) return;
    if (currentFrameIndex_ > 0) {
        accumulatedTime_ = 0.0f;
        advanceFrame(currentFrameIndex_ - 1);
    } else if (isLooping()) {
        accumulatedTime_ = 0.0f;
        advanceFrame(clip_->getFrameCount() - 1);
    }
}

void AnimationController::update(float dt) {
    if (state_ != AnimPlayState::Playing) return;
    if (!clip_ || clip_->empty()) return;

    // 累加时间（转换为毫秒）
    float dt_ms = dt * 1000.0f * playbackSpeed_;
    accumulatedTime_ += dt_ms;

    const AnimationFrame& currentFrame = clip_->getFrame(currentFrameIndex_);
    float frameDelay = currentFrame.delay;

    // 更新插值状态
    updateInterpolation();

    // 循环处理：支持一次跳过多帧（与原始 ANI 系统行为一致）
    while (accumulatedTime_ >= frameDelay) {
        accumulatedTime_ -= frameDelay;

        size_t totalFrames = clip_->getFrameCount();

        if (currentFrameIndex_ < totalFrames - 1) {
            // 推进到下一帧
            advanceFrame(currentFrameIndex_ + 1);
        } else {
            // 最后一帧播放完毕
            if (isLooping()) {
                advanceFrame(0);
            } else {
                // 动画结束
                state_ = AnimPlayState::Stopped;
                if (onComplete_) {
                    onComplete_();
                }
                return;
            }
        }

        // 更新下一帧的延迟
        frameDelay = clip_->getFrame(currentFrameIndex_).delay;
    }

    // 更新插值因子
    updateInterpolation();
}

size_t AnimationController::getTotalFrames() const {
    return clip_ ? clip_->getFrameCount() : 0;
}

const AnimationFrame& AnimationController::getCurrentFrame() const {
    assert(clip_ && currentFrameIndex_ < clip_->getFrameCount());
    return clip_->getFrame(currentFrameIndex_);
}

bool AnimationController::isLooping() const {
    if (hasLoopOverride_) return loopOverride_;
    return clip_ ? clip_->isLooping() : false;
}

void AnimationController::setLooping(bool loop) {
    hasLoopOverride_ = true;
    loopOverride_ = loop;
}

void AnimationController::advanceFrame(size_t newIndex) {
    if (!clip_ || newIndex >= clip_->getFrameCount()) return;

    size_t oldIndex = currentFrameIndex_;
    currentFrameIndex_ = newIndex;

    const AnimationFrame& frame = clip_->getFrame(newIndex);

    // 触发帧变更回调
    if (onFrameChange_) {
        onFrameChange_(oldIndex, newIndex, frame);
    }

    // 处理帧属性（关键帧、音效等）
    processFrameProperties(frame);
}

void AnimationController::processFrameProperties(const AnimationFrame& frame) {
    const auto& props = frame.properties;

    // 关键帧回调
    if (props.has(FramePropertyKey::SetFlag)) {
        auto flagIndex = props.get<int>(FramePropertyKey::SetFlag);
        if (flagIndex.has_value() && onKeyframe_) {
            onKeyframe_(flagIndex.value());
        }
    }

    // 音效触发
    if (props.has(FramePropertyKey::PlaySound)) {
        auto soundPath = props.get<std::string>(FramePropertyKey::PlaySound);
        if (soundPath.has_value() && onSoundTrigger_) {
            onSoundTrigger_(soundPath.value());
        }
    }
}

void AnimationController::updateInterpolation() {
    if (!clip_ || clip_->empty()) {
        interpolating_ = false;
        interpolationFactor_ = 0.0f;
        return;
    }

    const AnimationFrame& currentFrame = clip_->getFrame(currentFrameIndex_);

    if (currentFrame.hasInterpolation() &&
        currentFrameIndex_ + 1 < clip_->getFrameCount()) {
        interpolating_ = true;
        float frameDelay = currentFrame.delay;
        interpolationFactor_ = (frameDelay > 0.0f)
            ? math::clamp(accumulatedTime_ / frameDelay, 0.0f, 1.0f)
            : 0.0f;
    } else {
        interpolating_ = false;
        interpolationFactor_ = 0.0f;
    }
}

} // namespace easy2d
