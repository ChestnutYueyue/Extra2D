#pragma once

#include <easy2d/core/types.h>
#include <easy2d/animation/animation_clip.h>
#include <easy2d/animation/animation_cache.h>
#include <string>
#include <functional>

namespace easy2d {

// ============================================================================
// ANI 文件解析结果
// ============================================================================
struct AniParseResult {
    bool success = false;
    std::string errorMessage;
    Ptr<AnimationClip> clip;
};

// ============================================================================
// AniParser - ANI 脚本文件解析器
// 将原始 ANI 文件格式解析为 AnimationClip 数据
// ============================================================================
class AniParser {
public:
    AniParser() = default;

    /// 从文件解析
    AniParseResult parse(const std::string& filePath);

    /// 从内存内容解析
    AniParseResult parseFromMemory(const std::string& content,
                                    const std::string& basePath = "");

    /// 设置路径替换回调（对应原始 AdditionalOptions）
    void setPathResolver(PathResolveCallback callback) {
        pathResolver_ = std::move(callback);
    }

    /// 设置基础路径（用于解析相对路径）
    void setBasePath(const std::string& basePath) {
        basePath_ = basePath;
    }

private:
    PathResolveCallback pathResolver_;
    std::string basePath_;

    std::string resolvePath(const std::string& relativePath) const;
};

} // namespace easy2d
