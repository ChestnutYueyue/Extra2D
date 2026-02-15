#include <extra2d/platform/input_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/core/service_locator.h>
#include <extra2d/platform/window_module.h>
#include <extra2d/services/event_service.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>

#include "backends/sdl2/sdl2_input.h"

using json = nlohmann::json;

namespace extra2d {

static ModuleId s_inputModuleId = INVALID_MODULE_ID;

ModuleId get_input_module_id() {
    return s_inputModuleId;
}

bool InputModuleConfig::validate() const {
    return inputConfig.isDeadzoneValid();
}

void InputModuleConfig::applyPlatformConstraints(PlatformType platform) {
#ifdef __SWITCH__
    (void)platform;
    inputConfig.enableVibration = true;
    inputConfig.maxGamepads = 2;
#else
    (void)platform;
#endif
}

bool InputModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("enabled")) {
            inputConfig.enabled = j["enabled"].get<bool>();
        }
        if (j.contains("rawMouseInput")) {
            inputConfig.rawMouseInput = j["rawMouseInput"].get<bool>();
        }
        if (j.contains("mouseSensitivity")) {
            inputConfig.mouseSensitivity = j["mouseSensitivity"].get<float>();
        }
        if (j.contains("invertMouseY")) {
            inputConfig.invertMouseY = j["invertMouseY"].get<bool>();
        }
        if (j.contains("invertMouseX")) {
            inputConfig.invertMouseX = j["invertMouseX"].get<bool>();
        }
        if (j.contains("deadzone")) {
            inputConfig.deadzone = j["deadzone"].get<float>();
        }
        if (j.contains("triggerThreshold")) {
            inputConfig.triggerThreshold = j["triggerThreshold"].get<float>();
        }
        if (j.contains("enableVibration")) {
            inputConfig.enableVibration = j["enableVibration"].get<bool>();
        }
        if (j.contains("maxGamepads")) {
            inputConfig.maxGamepads = j["maxGamepads"].get<int>();
        }
        if (j.contains("autoConnectGamepads")) {
            inputConfig.autoConnectGamepads = j["autoConnectGamepads"].get<bool>();
        }
        if (j.contains("gamepadMappingFile")) {
            inputConfig.gamepadMappingFile = j["gamepadMappingFile"].get<std::string>();
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool InputModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) return false;
    
    try {
        json& j = *static_cast<json*>(jsonData);
        
        j["enabled"] = inputConfig.enabled;
        j["rawMouseInput"] = inputConfig.rawMouseInput;
        j["mouseSensitivity"] = inputConfig.mouseSensitivity;
        j["invertMouseY"] = inputConfig.invertMouseY;
        j["invertMouseX"] = inputConfig.invertMouseX;
        j["deadzone"] = inputConfig.deadzone;
        j["triggerThreshold"] = inputConfig.triggerThreshold;
        j["enableVibration"] = inputConfig.enableVibration;
        j["maxGamepads"] = inputConfig.maxGamepads;
        j["autoConnectGamepads"] = inputConfig.autoConnectGamepads;
        j["gamepadMappingFile"] = inputConfig.gamepadMappingFile;
        
        return true;
    } catch (...) {
        return false;
    }
}

InputModuleInitializer::InputModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , input_(nullptr)
    , initialized_(false) {
}

InputModuleInitializer::~InputModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

std::vector<ModuleId> InputModuleInitializer::getDependencies() const {
    return { get_window_module_id() };
}

bool InputModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) return true;
    
    const InputModuleConfig* inputConfig = dynamic_cast<const InputModuleConfig*>(config);
    if (!inputConfig) {
        E2D_LOG_ERROR("Invalid input module config");
        return false;
    }
    
    config_ = inputConfig->inputConfig;
    
    auto& registry = ModuleRegistry::instance();
    auto* windowInitializer = registry.getInitializer(get_window_module_id());
    if (!windowInitializer) {
        E2D_LOG_ERROR("Window module not found - Input module depends on it");
        return false;
    }
    
    auto* windowModule = static_cast<WindowModuleInitializer*>(windowInitializer);
    IWindow* window = windowModule->getWindow();
    if (!window) {
        E2D_LOG_ERROR("Window not created - cannot get input interface");
        return false;
    }
    
    input_ = window->input();
    if (!input_) {
        E2D_LOG_ERROR("Input interface not available from window");
        return false;
    }
    
    SDL2Input* sdl2Input = dynamic_cast<SDL2Input*>(input_);
    if (sdl2Input) {
        auto eventService = ServiceLocator::instance().getService<IEventService>();
        if (eventService) {
            sdl2Input->setEventCallback([eventService](const Event& event) {
                Event mutableEvent = event;
                eventService->dispatch(mutableEvent);
            });
            E2D_LOG_INFO("Input events connected to EventService");
        } else {
            E2D_LOG_WARN("EventService not available - input events will not be dispatched");
        }
    }
    
    initialized_ = true;
    E2D_LOG_INFO("Input module initialized");
    E2D_LOG_INFO("  Deadzone: {}", config_.deadzone);
    E2D_LOG_INFO("  Mouse sensitivity: {}", config_.mouseSensitivity);
    E2D_LOG_INFO("  Vibration: {}", config_.enableVibration ? "enabled" : "disabled");
    
    return true;
}

void InputModuleInitializer::shutdown() {
    if (!initialized_) return;
    
    E2D_LOG_INFO("Input module shutting down");
    
    input_ = nullptr;
    initialized_ = false;
}

void InputModuleInitializer::update() {
    if (!initialized_ || !input_) return;
    
    input_->update();
}

void register_input_module() {
    if (s_inputModuleId != INVALID_MODULE_ID) return;
    
    s_inputModuleId = ModuleRegistry::instance().registerModule(
        makeUnique<InputModuleConfig>(),
        []() -> UniquePtr<IModuleInitializer> {
            auto initializer = makeUnique<InputModuleInitializer>();
            initializer->setModuleId(s_inputModuleId);
            return initializer;
        }
    );
}

namespace {
    struct InputModuleAutoRegister {
        InputModuleAutoRegister() {
            register_input_module();
        }
    };
    
    static InputModuleAutoRegister s_autoRegister;
}

} 
