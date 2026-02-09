#pragma once

#include <easy2d/animation/animation_clip.h>
#include <unordered_map>
#include <mutex>
#include <string>
#include <functional>

namespace easy2d {

// 路径替换回调（对应原始 AdditionalOptions）
using PathResolveCallback = std::function<std::string(const std::string&)>;

// ============================================================================
// AnimationCache - 动画片段全局缓存（借鉴 Cocos AnimationCache）
// 同一 ANI 文件只解析一次，后续直接复用数据
// ============================================================================
class AnimationCache {
public:
    static AnimationCache& getInstance() {
        static AnimationCache instance;
        return instance;
    }

    // ------ 加载与获取 ------

    /// 从文件加载（自动缓存），已缓存则直接返回
    /// 注意：实际的 ANI 解析逻辑在 AniParser 中实现
    /// 此方法在 animation_cache.cpp 中实现，依赖 AniParser
    Ptr<AnimationClip> loadClip(const std::string& aniFilePath);

    /// 从缓存获取（不触发加载）
    Ptr<AnimationClip> getClip(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = clips_.find(name);
        if (it != clips_.end()) return it->second;
        return nullptr;
    }

    /// 手动添加到缓存
    void addClip(Ptr<AnimationClip> clip, const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        clips_[name] = std::move(clip);
    }

    // ------ 缓存管理 ------

    bool has(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return clips_.find(name) != clips_.end();
    }

    void removeClip(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        clips_.erase(name);
    }

    /// 移除未被外部引用的动画片段
    void removeUnusedClips() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = clips_.begin(); it != clips_.end(); ) {
            if (it->second.use_count() == 1) {
                it = clips_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        clips_.clear();
    }

    size_t count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return clips_.size();
    }

    // ------ 路径配置 ------
    void setPathResolver(PathResolveCallback resolver) {
        pathResolver_ = std::move(resolver);
    }

    PathResolveCallback getPathResolver() const {
        return pathResolver_;
    }

private:
    AnimationCache() = default;
    ~AnimationCache() = default;
    AnimationCache(const AnimationCache&) = delete;
    AnimationCache& operator=(const AnimationCache&) = delete;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Ptr<AnimationClip>> clips_;
    PathResolveCallback pathResolver_;
};

// 便捷宏
#define E2D_ANIMATION_CACHE() ::easy2d::AnimationCache::getInstance()

} // namespace easy2d
