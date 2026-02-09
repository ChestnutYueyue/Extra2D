#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/math_types.h>
#include <easy2d/core/color.h>
#include <variant>
#include <unordered_map>
#include <optional>
#include <any>
#include <string>
#include <vector>

namespace easy2d {

// ============================================================================
// 帧属性键 - 强类型枚举替代原始 ANI 的字符串键
// ============================================================================
enum class FramePropertyKey : uint32 {
    // 事件触发
    SetFlag             = 0x0001,   // int: 关键帧回调索引
    PlaySound           = 0x0002,   // string: 音效路径

    // 变换属性
    ImageRate           = 0x0010,   // Vec2: 缩放比例
    ImageRotate         = 0x0011,   // float: 旋转角度（度）
    ImageOffset         = 0x0012,   // Vec2: 额外位置偏移

    // 视觉效果
    BlendLinearDodge    = 0x0020,   // bool: 线性减淡
    BlendAdditive       = 0x0021,   // bool: 加法混合
    ColorTint           = 0x0022,   // Color: RGBA 颜色

    // 控制标记
    Interpolation       = 0x0030,   // bool: 启用到下一帧的插值
    Loop                = 0x0031,   // bool: 全局循环标记

    // DNF ANI 扩展属性
    DamageType          = 0x0040,   // int: 伤害类型 (0=Normal, 1=SuperArmor, 2=Unbreakable)
    Shadow              = 0x0041,   // bool: 阴影
    FlipType            = 0x0042,   // int: 翻转类型 (1=Horizon, 2=Vertical, 3=All)
    Coord               = 0x0043,   // int: 坐标系
    LoopStart           = 0x0044,   // bool: 循环起始标记
    LoopEnd             = 0x0045,   // int: 循环结束帧数
    GraphicEffect       = 0x0046,   // int: 图形特效类型
    ClipRegion          = 0x0047,   // vector<int>: 裁剪区域 [4个int16]

    // 用户自定义扩展区间 (0x1000+)
    UserDefined         = 0x1000,
};

// ============================================================================
// 帧属性值 - variant 多态值
// ============================================================================
using FramePropertyValue = std::variant<
    bool,
    int,
    float,
    std::string,
    Vec2,
    Color,
    std::vector<int>
>;

// ============================================================================
// FramePropertyKey 的 hash 支持
// ============================================================================
struct FramePropertyKeyHash {
    size_t operator()(FramePropertyKey key) const noexcept {
        return std::hash<uint32>{}(static_cast<uint32>(key));
    }
};

// ============================================================================
// FramePropertySet - 单帧属性集合
// 同时支持强类型属性和自定义扩展（不固定数据）
// ============================================================================
class FramePropertySet {
public:
    FramePropertySet() = default;

    // ------ 设置属性 ------
    void set(FramePropertyKey key, FramePropertyValue value) {
        properties_[key] = std::move(value);
    }

    void setCustom(const std::string& key, std::any value) {
        customProperties_[key] = std::move(value);
    }

    // ------ 类型安全获取 ------
    template<typename T>
    std::optional<T> get(FramePropertyKey key) const {
        auto it = properties_.find(key);
        if (it == properties_.end()) return std::nullopt;
        if (auto* val = std::get_if<T>(&it->second)) {
            return *val;
        }
        return std::nullopt;
    }

    template<typename T>
    T getOr(FramePropertyKey key, const T& defaultValue) const {
        auto result = get<T>(key);
        return result.value_or(defaultValue);
    }

    std::optional<std::any> getCustom(const std::string& key) const {
        auto it = customProperties_.find(key);
        if (it == customProperties_.end()) return std::nullopt;
        return it->second;
    }

    // ------ 查询 ------
    bool has(FramePropertyKey key) const {
        return properties_.find(key) != properties_.end();
    }

    bool hasCustom(const std::string& key) const {
        return customProperties_.find(key) != customProperties_.end();
    }

    bool empty() const {
        return properties_.empty() && customProperties_.empty();
    }

    size_t count() const {
        return properties_.size() + customProperties_.size();
    }

    // ------ 移除 ------
    void remove(FramePropertyKey key) {
        properties_.erase(key);
    }

    void removeCustom(const std::string& key) {
        customProperties_.erase(key);
    }

    void clear() {
        properties_.clear();
        customProperties_.clear();
    }

    // ------ 迭代 ------
    using PropertyMap = std::unordered_map<FramePropertyKey, FramePropertyValue, FramePropertyKeyHash>;
    const PropertyMap& properties() const { return properties_; }

    // ------ 链式 API ------
    FramePropertySet& withSetFlag(int index) {
        set(FramePropertyKey::SetFlag, index);
        return *this;
    }

    FramePropertySet& withPlaySound(const std::string& path) {
        set(FramePropertyKey::PlaySound, path);
        return *this;
    }

    FramePropertySet& withImageRate(const Vec2& scale) {
        set(FramePropertyKey::ImageRate, scale);
        return *this;
    }

    FramePropertySet& withImageRotate(float degrees) {
        set(FramePropertyKey::ImageRotate, degrees);
        return *this;
    }

    FramePropertySet& withColorTint(const Color& color) {
        set(FramePropertyKey::ColorTint, color);
        return *this;
    }

    FramePropertySet& withInterpolation(bool enabled = true) {
        set(FramePropertyKey::Interpolation, enabled);
        return *this;
    }

    FramePropertySet& withBlendLinearDodge(bool enabled = true) {
        set(FramePropertyKey::BlendLinearDodge, enabled);
        return *this;
    }

    FramePropertySet& withLoop(bool enabled = true) {
        set(FramePropertyKey::Loop, enabled);
        return *this;
    }

private:
    PropertyMap properties_;
    std::unordered_map<std::string, std::any> customProperties_;
};

} // namespace easy2d
