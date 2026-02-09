#pragma once

#include <easy2d/animation/animation_frame.h>
#include <easy2d/animation/sprite_frame_cache.h>
#include <vector>
#include <string>
#include <cassert>

namespace easy2d {

// ============================================================================
// AnimationClip - 动画片段（纯数据，可复用）
// 借鉴 Cocos：一份 AnimationClip 可被多个 AnimationNode 同时使用
// ============================================================================
class AnimationClip {
public:
    AnimationClip() = default;

    explicit AnimationClip(const std::string& name)
        : name_(name) {}

    // ------ 帧管理 ------
    void addFrame(const AnimationFrame& frame) {
        frames_.push_back(frame);
    }

    void addFrame(AnimationFrame&& frame) {
        frames_.push_back(std::move(frame));
    }

    void insertFrame(size_t index, const AnimationFrame& frame) {
        assert(index <= frames_.size());
        frames_.insert(frames_.begin() + static_cast<ptrdiff_t>(index), frame);
    }

    void removeFrame(size_t index) {
        assert(index < frames_.size());
        frames_.erase(frames_.begin() + static_cast<ptrdiff_t>(index));
    }

    void clearFrames() {
        frames_.clear();
    }

    const AnimationFrame& getFrame(size_t index) const {
        assert(index < frames_.size());
        return frames_[index];
    }

    AnimationFrame& getFrame(size_t index) {
        assert(index < frames_.size());
        return frames_[index];
    }

    size_t getFrameCount() const { return frames_.size(); }
    bool empty() const { return frames_.empty(); }

    // ------ 全局属性（对应原始 AnimationFlag）------
    FramePropertySet& globalProperties() { return globalProperties_; }
    const FramePropertySet& globalProperties() const { return globalProperties_; }

    bool isLooping() const {
        return globalProperties_.getOr<bool>(FramePropertyKey::Loop, false);
    }

    void setLooping(bool loop) {
        globalProperties_.withLoop(loop);
    }

    // ------ 时间信息 ------
    float getTotalDuration() const {
        float total = 0.0f;
        for (const auto& frame : frames_) {
            total += frame.delay;
        }
        return total;
    }

    // ------ 预计算最大帧尺寸 ------
    Size getMaxFrameSize() const {
        Size maxSize;
        for (const auto& frame : frames_) {
            if (frame.spriteFrame && frame.spriteFrame->isValid()) {
                const auto& rect = frame.spriteFrame->getRect();
                if (rect.size.width > maxSize.width)
                    maxSize.width = rect.size.width;
                if (rect.size.height > maxSize.height)
                    maxSize.height = rect.size.height;
            }
        }
        return maxSize;
    }

    // ------ 元数据 ------
    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_; }

    void setSourcePath(const std::string& path) { sourcePath_ = path; }
    const std::string& getSourcePath() const { return sourcePath_; }

    // ------ 静态工厂 ------
    static Ptr<AnimationClip> create(const std::string& name = "") {
        return makePtr<AnimationClip>(name);
    }

    /// 从精灵图网格创建（所有帧按顺序）
    static Ptr<AnimationClip> createFromGrid(
        Ptr<Texture> texture, int frameWidth, int frameHeight,
        float frameDurationMs = 100.0f, int frameCount = -1,
        int spacing = 0, int margin = 0)
    {
        if (!texture) return nullptr;

        int texW = texture->getWidth();
        int texH = texture->getHeight();
        int usableW = texW - 2 * margin;
        int usableH = texH - 2 * margin;
        int cols = (usableW + spacing) / (frameWidth + spacing);
        int rows = (usableH + spacing) / (frameHeight + spacing);
        int total = (frameCount > 0) ? frameCount : cols * rows;

        auto clip = makePtr<AnimationClip>();
        for (int i = 0; i < total; ++i) {
            int col = i % cols;
            int row = i / cols;
            if (row >= rows) break;

            // 翻转行顺序：精灵图第0行在顶部，但OpenGL纹理V坐标从底部开始
            // 所以将行索引翻转，使第0行对应纹理底部（V=1.0），第3行对应纹理顶部（V=0.0）
            int flippedRow = (rows - 1) - row;

            Rect rect(
                static_cast<float>(margin + col * (frameWidth + spacing)),
                static_cast<float>(margin + flippedRow * (frameHeight + spacing)),
                static_cast<float>(frameWidth),
                static_cast<float>(frameHeight)
            );

            auto sf = SpriteFrame::create(texture, rect);
            AnimationFrame frame;
            frame.spriteFrame = std::move(sf);
            frame.delay = frameDurationMs;
            clip->addFrame(std::move(frame));
        }
        return clip;
    }

    /// 从精灵图网格创建（指定帧索引列表）
    static Ptr<AnimationClip> createFromGridIndices(
        Ptr<Texture> texture, int frameWidth, int frameHeight,
        const std::vector<int>& frameIndices,
        float frameDurationMs = 100.0f,
        int spacing = 0, int margin = 0)
    {
        if (!texture) return nullptr;

        int texW = texture->getWidth();
        int texH = texture->getHeight();
        int usableW = texW - 2 * margin;
        int usableH = texH - 2 * margin;
        int cols = (usableW + spacing) / (frameWidth + spacing);
        int rows = (usableH + spacing) / (frameHeight + spacing);

        auto clip = makePtr<AnimationClip>();
        for (int idx : frameIndices) {
            int col = idx % cols;
            int row = idx / cols;

            // 翻转行顺序：精灵图第0行在顶部，但OpenGL纹理V坐标从底部开始
            int flippedRow = (rows - 1) - row;

            Rect rect(
                static_cast<float>(margin + col * (frameWidth + spacing)),
                static_cast<float>(margin + flippedRow * (frameHeight + spacing)),
                static_cast<float>(frameWidth),
                static_cast<float>(frameHeight)
            );

            auto sf = SpriteFrame::create(texture, rect);
            AnimationFrame frame;
            frame.spriteFrame = std::move(sf);
            frame.delay = frameDurationMs;
            clip->addFrame(std::move(frame));
        }
        return clip;
    }

private:
    std::string name_;
    std::string sourcePath_;
    std::vector<AnimationFrame> frames_;
    FramePropertySet globalProperties_;
};

} // namespace easy2d
