#pragma once

#include <easy2d/script/sq_binding.h>

namespace easy2d {
namespace sq {

void registerNode(HSQUIRRELVM vm);
void registerSprite(HSQUIRRELVM vm);
void registerScene(HSQUIRRELVM vm);
void registerSceneManager(HSQUIRRELVM vm);
void registerApplication(HSQUIRRELVM vm);
void registerNodeBindings(HSQUIRRELVM vm);

} // namespace sq
} // namespace easy2d
