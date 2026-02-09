#pragma once

#include <easy2d/script/sq_binding.h>

namespace easy2d {
namespace sq {

void registerInput(HSQUIRRELVM vm);
void registerKeyConstants(HSQUIRRELVM vm);
void registerInputBindings(HSQUIRRELVM vm);

} // namespace sq
} // namespace easy2d
