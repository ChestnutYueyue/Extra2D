#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/math_types.h>
#include <string>
#include <vector>

namespace easy2d {

// ============================================================================
// ALS 图层信息
// ============================================================================
struct AlsLayerInfo {
    std::string aniPath;    // 子动画的 ANI 文件路径
    int         zOrder = 0; // 层级顺序
    Vec2        offset;     // 层偏移
};

// ============================================================================
// ALS 解析结果
// ============================================================================
struct AlsParseResult {
    bool success = false;
    std::string errorMessage;
    std::vector<AlsLayerInfo> layers;
};

// ============================================================================
// AlsParser - ALS 复合动画文件解析器
// 解析 .als 文件获取多层动画的图层信息
// ============================================================================
class AlsParser {
public:
    AlsParser() = default;

    /// 从文件解析
    AlsParseResult parse(const std::string& filePath);

    /// 从内存内容解析
    AlsParseResult parseFromMemory(const std::string& content,
                                    const std::string& basePath = "");

    /// 设置基础路径
    void setBasePath(const std::string& basePath) {
        basePath_ = basePath;
    }

private:
    std::string basePath_;

    std::string resolvePath(const std::string& relativePath) const;
};

} // namespace easy2d
