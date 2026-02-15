#include <extra2d/platform/window_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/config/config_manager.h>
#include <extra2d/platform/platform_module.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>

#ifdef E2D_BACKEND_SDL2
#include <SDL.h>
#endif

#ifdef E2D_BACKEND_GLFW
#include <GLFW/glfw3.h>
#endif

#ifdef __SWITCH__
#include <switch.h>
#endif

using json = nlohmann::json;

namespace extra2d {

static ModuleId s_windowModuleId = INVALID_MODULE_ID;

ModuleId get_window_module_id() {
    return s_windowModuleId;
}

bool WindowModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("backend")) {
            backend = j["backend"].get<std::string>();
        }
        
        if (j.contains("title")) {
            windowConfig.title = j["title"].get<std::string>();
        }
        if (j.contains("width")) {
            windowConfig.width = j["width"].get<int>();
        }
        if (j.contains("height")) {
            windowConfig.height = j["height"].get<int>();
        }
        if (j.contains("fullscreen")) {
            windowConfig.mode = j["fullscreen"].get<bool>() ? WindowMode::Fullscreen : WindowMode::Windowed;
        }
        if (j.contains("vsync")) {
            windowConfig.vsync = j["vsync"].get<bool>();
        }
        if (j.contains("resizable")) {
            windowConfig.resizable = j["resizable"].get<bool>();
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
        j["backend"] = backend;
        j["title"] = windowConfig.title;
        j["width"] = windowConfig.width;
        j["height"] = windowConfig.height;
        j["fullscreen"] = (windowConfig.mode == WindowMode::Fullscreen);
        j["vsync"] = windowConfig.vsync;
        j["resizable"] = windowConfig.resizable;
        return true;
    } catch (...) {
        return false;
    }
}

WindowModuleInitializer::WindowModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , initialized_(false)
    , backendInitialized_(false) {
}

WindowModuleInitializer::~WindowModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

bool WindowModuleInitializer::initBackend() {
#ifdef E2D_BACKEND_SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        E2D_LOG_ERROR("Failed to initialize SDL2: {}", SDL_GetError());
        return false;
    }
    E2D_LOG_INFO("SDL2 backend initialized");
    backendInitialized_ = true;
    return true;
#endif

#ifdef E2D_BACKEND_GLFW
    if (!glfwInit()) {
        E2D_LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    E2D_LOG_INFO("GLFW backend initialized");
    backendInitialized_ = true;
    return true;
#endif

#ifdef E2D_BACKEND_SWITCH
    E2D_LOG_INFO("Switch backend (no init required)");
    backendInitialized_ = true;
    return true;
#endif

    E2D_LOG_ERROR("No backend available");
    return false;
}

void WindowModuleInitializer::shutdownBackend() {
    if (!backendInitialized_) return;
    
#ifdef E2D_BACKEND_SDL2
    SDL_Quit();
    E2D_LOG_INFO("SDL2 backend shutdown");
#endif

#ifdef E2D_BACKEND_GLFW
    glfwTerminate();
    E2D_LOG_INFO("GLFW backend shutdown");
#endif

    backendInitialized_ = false;
}

bool WindowModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) return true;
    
    const WindowModuleConfig* windowConfig = dynamic_cast<const WindowModuleConfig*>(config);
    if (!windowConfig) {
        E2D_LOG_ERROR("Invalid window module config");
        return false;
    }

    backend_ = windowConfig->backend;
    windowConfig_ = windowConfig->windowConfig;
    
#ifdef __SWITCH__
    backend_ = "switch";
    windowConfig_.mode = WindowMode::Fullscreen;
    windowConfig_.resizable = false;
#endif

    if (!initBackend()) {
        return false;
    }

#ifdef E2D_BACKEND_SDL2
    extern void initSDL2Backend();
    initSDL2Backend();
#endif

    if (!BackendFactory::has(backend_)) {
        E2D_LOG_ERROR("Backend '{}' not available", backend_);
        auto backends = BackendFactory::backends();
        if (backends.empty()) {
            E2D_LOG_ERROR("No backends registered!");
            shutdownBackend();
            return false;
        }
        backend_ = backends[0];
        E2D_LOG_WARN("Using fallback backend: {}", backend_);
    }

    if (!createWindow(backend_, windowConfig_)) {
        E2D_LOG_ERROR("Failed to create window");
        shutdownBackend();
        return false;
    }

    initialized_ = true;
    E2D_LOG_INFO("Window module initialized");
    E2D_LOG_INFO("  Window: {}x{}", window_->width(), window_->height());
    E2D_LOG_INFO("  Backend: {}", backend_);
    E2D_LOG_INFO("  VSync: {}", windowConfig_.vsync);
    
    return true;
}

bool WindowModuleInitializer::createWindow(const std::string& backend, const WindowConfigData& config) {
    window_ = BackendFactory::createWindow(backend);
    if (!window_) {
        E2D_LOG_ERROR("Failed to create window for backend: {}", backend);
        return false;
    }

    if (!window_->create(config)) {
        E2D_LOG_ERROR("Failed to create window");
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
    
    shutdownBackend();
    
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

} // namespace extra2d
