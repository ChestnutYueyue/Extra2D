#include <extra2d/animation/frame_property.h>

namespace extra2d {

// ============================================================================
// FramePropertySet 实现
// ============================================================================

void FramePropertySet::set(FramePropertyKey key, FramePropertyValue value) {
  properties_[key] = value;
}

void FramePropertySet::set(FramePropertyKey key, const std::string& value) {
  FramePropertyValue pv;
  pv.type = PropertyValueType::String;
  pv.data.stringIndex = allocateString(value);
  properties_[key] = pv;
}

void FramePropertySet::set(FramePropertyKey key, const std::vector<int>& value) {
  FramePropertyValue pv;
  pv.type = PropertyValueType::IntVector;
  pv.data.vectorIndex = allocateVector(value);
  properties_[key] = pv;
}

void FramePropertySet::setCustom(const std::string &key, std::any value) {
  customProperties_[key] = std::move(value);
}

bool FramePropertySet::has(FramePropertyKey key) const {
  return properties_.find(key) != properties_.end();
}

bool FramePropertySet::hasCustom(const std::string &key) const {
  return customProperties_.find(key) != customProperties_.end();
}

void FramePropertySet::remove(FramePropertyKey key) {
  properties_.erase(key);
}

void FramePropertySet::removeCustom(const std::string &key) {
  customProperties_.erase(key);
}

void FramePropertySet::clear() {
  properties_.clear();
  customProperties_.clear();
  stringPool_.clear();
  vectorPool_.clear();
  nextStringIndex_ = 0;
  nextVectorIndex_ = 0;
}

std::optional<std::any> FramePropertySet::getCustom(const std::string &key) const {
  auto it = customProperties_.find(key);
  if (it == customProperties_.end())
    return std::nullopt;
  return it->second;
}

// ============================================================================
// 字符串池和vector池管理
// ============================================================================

uint32_t FramePropertySet::allocateString(const std::string& str) {
  // 查找是否已存在相同字符串
  for (uint32_t i = 0; i < stringPool_.size(); ++i) {
    if (stringPool_[i] == str) {
      return i;
    }
  }
  // 分配新字符串
  uint32_t index = static_cast<uint32_t>(stringPool_.size());
  stringPool_.push_back(str);
  return index;
}

uint32_t FramePropertySet::allocateVector(const std::vector<int>& vec) {
  // 查找是否已存在相同vector
  for (uint32_t i = 0; i < vectorPool_.size(); ++i) {
    if (vectorPool_[i] == vec) {
      return i;
    }
  }
  // 分配新vector
  uint32_t index = static_cast<uint32_t>(vectorPool_.size());
  vectorPool_.push_back(vec);
  return index;
}

const std::string* FramePropertySet::getString(uint32_t index) const {
  if (index < stringPool_.size()) {
    return &stringPool_[index];
  }
  return nullptr;
}

const std::vector<int>* FramePropertySet::getVector(uint32_t index) const {
  if (index < vectorPool_.size()) {
    return &vectorPool_[index];
  }
  return nullptr;
}

// ============================================================================
// 模板特化实现
// ============================================================================

template <> std::optional<bool> FramePropertySet::get<bool>(FramePropertyKey key) const {
  auto it = properties_.find(key);
  if (it == properties_.end()) return std::nullopt;
  if (it->second.type == PropertyValueType::Bool) {
    return it->second.data.boolValue;
  }
  return std::nullopt;
}

template <> std::optional<int> FramePropertySet::get<int>(FramePropertyKey key) const {
  auto it = properties_.find(key);
  if (it == properties_.end()) return std::nullopt;
  if (it->second.type == PropertyValueType::Int) {
    return it->second.data.intValue;
  }
  return std::nullopt;
}

template <> std::optional<float> FramePropertySet::get<float>(FramePropertyKey key) const {
  auto it = properties_.find(key);
  if (it == properties_.end()) return std::nullopt;
  if (it->second.type == PropertyValueType::Float) {
    return it->second.data.floatValue;
  }
  return std::nullopt;
}

template <> std::optional<Vec2> FramePropertySet::get<Vec2>(FramePropertyKey key) const {
  auto it = properties_.find(key);
  if (it == properties_.end()) return std::nullopt;
  if (it->second.type == PropertyValueType::Vec2) {
    return it->second.data.vec2Value;
  }
  return std::nullopt;
}

template <> std::optional<Color> FramePropertySet::get<Color>(FramePropertyKey key) const {
  auto it = properties_.find(key);
  if (it == properties_.end()) return std::nullopt;
  if (it->second.type == PropertyValueType::Color) {
    return it->second.data.colorValue;
  }
  return std::nullopt;
}

template <> std::optional<std::string> FramePropertySet::get<std::string>(FramePropertyKey key) const {
  auto it = properties_.find(key);
  if (it == properties_.end()) return std::nullopt;
  if (it->second.type == PropertyValueType::String) {
    const std::string* str = getString(it->second.data.stringIndex);
    if (str) return *str;
  }
  return std::nullopt;
}

template <> std::optional<std::vector<int>> FramePropertySet::get<std::vector<int>>(FramePropertyKey key) const {
  auto it = properties_.find(key);
  if (it == properties_.end()) return std::nullopt;
  if (it->second.type == PropertyValueType::IntVector) {
    const std::vector<int>* vec = getVector(it->second.data.vectorIndex);
    if (vec) return *vec;
  }
  return std::nullopt;
}

// ============================================================================
// 链式 API 实现
// ============================================================================

FramePropertySet &FramePropertySet::withSetFlag(int index) {
  set(FramePropertyKey::SetFlag, FramePropertyValue(index));
  return *this;
}

FramePropertySet &FramePropertySet::withPlaySound(const std::string &path) {
  set(FramePropertyKey::PlaySound, path);
  return *this;
}

FramePropertySet &FramePropertySet::withImageRate(const Vec2 &scale) {
  set(FramePropertyKey::ImageRate, FramePropertyValue(scale));
  return *this;
}

FramePropertySet &FramePropertySet::withImageRotate(float degrees) {
  set(FramePropertyKey::ImageRotate, FramePropertyValue(degrees));
  return *this;
}

FramePropertySet &FramePropertySet::withColorTint(const Color &color) {
  set(FramePropertyKey::ColorTint, FramePropertyValue(color));
  return *this;
}

FramePropertySet &FramePropertySet::withInterpolation(bool enabled) {
  set(FramePropertyKey::Interpolation, FramePropertyValue(enabled));
  return *this;
}

FramePropertySet &FramePropertySet::withBlendLinearDodge(bool enabled) {
  set(FramePropertyKey::BlendLinearDodge, FramePropertyValue(enabled));
  return *this;
}

FramePropertySet &FramePropertySet::withLoop(bool enabled) {
  set(FramePropertyKey::Loop, FramePropertyValue(enabled));
  return *this;
}

} // namespace extra2d
