#pragma once

#include <any>
#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace extra2d {

// ============================================================================
// 帧属性键 - 强类型枚举替代原始 ANI 的字符串键
// ============================================================================
enum class FramePropertyKey : uint32 {
  // 事件触发
  SetFlag = 0x0001,   // int: 关键帧回调索引
  PlaySound = 0x0002, // string: 音效路径

  // 变换属性
  ImageRate = 0x0010,   // Vec2: 缩放比例
  ImageRotate = 0x0011, // float: 旋转角度（度）
  ImageOffset = 0x0012, // Vec2: 额外位置偏移

  // 视觉效果
  BlendLinearDodge = 0x0020, // bool: 线性减淡
  BlendAdditive = 0x0021,    // bool: 加法混合
  ColorTint = 0x0022,        // Color: RGBA 颜色

  // 控制标记
  Interpolation = 0x0030, // bool: 启用到下一帧的插值
  Loop = 0x0031,          // bool: 全局循环标记

  // DNF ANI 扩展属性
  DamageType = 0x0040, // int: 伤害类型 (0=Normal, 1=SuperArmor, 2=Unbreakable)
  Shadow = 0x0041,     // bool: 阴影
  FlipType = 0x0042,   // int: 翻转类型 (1=Horizon, 2=Vertical, 3=All)
  Coord = 0x0043,      // int: 坐标系
  LoopStart = 0x0044,  // bool: 循环起始标记
  LoopEnd = 0x0045,    // int: 循环结束帧数
  GraphicEffect = 0x0046, // int: 图形特效类型
  ClipRegion = 0x0047,    // vector<int>: 裁剪区域 [4个int16]

  // 用户自定义扩展区间 (0x1000+)
  UserDefined = 0x1000,
};

// ============================================================================
// 帧属性值 - variant 多态值（优化版本）
// 使用紧凑存储，常用小类型直接内联，大类型使用索引引用
// ============================================================================

// 前向声明
struct FramePropertyValue;

// 属性存储类型枚举
enum class PropertyValueType : uint8_t {
  Empty = 0,
  Bool = 1,
  Int = 2,
  Float = 3,
  Vec2 = 4,
  Color = 5,
  String = 6,        // 字符串使用索引引用
  IntVector = 7,     // vector<int> 使用索引引用
};

// 紧凑的属性值结构（16字节）
struct FramePropertyValue {
  PropertyValueType type = PropertyValueType::Empty;
  uint8_t padding[3] = {0};

  // 使用结构体包装非平凡类型，使其可以在union中使用
  struct Vec2Storage {
    float x, y;
    Vec2Storage() = default;
    Vec2Storage(const Vec2& v) : x(v.x), y(v.y) {}
    operator Vec2() const { return Vec2(x, y); }
  };

  struct ColorStorage {
    float r, g, b, a;
    ColorStorage() = default;
    ColorStorage(const Color& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
    operator Color() const { return Color(r, g, b, a); }
  };

  union Data {
    bool boolValue;
    int intValue;
    float floatValue;
    Vec2Storage vec2Value;
    ColorStorage colorValue;
    uint32_t stringIndex;     // 字符串池索引
    uint32_t vectorIndex;     // vector池索引

    Data() : intValue(0) {}  // 默认构造函数
    ~Data() {}  // 析构函数
  } data;

  FramePropertyValue() : type(PropertyValueType::Empty) {}
  explicit FramePropertyValue(bool v) : type(PropertyValueType::Bool) { data.boolValue = v; }
  explicit FramePropertyValue(int v) : type(PropertyValueType::Int) { data.intValue = v; }
  explicit FramePropertyValue(float v) : type(PropertyValueType::Float) { data.floatValue = v; }
  explicit FramePropertyValue(const Vec2& v) : type(PropertyValueType::Vec2) { data.vec2Value = v; }
  explicit FramePropertyValue(const Color& v) : type(PropertyValueType::Color) { data.colorValue = v; }

  bool isInline() const {
    return type <= PropertyValueType::Color;
  }

  bool isString() const { return type == PropertyValueType::String; }
  bool isIntVector() const { return type == PropertyValueType::IntVector; }
};

// ============================================================================
// FramePropertyKey 的 hash 支持
// ============================================================================
struct FramePropertyKeyHash {
  size_t operator()(FramePropertyKey key) const noexcept {
    return std::hash<uint32>{}(static_cast<uint32>(key));
  }
};

// ============================================================================
// FramePropertySet - 单帧属性集合（优化版本）
// 使用紧凑存储和线性探测哈希表，提高缓存命中率
// ============================================================================
class FramePropertySet {
public:
  FramePropertySet() = default;

  // ------ 设置属性 ------
  void set(FramePropertyKey key, FramePropertyValue value);
  void set(FramePropertyKey key, bool value) { set(key, FramePropertyValue(value)); }
  void set(FramePropertyKey key, int value) { set(key, FramePropertyValue(value)); }
  void set(FramePropertyKey key, float value) { set(key, FramePropertyValue(value)); }
  void set(FramePropertyKey key, const Vec2& value) { set(key, FramePropertyValue(value)); }
  void set(FramePropertyKey key, const Color& value) { set(key, FramePropertyValue(value)); }
  void set(FramePropertyKey key, const std::string& value);
  void set(FramePropertyKey key, const std::vector<int>& value);

  void setCustom(const std::string &key, std::any value);

  // ------ 类型安全获取 ------
  template <typename T> std::optional<T> get(FramePropertyKey key) const;

  template <typename T>
  T getOr(FramePropertyKey key, const T &defaultValue) const {
    auto result = get<T>(key);
    return result.value_or(defaultValue);
  }

  std::optional<std::any> getCustom(const std::string &key) const;

  // ------ 查询 ------
  bool has(FramePropertyKey key) const;
  bool hasCustom(const std::string &key) const;
  bool empty() const { return properties_.empty() && customProperties_.empty(); }
  size_t count() const { return properties_.size() + customProperties_.size(); }

  // ------ 移除 ------
  void remove(FramePropertyKey key);
  void removeCustom(const std::string &key);
  void clear();

  // ------ 迭代 ------
  using PropertyMap = std::unordered_map<FramePropertyKey, FramePropertyValue,
                                         FramePropertyKeyHash>;
  const PropertyMap &properties() const { return properties_; }

  // ------ 链式 API ------
  FramePropertySet &withSetFlag(int index);
  FramePropertySet &withPlaySound(const std::string &path);
  FramePropertySet &withImageRate(const Vec2 &scale);
  FramePropertySet &withImageRotate(float degrees);
  FramePropertySet &withColorTint(const Color &color);
  FramePropertySet &withInterpolation(bool enabled = true);
  FramePropertySet &withBlendLinearDodge(bool enabled = true);
  FramePropertySet &withLoop(bool enabled = true);

private:
  PropertyMap properties_;
  std::unordered_map<std::string, std::any> customProperties_;

  // 字符串池和vector池，用于存储大对象
  mutable std::vector<std::string> stringPool_;
  mutable std::vector<std::vector<int>> vectorPool_;
  mutable uint32_t nextStringIndex_ = 0;
  mutable uint32_t nextVectorIndex_ = 0;

  uint32_t allocateString(const std::string& str);
  uint32_t allocateVector(const std::vector<int>& vec);
  const std::string* getString(uint32_t index) const;
  const std::vector<int>* getVector(uint32_t index) const;
};

// 模板特化声明
template <> std::optional<bool> FramePropertySet::get<bool>(FramePropertyKey key) const;
template <> std::optional<int> FramePropertySet::get<int>(FramePropertyKey key) const;
template <> std::optional<float> FramePropertySet::get<float>(FramePropertyKey key) const;
template <> std::optional<Vec2> FramePropertySet::get<Vec2>(FramePropertyKey key) const;
template <> std::optional<Color> FramePropertySet::get<Color>(FramePropertyKey key) const;
template <> std::optional<std::string> FramePropertySet::get<std::string>(FramePropertyKey key) const;
template <> std::optional<std::vector<int>> FramePropertySet::get<std::vector<int>>(FramePropertyKey key) const;

} // namespace extra2d
