#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/types.h>

namespace extra2d {

class RenderModuleConfig : public IModuleConfig {
public:
    BackendType backend = BackendType::OpenGL;
    bool vsync = true;
    int targetFPS = 60;
    int multisamples = 0;
    bool sRGBFramebuffer = false;
    int spriteBatchSize = 1000;
    
    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.name = "Render";
        info.version = "1.0.0";
        info.priority = ModulePriority::Graphics;
        info.enabled = true;
        return info;
    }
    
    std::string getConfigSectionName() const override { return "render"; }
    
    bool validate() const override;
    void applyPlatformConstraints(PlatformType platform) override;
    void resetToDefaults() override;
    bool loadFromJson(const void* jsonData) override;
    bool saveToJson(void* jsonData) const override;
};

class RenderModuleInitializer : public IModuleInitializer {
public:
    RenderModuleInitializer();
    ~RenderModuleInitializer() override;
    
    ModuleId getModuleId() const override { return moduleId_; }
    ModulePriority getPriority() const override { return ModulePriority::Graphics; }
    std::vector<ModuleId> getDependencies() const override;
    
    bool initialize(const IModuleConfig* config) override;
    void shutdown() override;
    bool isInitialized() const override { return initialized_; }
    
    void setModuleId(ModuleId id) { moduleId_ = id; }
    void setWindow(IWindow* window) { window_ = window; }
    
    RenderBackend* getRenderer() const { return renderer_.get(); }

private:
    ModuleId moduleId_ = INVALID_MODULE_ID;
    IWindow* window_ = nullptr;
    UniquePtr<RenderBackend> renderer_;
    bool initialized_ = false;
};

ModuleId get_render_module_id();
void register_render_module();

} // namespace extra2d
