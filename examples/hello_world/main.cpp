#include <extra2d/extra2d.h>

using namespace extra2d;


// ============================================================================
// Hello World 场景
// ============================================================================

/**
 * @brief Hello World 场景类
 * 显示简单的 "Hello World" 文字
 */
class HelloWorldScene : public Scene {
public:
  /**
   * @brief 场景进入时调用
   */
  void onEnter() override {
    E2D_LOG_INFO("HelloWorldScene::onEnter - 进入场景");

    // 设置背景颜色为深蓝色
    setBackgroundColor(Color(0.1f, 0.1f, 0.3f, 1.0f));

    // 加载字体（支持多种字体后备）
    auto &resources = Application::instance().resources();
    font_ = resources.loadFont("assets/font.ttf", 48, true);

    if (!font_) {
      E2D_LOG_ERROR("字体加载失败，文字渲染将不可用！");
    }
  }

  /**
   * @brief 每帧更新时调用
   * @param dt 时间间隔（秒）
   */
  void onUpdate(float dt) override {
    Scene::onUpdate(dt);

    // 检查退出按键
    auto &input = Application::instance().input();

    // Switch: 使用手柄 START 按钮
    if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_START)) {
      E2D_LOG_INFO("退出应用 (START 按钮)");
      Application::instance().quit();
    }
  }

  /**
   * @brief 渲染时调用
   * @param renderer 渲染后端
   */
  void onRender(RenderBackend &renderer) override {
    Scene::onRender(renderer);

    if (!font_)
      return;

    // 屏幕中心位置
    float centerX = 640.0f; // 1280 / 2
    float centerY = 360.0f; // 720 / 2

    // 绘制 "你好世界" 文字（白色，居中）
    renderer.drawText(*font_, "你好世界", Vec2(centerX - 100.0f, centerY),Color(1.0f, 1.0f, 1.0f, 1.0f));

    // 绘制提示文字（黄色）
    renderer.drawText(*font_, "退出按键（START 按钮）",Vec2(centerX - 80.0f, centerY + 50.0f), Color(1.0f, 1.0f, 0.0f, 1.0f));
  }

private:
  Ptr<FontAtlas> font_; // 字体图集
};

// ============================================================================
// 程序入口
// ============================================================================

int main(int argc, char **argv)
{
  // 初始化日志系统
  Logger::init();
  Logger::setLevel(LogLevel::Debug);

  E2D_LOG_INFO("========================");
  E2D_LOG_INFO("Easy2D Hello World Demo");
  E2D_LOG_INFO("========================");

  // 获取应用实例
  auto &app = Application::instance();

  // 配置应用
  AppConfig config;
  config.title = "Easy2D - Hello World";
  config.width = 1280;
  config.height = 720;
  config.vsync = true;
  config.fpsLimit = 60;

  // 初始化应用
  if (!app.init(config)) {
    E2D_LOG_ERROR("应用初始化失败！");
    return -1;
  }

  // 进入 Hello World 场景
  app.enterScene(makePtr<HelloWorldScene>());

  E2D_LOG_INFO("开始主循环...");

  // 运行应用
  app.run();

  E2D_LOG_INFO("应用结束");

  return 0;
}
