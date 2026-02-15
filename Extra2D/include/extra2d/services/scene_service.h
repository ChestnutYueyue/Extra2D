#pragma once

#include <extra2d/core/service_interface.h>
#include <extra2d/scene/scene_manager.h>

namespace extra2d {

/**
 * @brief 场景服务接口
 * 定义场景管理的抽象接口，便于测试和Mock
 */
class ISceneService : public IService {
public:
    virtual ~ISceneService() = default;

    virtual void runWithScene(Ptr<Scene> scene) = 0;
    virtual void replaceScene(Ptr<Scene> scene) = 0;
    virtual void pushScene(Ptr<Scene> scene) = 0;
    virtual void popScene() = 0;
    virtual void popToRootScene() = 0;
    virtual void popToScene(const std::string& name) = 0;

    virtual Ptr<Scene> getCurrentScene() const = 0;
    virtual Ptr<Scene> getPreviousScene() const = 0;
    virtual Ptr<Scene> getRootScene() const = 0;
    virtual Ptr<Scene> getSceneByName(const std::string& name) const = 0;

    virtual size_t getSceneCount() const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool hasScene(const std::string& name) const = 0;

    virtual void render(RenderBackend& renderer) = 0;
    virtual void collectRenderCommands(std::vector<RenderCommand>& commands) = 0;

    virtual bool isTransitioning() const = 0;
    virtual void setTransitionCallback(SceneManager::TransitionCallback callback) = 0;

    virtual void end() = 0;
    virtual void purgeCachedScenes() = 0;
    virtual void enterScene(Ptr<Scene> scene) = 0;
};

/**
 * @brief 场景服务实现
 * 包装 SceneManager，实现 IService 接口
 */
class SceneService : public ISceneService {
public:
    SceneService();
    ~SceneService() override = default;

    ServiceInfo getServiceInfo() const override;

    bool initialize() override;
    void shutdown() override;
    void update(float deltaTime) override;

    void runWithScene(Ptr<Scene> scene) override;
    void replaceScene(Ptr<Scene> scene) override;
    void pushScene(Ptr<Scene> scene) override;
    void popScene() override;
    void popToRootScene() override;
    void popToScene(const std::string& name) override;

    Ptr<Scene> getCurrentScene() const override;
    Ptr<Scene> getPreviousScene() const override;
    Ptr<Scene> getRootScene() const override;
    Ptr<Scene> getSceneByName(const std::string& name) const override;

    size_t getSceneCount() const override;
    bool isEmpty() const override;
    bool hasScene(const std::string& name) const override;

    void render(RenderBackend& renderer) override;
    void collectRenderCommands(std::vector<RenderCommand>& commands) override;

    bool isTransitioning() const override;
    void setTransitionCallback(SceneManager::TransitionCallback callback) override;

    void end() override;
    void purgeCachedScenes() override;
    void enterScene(Ptr<Scene> scene) override;

    SceneManager& getManager() { return manager_; }
    const SceneManager& getManager() const { return manager_; }

private:
    SceneManager manager_;
};

} 
