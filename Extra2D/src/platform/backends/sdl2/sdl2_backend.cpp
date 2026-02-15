#include "sdl2_window.h"
#include "sdl2_input.h"
#include <extra2d/platform/platform_module.h>

namespace extra2d {

namespace {
    static bool s_sdl2BackendRegistered = false;
}

void initSDL2Backend() {
    if (s_sdl2BackendRegistered) {
        return;
    }
    s_sdl2BackendRegistered = true;
    
    BackendFactory::reg(
        "sdl2",
        []() -> UniquePtr<IWindow> { 
            return makeUnique<SDL2Window>(); 
        },
        []() -> UniquePtr<IInput> { 
            return makeUnique<SDL2Input>(); 
        }
    );
}

} // namespace extra2d
