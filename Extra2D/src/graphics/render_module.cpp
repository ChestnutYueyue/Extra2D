#include <extra2d/graphics/render_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/graphics/opengl/gl_shader.h>
#include <extra2d/graphics/shader_manager.h>
#include <extra2d/platform/iwindow.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

namespace extra2d {

static ModuleId s_renderModuleId = INVALID_MODULE_ID;

ModuleId get_render_module_id() {
    return s_renderModuleId;
}

bool RenderModuleConfig::validate() const {
    if (targetFPS < 1 || targetFPS > 240) {
        return false;
    }
    
    if (multisamples != 0 && multisamples != 2 && multisamples != 4 && 
        multisamples != 8 && multisamples != 16) {
        return false;
    }
    
    if (spriteBatchSize <= 0) {
        return false;
    }
    
    return true;
}

void RenderModuleConfig::applyPlatformConstraints(PlatformType platform) {
    switch (platform) {
        case PlatformType::Switch:
            if (multisamples > 4) {
                multisamples = 4;
            }
            if (sRGBFramebuffer) {
                sRGBFramebuffer = false;
            }
            if (targetFPS > 60) {
                targetFPS = 60;
            }
            break;
        default:
            break;
    }
}

void RenderModuleConfig::resetToDefaults() {
    backend = BackendType::OpenGL;
    vsync = true;
    targetFPS = 60;
    multisamples = 0;
    sRGBFramebuffer = false;
    spriteBatchSize = 1000;
}

bool RenderModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("backend")) {
            std::string backendStr = j["backend"].get<std::string>();
            if (backendStr == "opengl") {
                backend = BackendType::OpenGL;
            }
        }
        
        if (j.contains("vsync")) {
            vsync = j["vsync"].get<bool>();
        }
        
        if (j.contains("targetFPS")) {
            targetFPS = j["targetFPS"].get<int>();
        }
        
        if (j.contains("multisamples")) {
            multisamples = j["multisamples"].get<int>();
        }
        
        if (j.contains("sRGBFramebuffer")) {
            sRGBFramebuffer = j["sRGBFramebuffer"].get<bool>();
        }
        
        if (j.contains("spriteBatchSize")) {
            spriteBatchSize = j["spriteBatchSize"].get<int>();
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool RenderModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) return false;
    
    try {
        json& j = *static_cast<json*>(jsonData);
        j["backend"] = "opengl";
        j["vsync"] = vsync;
        j["targetFPS"] = targetFPS;
        j["multisamples"] = multisamples;
        j["sRGBFramebuffer"] = sRGBFramebuffer;
        j["spriteBatchSize"] = spriteBatchSize;
        return true;
    } catch (...) {
        return false;
    }
}

RenderModuleInitializer::RenderModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , window_(nullptr)
    , initialized_(false) {
}

RenderModuleInitializer::~RenderModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

std::vector<ModuleId> RenderModuleInitializer::getDependencies() const {
    return {};
}

bool RenderModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) return true;
    
    if (!config) return false;
    
    const RenderModuleConfig* renderConfig = dynamic_cast<const RenderModuleConfig*>(config);
    if (!renderConfig) return false;
    
    if (!renderConfig->validate()) return false;
    
    if (!window_) {
        E2D_LOG_ERROR("Render module requires window to be set");
        return false;
    }

    auto shaderFactory = std::make_shared<GLShaderFactory>();
    if (!ShaderManager::getInstance().init(shaderFactory, "extra2d")) {
        E2D_LOG_WARN("Failed to initialize ShaderManager with default paths");
    }

    if (!ShaderManager::getInstance().loadBuiltinShaders()) {
        E2D_LOG_WARN("Failed to load some builtin shaders");
    }
    
    renderer_ = RenderBackend::create(renderConfig->backend);
    if (!renderer_) {
        E2D_LOG_ERROR("Failed to create render backend");
        return false;
    }
    
    if (!renderer_->init(window_)) {
        E2D_LOG_ERROR("Failed to initialize renderer");
        renderer_.reset();
        return false;
    }
    
    initialized_ = true;
    E2D_LOG_INFO("Render module initialized");
    return true;
}

void RenderModuleInitializer::shutdown() {
    if (!initialized_) return;
    
    if (renderer_) {
        renderer_->shutdown();
        renderer_.reset();
    }
    
    ShaderManager::getInstance().shutdown();
    
    initialized_ = false;
    E2D_LOG_INFO("Render module shutdown");
}

void register_render_module() {
    if (s_renderModuleId != INVALID_MODULE_ID) return;
    
    s_renderModuleId = ModuleRegistry::instance().registerModule(
        makeUnique<RenderModuleConfig>(),
        []() -> UniquePtr<IModuleInitializer> {
            auto initializer = makeUnique<RenderModuleInitializer>();
            initializer->setModuleId(s_renderModuleId);
            return initializer;
        }
    );
}

namespace {
    struct RenderModuleAutoRegister {
        RenderModuleAutoRegister() {
            register_render_module();
        }
    };
    
    static RenderModuleAutoRegister s_autoRegister;
}

} // namespace extra2d
