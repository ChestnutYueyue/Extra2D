#pragma once

#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/script/sq_binding.h>

namespace extra2d {
namespace sq {

// SqClassName specializations for value types
template <> struct SqClassName<Vec2> {
  static const char *name() { return "Vec2"; }
};
template <> struct SqClassName<Size> {
  static const char *name() { return "Size"; }
};
template <> struct SqClassName<Rect> {
  static const char *name() { return "Rect"; }
};
template <> struct SqClassName<Color> {
  static const char *name() { return "Color"; }
};

void registerVec2(HSQUIRRELVM vm);
void registerSize(HSQUIRRELVM vm);
void registerRect(HSQUIRRELVM vm);
void registerColor(HSQUIRRELVM vm);
void registerValueTypes(HSQUIRRELVM vm);

} // namespace sq
} // namespace extra2d
