#pragma once

#include <easy2d/animation/animation_frame.h>
#include <easy2d/animation/sprite_frame.h>
#include <easy2d/animation/sprite_frame_cache.h>
#include <easy2d/animation/interpolation_engine.h>
#include <easy2d/graphics/render_backend.h>
#include <vector>

namespace easy2d {

// ============================================================================
// FrameRenderer - 帧渲染器
// 单渲染器 + SpriteFrame 引用策略，替代 N帧=N个Sprite 的旧设计
// 负责预加载帧的 SpriteFrame、渲染当前帧、处理混合模式
// ============================================================================
class FrameRenderer {
public:
    FrameRenderer() = default;

    // ------ 预加载 ------
    // 解析所有帧的 SpriteFrame（通过 SpriteFrameCache）
    bool preloadFrames(const std::vector<AnimationFrame>& frames);
    void releaseFrames();

    // ------ 渲染当前帧 ------
    void renderFrame(RenderBackend& renderer,
                     const AnimationFrame& frame,
                     size_t frameIndex,
                     const Vec2& position,
                     float nodeOpacity,
                     const Color& tintColor,
                     bool flipX, bool flipY);

    // ------ 渲染插值帧 ------
    void renderInterpolated(RenderBackend& renderer,
                            const AnimationFrame& fromFrame,
                            size_t fromIndex,
                            const InterpolatedProperties& props,
                            const Vec2& position,
                            float nodeOpacity,
                            const Color& tintColor,
                            bool flipX, bool flipY);

    // ------ 混合模式映射 ------
    static BlendMode mapBlendMode(const FramePropertySet& props);

    // ------ 查询 ------
    Ptr<SpriteFrame> getSpriteFrame(size_t frameIndex) const;
    Size getMaxFrameSize() const { return maxFrameSize_; }
    bool isLoaded() const { return !spriteFrames_.empty(); }

private:
    std::vector<Ptr<SpriteFrame>> spriteFrames_;
    Size maxFrameSize_;

    void drawSpriteFrame(RenderBackend& renderer,
                         Ptr<SpriteFrame> sf,
                         const Vec2& position,
                         const Vec2& offset,
                         const Vec2& scale,
                         float rotation,
                         float opacity,
                         const Color& tint,
                         bool flipX, bool flipY,
                         BlendMode blend);
};

} // namespace easy2d
