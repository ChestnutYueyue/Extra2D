#pragma once

#include <easy2d/core/types.h>
#include <easy2d/animation/ani_parser.h>
#include <easy2d/animation/animation_cache.h>
#include <string>
#include <functional>
#include <cstdint>

namespace easy2d {

// ============================================================================
// DNF ANI 二进制格式中的节点类型枚举
// ============================================================================
enum class AniNodeType : uint16_t {
    Loop            = 0,
    Shadow          = 1,
    Coord           = 3,
    ImageRate       = 7,
    ImageRotate     = 8,
    RGBA            = 9,
    Interpolation   = 10,
    GraphicEffect   = 11,
    Delay           = 12,
    DamageType      = 13,
    DamageBox       = 14,
    AttackBox       = 15,
    PlaySound       = 16,
    Preload         = 17,
    Spectrum        = 18,
    SetFlag         = 23,
    FlipType        = 24,
    LoopStart       = 25,
    LoopEnd         = 26,
    Clip            = 27,
    Operation       = 28,
};

// ============================================================================
// AniBinaryParser - ANI 二进制格式解析器
// 参考 DNF-Porting 的 PvfAnimation 实现
// ============================================================================
class AniBinaryParser {
public:
    AniBinaryParser() = default;

    /// 从二进制数据解析
    AniParseResult parse(const uint8_t* data, size_t length);

    /// 从文件解析
    AniParseResult parseFromFile(const std::string& filePath);

    /// 设置路径替换回调
    void setPathResolver(PathResolveCallback callback) {
        pathResolver_ = std::move(callback);
    }

    /// 设置基础路径
    void setBasePath(const std::string& basePath) {
        basePath_ = basePath;
    }

private:
    PathResolveCallback pathResolver_;
    std::string basePath_;

    std::string resolvePath(const std::string& relativePath) const;
};

} // namespace easy2d
