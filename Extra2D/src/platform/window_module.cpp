#include <extra2d/platform/window_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/platform/platform_module.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>
#include <SDL.h>

#ifdef __SWITCH__
#include <switch.h>
#endif

using json = nlohmann::json;

namespace extra2d {

static ModuleId s_windowModuleId = INVALID_MODULE_ID;

ModuleId get_window_module_id() {
    return s_windowModuleId;
}

void WindowModuleConfig::applyPlatformConstraints(PlatformType platform) {
#ifdef __SWITCH__
    (void)platform;
    windowConfig.mode = WindowMode::Fullscreen;
    windowConfig.resizable = false;
    windowConfig.highDPI = false;
    windowConfig.width = 1920;
    windowConfig.height = 1080;
#else
    (void)platform;
#endif
}

bool WindowModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("title")) {
            windowConfig.title = j["title"].get<std::string>();
        }
        if (j.contains("width")) {
            windowConfig.width = j["width"].get<int>();
        }
        if (j.contains("height")) {
            windowConfig.height = j["height"].get<int>();
        }
        if (j.contains("minWidth")) {
            windowConfig.minWidth = j["minWidth"].get<int>();
        }
        if (j.contains("minHeight")) {
            windowConfig.minHeight = j["minHeight"].get<int>();
        }
        if (j.contains("fullscreen")) {
            windowConfig.mode = j["fullscreen"].get<bool>() ? WindowMode::Fullscreen : WindowMode::Windowed;
        }
        if (j.contains("mode")) {
            std::string modeStr = j["mode"].get<std::string>();
            if (modeStr == "fullscreen") {
                windowConfig.mode = WindowMode::Fullscreen;
            } else if (modeStr == "borderless") {
                windowConfig.mode = WindowMode::Borderless;
            } else {
                windowConfig.mode = WindowMode::Windowed;
            }
        }
        if (j.contains("vsync")) {
            windowConfig.vsync = j["vsync"].get<bool>();
        }
        if (j.contains("resizable")) {
            windowConfig.resizable = j["resizable"].get<bool>();
        }
        if (j.contains("highDPI")) {
            windowConfig.highDPI = j["highDPI"].get<bool>();
        }
        if (j.contains("multisamples")) {
            windowConfig.multisamples = j["multisamples"].get<int>();
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool WindowModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) return false;
    
    try {
        json& j = *static_cast<json*>(jsonData);
        j["title"] = windowConfig.title;
        j["width"] = windowConfig.width;
        j["height"] = windowConfig.height;
        j["minWidth"] = windowConfig.minWidth;
        j["minHeight"] = windowConfig.minHeight;
        
        switch (windowConfig.mode) {
            case WindowMode::Fullscreen:
                j["mode"] = "fullscreen";
                break;
            case WindowMode::Borderless:
                j["mode"] = "borderless";
                break;
            default:
                j["mode"] = "windowed";
                break;
        }
        
        j["vsync"] = windowConfig.vsync;
        j["resizable"] = windowConfig.resizable;
        j["highDPI"] = windowConfig.highDPI;
        j["multisamples"] = windowConfig.multisamples;
        return true;
    } catch (...) {
        return false;
    }
}

WindowModuleInitializer::WindowModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , initialized_(false)
    , sdl2Initialized_(false) {
}

WindowModuleInitializer::~WindowModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

bool WindowModuleInitializer::initSDL2() {
    Uint32 initFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER;
    
#ifdef __SWITCH__
    initFlags |= SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;
#endif

    if (SDL_Init(initFlags) != 0) {
        E2D_LOG_ERROR("Failed to initialize SDL2: {}", SDL_GetError());
        return false;
    }
    
    sdl2Initialized_ = true;
    E2D_LOG_INFO("SDL2 initialized successfully");
    return true;
}

void WindowModuleInitializer::shutdownSDL2() {
    if (!sdl2Initialized_) return;
    
    SDL_Quit();
    sdl2Initialized_ = false;
    E2D_LOG_INFO("SDL2 shutdown");
}

bool WindowModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) return true;
    
    const WindowModuleConfig* windowConfig = dynamic_cast<const WindowModuleConfig*>(config);
    if (!windowConfig) {
        E2D_LOG_ERROR("Invalid window module config");
        return false;
    }

    windowConfig_ = windowConfig->windowConfig;

#ifdef __SWITCH__
    windowConfig_.mode = WindowMode::Fullscreen;
    windowConfig_.resizable = false;
    windowConfig_.highDPI = false;
    E2D_LOG_INFO("Switch platform: forcing fullscreen mode");
#endif

    if (!initSDL2()) {
        return false;
    }

    extern void initSDL2Backend();
    initSDL2Backend();

    if (!BackendFactory::has("sdl2")) {
        E2D_LOG_ERROR("SDL2 backend not registered!");
        shutdownSDL2();
        return false;
    }

    if (!createWindow(windowConfig_)) {
        E2D_LOG_ERROR("Failed to create window");
        shutdownSDL2();
        return false;
    }

    initialized_ = true;
    E2D_LOG_INFO("Window module initialized");
    E2D_LOG_INFO("  Window: {}x{}", window_->width(), window_->height());
    E2D_LOG_INFO("  Backend: SDL2");
    E2D_LOG_INFO("  VSync: {}", windowConfig_.vsync);
    E2D_LOG_INFO("  Fullscreen: {}", windowConfig_.isFullscreen());
    
    return true;
}

bool WindowModuleInitializer::createWindow(const WindowConfigData& config) {
    window_ = BackendFactory::createWindow("sdl2");
    if (!window_) {
        E2D_LOG_ERROR("Failed to create SDL2 window");
        return false;
    }

    if (!window_->create(config)) {
        E2D_LOG_ERROR("Failed to create window with specified config");
        return false;
    }

    return true;
}

void WindowModuleInitializer::shutdown() {
    if (!initialized_) return;
    
    E2D_LOG_INFO("Window module shutting down");
    
    if (window_) {
        window_->destroy();
        window_.reset();
    }
    
    shutdownSDL2();
    
    initialized_ = false;
}

void register_window_module() {
    if (s_windowModuleId != INVALID_MODULE_ID) return;
    
    s_windowModuleId = ModuleRegistry::instance().registerModule(
        makeUnique<WindowModuleConfig>(),
        []() -> UniquePtr<IModuleInitializer> {
            auto initializer = makeUnique<WindowModuleInitializer>();
            initializer->setModuleId(s_windowModuleId);
            return initializer;
        }
    );
}

namespace {
    struct WindowModuleAutoRegister {
        WindowModuleAutoRegister() {
            register_window_module();
        }
    };
    
    static WindowModuleAutoRegister s_autoRegister;
}

} 
