#pragma once

#include <extra2d/core/service_interface.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/viewport_adapter.h>

namespace extra2d {

/**
 * @brief 相机服务接口
 */
class ICameraService : public IService {
public:
    virtual ~ICameraService() = default;

    virtual void setPosition(const Vec2& position) = 0;
    virtual void setPosition(float x, float y) = 0;
    virtual Vec2 getPosition() const = 0;

    virtual void setRotation(float degrees) = 0;
    virtual float getRotation() const = 0;

    virtual void setZoom(float zoom) = 0;
    virtual float getZoom() const = 0;

    virtual void setViewport(float left, float right, float bottom, float top) = 0;
    virtual Rect getViewport() const = 0;

    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;
    virtual glm::mat4 getViewProjectionMatrix() const = 0;

    virtual Vec2 screenToWorld(const Vec2& screenPos) const = 0;
    virtual Vec2 worldToScreen(const Vec2& worldPos) const = 0;

    virtual void move(const Vec2& offset) = 0;
    virtual void move(float x, float y) = 0;

    virtual void setBounds(const Rect& bounds) = 0;
    virtual void clearBounds() = 0;

    virtual void lookAt(const Vec2& target) = 0;

    virtual void setViewportConfig(const ViewportConfig& config) = 0;
    virtual const ViewportConfig& getViewportConfig() const = 0;
    virtual void updateViewport(int screenWidth, int screenHeight) = 0;
    virtual const ViewportResult& getViewportResult() const = 0;

    virtual void applyViewportAdapter() = 0;
};

/**
 * @brief 相机服务实现
 */
class CameraService : public ICameraService {
public:
    CameraService();
    explicit CameraService(float left, float right, float bottom, float top);
    ~CameraService() override = default;

    ServiceInfo getServiceInfo() const override;

    bool initialize() override;
    void shutdown() override;

    void setPosition(const Vec2& position) override;
    void setPosition(float x, float y) override;
    Vec2 getPosition() const override;

    void setRotation(float degrees) override;
    float getRotation() const override;

    void setZoom(float zoom) override;
    float getZoom() const override;

    void setViewport(float left, float right, float bottom, float top) override;
    Rect getViewport() const override;

    glm::mat4 getViewMatrix() const override;
    glm::mat4 getProjectionMatrix() const override;
    glm::mat4 getViewProjectionMatrix() const override;

    Vec2 screenToWorld(const Vec2& screenPos) const override;
    Vec2 worldToScreen(const Vec2& worldPos) const override;

    void move(const Vec2& offset) override;
    void move(float x, float y) override;

    void setBounds(const Rect& bounds) override;
    void clearBounds() override;

    void lookAt(const Vec2& target) override;

    void setViewportConfig(const ViewportConfig& config) override;
    const ViewportConfig& getViewportConfig() const override;
    void updateViewport(int screenWidth, int screenHeight) override;
    const ViewportResult& getViewportResult() const override;

    void applyViewportAdapter() override;

    Camera& getCamera() { return camera_; }
    const Camera& getCamera() const { return camera_; }
    ViewportAdapter& getViewportAdapter() { return viewportAdapter_; }
    const ViewportAdapter& getViewportAdapter() const { return viewportAdapter_; }

private:
    Camera camera_;
    ViewportAdapter viewportAdapter_;
};

} 
