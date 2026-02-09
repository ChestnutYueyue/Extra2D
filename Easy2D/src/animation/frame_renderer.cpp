#include <easy2d/animation/frame_renderer.h>
#include <easy2d/graphics/texture.h>

namespace easy2d {

// ============================================================================
// 预加载
// ============================================================================

bool FrameRenderer::preloadFrames(const std::vector<AnimationFrame>& frames) {
    releaseFrames();
    spriteFrames_.reserve(frames.size());
    maxFrameSize_ = Size{0.0f, 0.0f};

    for (const auto& frame : frames) {
        Ptr<SpriteFrame> sf = frame.spriteFrame;

        // 如果帧自身没有 SpriteFrame，尝试通过缓存获取
        if (!sf && !frame.texturePath.empty()) {
            sf = SpriteFrameCache::getInstance().getOrCreateFromFile(
                frame.texturePath, frame.textureIndex);
        }

        spriteFrames_.push_back(sf);

        // 更新最大帧尺寸
        if (sf && sf->isValid()) {
            const auto& rect = sf->getRect();
            if (rect.size.width > maxFrameSize_.width)
                maxFrameSize_.width = rect.size.width;
            if (rect.size.height > maxFrameSize_.height)
                maxFrameSize_.height = rect.size.height;
        }
    }

    return true;
}

void FrameRenderer::releaseFrames() {
    spriteFrames_.clear();
    maxFrameSize_ = Size{0.0f, 0.0f};
}

// ============================================================================
// 渲染当前帧
// ============================================================================

void FrameRenderer::renderFrame(RenderBackend& renderer,
                                 const AnimationFrame& frame,
                                 size_t frameIndex,
                                 const Vec2& position,
                                 float nodeOpacity,
                                 const Color& tintColor,
                                 bool flipX, bool flipY) {
    if (frameIndex >= spriteFrames_.size()) return;

    auto sf = spriteFrames_[frameIndex];
    if (!sf || !sf->isValid()) return;

    BlendMode blend = mapBlendMode(frame.properties);
    Vec2 scale = frame.getEffectiveScale();
    float rotation = frame.getEffectiveRotation();
    Color frameColor = frame.getEffectiveColor();

    // 合并帧颜色和节点染色
    Color finalTint{
        tintColor.r * frameColor.r,
        tintColor.g * frameColor.g,
        tintColor.b * frameColor.b,
        tintColor.a * frameColor.a * nodeOpacity
    };

    drawSpriteFrame(renderer, sf, position, frame.offset,
                    scale, rotation, 1.0f, finalTint,
                    flipX, flipY, blend);
}

// ============================================================================
// 渲染插值帧
// ============================================================================

void FrameRenderer::renderInterpolated(RenderBackend& renderer,
                                        const AnimationFrame& fromFrame,
                                        size_t fromIndex,
                                        const InterpolatedProperties& props,
                                        const Vec2& position,
                                        float nodeOpacity,
                                        const Color& tintColor,
                                        bool flipX, bool flipY) {
    if (fromIndex >= spriteFrames_.size()) return;

    auto sf = spriteFrames_[fromIndex];
    if (!sf || !sf->isValid()) return;

    BlendMode blend = mapBlendMode(fromFrame.properties);

    Color finalTint{
        tintColor.r * props.color.r,
        tintColor.g * props.color.g,
        tintColor.b * props.color.b,
        tintColor.a * props.color.a * nodeOpacity
    };

    drawSpriteFrame(renderer, sf, position, props.position,
                    props.scale, props.rotation, 1.0f, finalTint,
                    flipX, flipY, blend);
}

// ============================================================================
// 混合模式映射
// ============================================================================

BlendMode FrameRenderer::mapBlendMode(const FramePropertySet& props) {
    if (props.has(FramePropertyKey::BlendAdditive)) {
        auto val = props.get<bool>(FramePropertyKey::BlendAdditive);
        if (val.has_value() && val.value())
            return BlendMode::Additive;
    }
    if (props.has(FramePropertyKey::BlendLinearDodge)) {
        auto val = props.get<bool>(FramePropertyKey::BlendLinearDodge);
        if (val.has_value() && val.value())
            return BlendMode::Additive;  // 线性减淡 ≈ 加法混合
    }
    return BlendMode::Alpha;
}

// ============================================================================
// 查询
// ============================================================================

Ptr<SpriteFrame> FrameRenderer::getSpriteFrame(size_t frameIndex) const {
    if (frameIndex >= spriteFrames_.size()) return nullptr;
    return spriteFrames_[frameIndex];
}

// ============================================================================
// 内部绘制
// ============================================================================

void FrameRenderer::drawSpriteFrame(RenderBackend& renderer,
                                     Ptr<SpriteFrame> sf,
                                     const Vec2& position,
                                     const Vec2& offset,
                                     const Vec2& scale,
                                     float rotation,
                                     float opacity,
                                     const Color& tint,
                                     bool flipX, bool flipY,
                                     BlendMode blend) {
    if (!sf || !sf->isValid()) return;

    auto texture = sf->getTexture();
    if (!texture) return;

    renderer.setBlendMode(blend);

    const Rect& srcRect = sf->getRect();

    // 计算目标矩形
    float w = srcRect.size.width * std::abs(scale.x);
    float h = srcRect.size.height * std::abs(scale.y);

    Vec2 finalPos{
        position.x + offset.x,
        position.y + offset.y
    };

    // 处理翻转（通过缩放符号）
    float flipScaleX = flipX ? -1.0f : 1.0f;
    float flipScaleY = flipY ? -1.0f : 1.0f;

    Rect destRect{
        {finalPos.x - w * 0.5f * flipScaleX, finalPos.y - h * 0.5f * flipScaleY},
        {w, h}
    };

    Color finalColor{tint.r, tint.g, tint.b, tint.a * opacity};

    renderer.drawSprite(*texture, destRect, srcRect, finalColor,
                        rotation, Vec2{0.5f, 0.5f});
}

} // namespace easy2d
