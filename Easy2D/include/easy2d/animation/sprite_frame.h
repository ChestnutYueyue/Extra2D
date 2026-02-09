#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/math_types.h>
#include <easy2d/graphics/texture.h>
#include <string>

namespace easy2d {

// ============================================================================
// SpriteFrame - 精灵帧（纹理 + 区域 + 偏移的中间抽象）
// 借鉴 Cocos2d-x SpriteFrame：解耦纹理物理存储与逻辑帧
// 一个纹理图集可包含多个 SpriteFrame，减少纹理切换提升渲染性能
// ============================================================================
class SpriteFrame {
public:
    SpriteFrame() = default;

    SpriteFrame(Ptr<Texture> texture, const Rect& rect)
        : texture_(std::move(texture))
        , rect_(rect)
        , originalSize_(rect.size) {}

    SpriteFrame(Ptr<Texture> texture, const Rect& rect,
                const Vec2& offset, const Size& originalSize)
        : texture_(std::move(texture))
        , rect_(rect)
        , offset_(offset)
        , originalSize_(originalSize) {}

    // ------ 静态创建 ------
    static Ptr<SpriteFrame> create(Ptr<Texture> texture, const Rect& rect) {
        return makePtr<SpriteFrame>(std::move(texture), rect);
    }

    static Ptr<SpriteFrame> create(Ptr<Texture> texture, const Rect& rect,
                                    const Vec2& offset, const Size& originalSize) {
        return makePtr<SpriteFrame>(std::move(texture), rect, offset, originalSize);
    }

    // ------ 纹理信息 ------
    void setTexture(Ptr<Texture> texture) { texture_ = std::move(texture); }
    Ptr<Texture> getTexture() const { return texture_; }

    // ------ 矩形区域（在纹理图集中的位置）------
    void setRect(const Rect& rect) { rect_ = rect; }
    const Rect& getRect() const { return rect_; }

    // ------ 偏移（图集打包时的裁剪偏移）------
    void setOffset(const Vec2& offset) { offset_ = offset; }
    const Vec2& getOffset() const { return offset_; }

    // ------ 原始尺寸（裁剪前的完整尺寸）------
    void setOriginalSize(const Size& size) { originalSize_ = size; }
    const Size& getOriginalSize() const { return originalSize_; }

    // ------ 旋转标志（图集工具可能旋转90度）------
    void setRotated(bool rotated) { rotated_ = rotated; }
    bool isRotated() const { return rotated_; }

    // ------ 名称（用于缓存索引）------
    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_; }

    // ------ 有效性检查 ------
    bool isValid() const { return texture_ != nullptr; }

private:
    Ptr<Texture> texture_;
    Rect rect_;
    Vec2 offset_;
    Size originalSize_;
    bool rotated_ = false;
    std::string name_;
};

} // namespace easy2d
