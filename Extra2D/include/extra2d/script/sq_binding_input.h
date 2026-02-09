#pragma once

#include <extra2d/script/sq_binding.h>

namespace extra2d {
namespace sq {

void registerInput(HSQUIRRELVM vm);
void registerKeyConstants(HSQUIRRELVM vm);
void registerInputBindings(HSQUIRRELVM vm);

} // namespace sq
} // namespace extra2d
