#include "hello_module.h"
#include <extra2d/app/application.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/scene/scene.h>
#include <extra2d/services/scene_service.h>
#include <extra2d/utils/logger.h>

using namespace extra2d;

/**
 * @brief 自定义场景类
 *
 * 展示如何在场景中使用自定义模块
 */
class HelloScene : public Scene {
public:
  static Ptr<HelloScene> create() { return makeShared<HelloScene>(); }

  void onEnter() override {
    Scene::onEnter();
    E2D_LOG_INFO("HelloScene entered");

    setBackgroundColor(Color(0.1f, 0.1f, 0.2f, 1.0f));

    ModuleId helloId = get_hello_module_id();
    auto *initializer = ModuleRegistry::instance().getInitializer(helloId);
    if (initializer) {
      auto *helloInit = dynamic_cast<HelloModuleInitializer *>(initializer);
      if (helloInit) {
        E2D_LOG_INFO("Scene calling HelloModule from onEnter...");
        helloInit->sayHello();
      }
    }
  }

  void onUpdate(float dt) override {
    Scene::onUpdate(dt);

    time_ += dt;

    if (time_ >= 5.0f) {
      ModuleId helloId = get_hello_module_id();
      auto *initializer = ModuleRegistry::instance().getInitializer(helloId);
      if (initializer) {
        auto *helloInit = dynamic_cast<HelloModuleInitializer *>(initializer);
        if (helloInit) {
          E2D_LOG_INFO("Scene calling HelloModule from onUpdate...");
          helloInit->sayHello();
        }
      }
      time_ = 0.0f;
    }
  }

private:
  float time_ = 0.0f;
};

/**
 * @brief 应用程序入口
 */
int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  E2D_LOG_INFO("=== Hello Module Example ===");
  E2D_LOG_INFO("This example demonstrates how to create a custom module");
  E2D_LOG_INFO("");

  Application &app = Application::get();

  AppConfig appConfig;
  appConfig.appName = "HelloModule Example";
  appConfig.appVersion = "1.0.0";

  if (!app.init(appConfig)) {
    E2D_LOG_ERROR("Failed to initialize application");
    return 1;
  }

  E2D_LOG_INFO("");
  E2D_LOG_INFO("Application initialized successfully");
  E2D_LOG_INFO("HelloModule should have been auto-registered and initialized");
  E2D_LOG_INFO("");

  auto scene = HelloScene::create();
  app.enterScene(scene);

  E2D_LOG_INFO("Starting main loop...");
  E2D_LOG_INFO("Press ESC or close window to exit");
  E2D_LOG_INFO("");

  app.run();

  E2D_LOG_INFO("Application shutting down...");

  app.shutdown();

  E2D_LOG_INFO("Application shutdown complete");
  return 0;
}
