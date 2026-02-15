#include <extra2d/services/scene_service.h>

namespace extra2d {

SceneService::SceneService() {
    info_.name = "SceneService";
    info_.priority = ServicePriority::Scene;
    info_.enabled = true;
}

ServiceInfo SceneService::getServiceInfo() const {
    return info_;
}

bool SceneService::initialize() {
    setState(ServiceState::Running);
    return true;
}

void SceneService::shutdown() {
    manager_.end();
    setState(ServiceState::Stopped);
}

void SceneService::update(float deltaTime) {
    if (getState() == ServiceState::Running) {
        manager_.update(deltaTime);
    }
}

void SceneService::runWithScene(Ptr<Scene> scene) {
    manager_.runWithScene(scene);
}

void SceneService::replaceScene(Ptr<Scene> scene) {
    manager_.replaceScene(scene);
}

void SceneService::pushScene(Ptr<Scene> scene) {
    manager_.pushScene(scene);
}

void SceneService::popScene() {
    manager_.popScene();
}

void SceneService::popToRootScene() {
    manager_.popToRootScene();
}

void SceneService::popToScene(const std::string& name) {
    manager_.popToScene(name);
}

Ptr<Scene> SceneService::getCurrentScene() const {
    return manager_.getCurrentScene();
}

Ptr<Scene> SceneService::getPreviousScene() const {
    return manager_.getPreviousScene();
}

Ptr<Scene> SceneService::getRootScene() const {
    return manager_.getRootScene();
}

Ptr<Scene> SceneService::getSceneByName(const std::string& name) const {
    return manager_.getSceneByName(name);
}

size_t SceneService::getSceneCount() const {
    return manager_.getSceneCount();
}

bool SceneService::isEmpty() const {
    return manager_.isEmpty();
}

bool SceneService::hasScene(const std::string& name) const {
    return manager_.hasScene(name);
}

void SceneService::render(RenderBackend& renderer) {
    manager_.render(renderer);
}

void SceneService::collectRenderCommands(std::vector<RenderCommand>& commands) {
    manager_.collectRenderCommands(commands);
}

bool SceneService::isTransitioning() const {
    return manager_.isTransitioning();
}

void SceneService::setTransitionCallback(SceneManager::TransitionCallback callback) {
    manager_.setTransitionCallback(callback);
}

void SceneService::end() {
    manager_.end();
}

void SceneService::purgeCachedScenes() {
    manager_.purgeCachedScenes();
}

void SceneService::enterScene(Ptr<Scene> scene) {
    manager_.enterScene(scene);
}

} 
