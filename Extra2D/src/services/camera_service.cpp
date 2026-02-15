#include <extra2d/services/camera_service.h>

namespace extra2d {

CameraService::CameraService() {
    info_.name = "CameraService";
    info_.priority = ServicePriority::Camera;
    info_.enabled = true;
}

CameraService::CameraService(float left, float right, float bottom, float top)
    : camera_(left, right, bottom, top) {
    info_.name = "CameraService";
    info_.priority = ServicePriority::Camera;
    info_.enabled = true;
}

ServiceInfo CameraService::getServiceInfo() const {
    return info_;
}

bool CameraService::initialize() {
    camera_.setViewportAdapter(&viewportAdapter_);
    setState(ServiceState::Running);
    return true;
}

void CameraService::shutdown() {
    setState(ServiceState::Stopped);
}

void CameraService::setPosition(const Vec2& position) {
    camera_.setPos(position);
}

void CameraService::setPosition(float x, float y) {
    camera_.setPos(x, y);
}

Vec2 CameraService::getPosition() const {
    return camera_.getPosition();
}

void CameraService::setRotation(float degrees) {
    camera_.setRotation(degrees);
}

float CameraService::getRotation() const {
    return camera_.getRotation();
}

void CameraService::setZoom(float zoom) {
    camera_.setZoom(zoom);
}

float CameraService::getZoom() const {
    return camera_.getZoom();
}

void CameraService::setViewport(float left, float right, float bottom, float top) {
    camera_.setViewport(left, right, bottom, top);
}

Rect CameraService::getViewport() const {
    return camera_.getViewport();
}

glm::mat4 CameraService::getViewMatrix() const {
    return camera_.getViewMatrix();
}

glm::mat4 CameraService::getProjectionMatrix() const {
    return camera_.getProjectionMatrix();
}

glm::mat4 CameraService::getViewProjectionMatrix() const {
    return camera_.getViewProjectionMatrix();
}

Vec2 CameraService::screenToWorld(const Vec2& screenPos) const {
    return camera_.screenToWorld(screenPos);
}

Vec2 CameraService::worldToScreen(const Vec2& worldPos) const {
    return camera_.worldToScreen(worldPos);
}

void CameraService::move(const Vec2& offset) {
    camera_.move(offset);
}

void CameraService::move(float x, float y) {
    camera_.move(x, y);
}

void CameraService::setBounds(const Rect& bounds) {
    camera_.setBounds(bounds);
}

void CameraService::clearBounds() {
    camera_.clearBounds();
}

void CameraService::lookAt(const Vec2& target) {
    camera_.lookAt(target);
}

void CameraService::setViewportConfig(const ViewportConfig& config) {
    viewportAdapter_.setConfig(config);
}

const ViewportConfig& CameraService::getViewportConfig() const {
    return viewportAdapter_.getConfig();
}

void CameraService::updateViewport(int screenWidth, int screenHeight) {
    viewportAdapter_.update(screenWidth, screenHeight);
}

const ViewportResult& CameraService::getViewportResult() const {
    return viewportAdapter_.getResult();
}

void CameraService::applyViewportAdapter() {
    camera_.applyViewportAdapter();
}

} 
