#include <extra2d/extra2d.h>

using namespace extra2d;

/**
 * @brief Hello World 场景类
 * 显示简单的精灵和形状
 */
class HelloWorldScene : public Scene {
public:
  /**
   * @brief 场景进入时调用
   */
  void onEnter() override {
    E2D_LOG_INFO("HelloWorldScene::onEnter - 进入场景");

    setBackgroundColor(Color(0.1f, 0.1f, 0.3f, 1.0f));

    auto center = ShapeNode::createFilledCircle(
        Vec2(640.0f, 360.0f), 50.0f, Color(1.0f, 1.0f, 1.0f, 1.0f), 32);
    center->setName("center_circle");
    addChild(center);

    auto rect = ShapeNode::createFilledRect(
        Rect(100.0f, 100.0f, 200.0f, 100.0f), Color(1.0f, 0.5f, 0.5f, 1.0f));
    rect->setName("red_rect");
    addChild(rect);

    auto line = ShapeNode::createLine(Vec2(500.0f, 500.0f), Vec2(780.0f, 500.0f),
                                      Color(0.0f, 1.0f, 1.0f, 1.0f), 3.0f);
    line->setName("cyan_line");
    addChild(line);

    auto triangle = ShapeNode::createFilledTriangle(
        Vec2(900.0f, 200.0f), Vec2(1000.0f, 350.0f), Vec2(800.0f, 350.0f),
        Color(0.5f, 1.0f, 0.5f, 1.0f));
    triangle->setName("green_triangle");
    addChild(triangle);
  }

  /**
   * @brief 每帧更新时调用
   * @param dt 时间间隔（秒）
   */
  void onUpdate(float dt) override {
    Scene::onUpdate(dt);

    auto &input = Application::instance().input();

    if (input.isButtonPressed(GamepadButton::Start)) {
      E2D_LOG_INFO("退出应用 (START 按钮)");
      Application::instance().quit();
    }
  }
};

/**
 * @brief 程序入口
 */
int main(int argc, char **argv) {
  Logger::init();
  Logger::setLevel(LogLevel::Debug);

  E2D_LOG_INFO("========================");
  E2D_LOG_INFO("Easy2D Hello World Demo");
  E2D_LOG_INFO("========================");

  auto &app = Application::instance();

  AppConfig config;
  config.title = "Easy2D - Hello World";
  config.width = 1280;
  config.height = 720;
  config.vsync = true;
  config.fpsLimit = 60;

  if (!app.init(config)) {
    E2D_LOG_ERROR("应用初始化失败！");
    return -1;
  }

  app.enterScene(makePtr<HelloWorldScene>());

  E2D_LOG_INFO("开始主循环...");

  app.run();

  E2D_LOG_INFO("应用结束");

  return 0;
}
