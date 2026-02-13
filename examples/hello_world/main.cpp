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
      return;
    }

    // 创建 "你好世界" 文本组件 - 使用屏幕空间（固定位置，不随相机移动）
    auto text1 = Text::create("你好世界", font_);
    text1->setCoordinateSpace(CoordinateSpace::Screen);
    text1->setScreenPosition(640.0f, 360.0f); // 屏幕中心
    text1->setAnchor(0.5f, 0.5f);             // 中心锚点，让文字中心对准位置
    text1->setTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    addChild(text1);

    // 创建提示文本组件 - 使用屏幕空间，固定在屏幕底部
    auto text2 = Text::create("退出按键（START 按钮）", font_);
    text2->setCoordinateSpace(CoordinateSpace::Screen);
    text2->setScreenPosition(640.0f, 650.0f); // 屏幕底部
    text2->setAnchor(0.5f, 0.5f);
    text2->setTextColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
    addChild(text2);

    // 创建相机空间文本 - 跟随相机但保持相对偏移
    auto text3 = Text::create("相机空间文本", font_);
    text3->setCoordinateSpace(CoordinateSpace::Camera);
    text3->setCameraOffset(50.0f, 50.0f); // 相机左上角偏移（屏幕坐标系Y向下）
    text3->setAnchor(0.0f, 0.0f);         // 左上角锚点，文字从指定位置开始显示
    text3->setTextColor(Color(0.0f, 1.0f, 1.0f, 1.0f));
    addChild(text3);

    // 创建世界空间文本 - 随相机移动（默认行为）
    auto text4 = Text::create("世界空间文本", font_);
    text4->setCoordinateSpace(CoordinateSpace::World);
    text4->setPosition(100.0f, 100.0f); // 世界坐标
    text4->setAnchor(0.0f, 0.0f);       // 左上角锚点，文字从指定位置开始显示
    text4->setTextColor(Color(1.0f, 0.5f, 0.5f, 1.0f));
    addChild(text4);
  }

  /**
   * @brief 每帧更新时调用
   * @param dt 时间间隔（秒）
   */
  void onUpdate(float dt) override {
    Scene::onUpdate(dt);

    // 检查退出按键
    auto &input = Application::instance().input();

    // 使用手柄 START 按钮退出 (GamepadButton::Start)
    if (input.isButtonPressed(GamepadButton::Start)) {
      E2D_LOG_INFO("退出应用 (START 按钮)");
      Application::instance().quit();
    }
  }

private:
  Ptr<FontAtlas> font_; // 字体图集
};

// ============================================================================
// 程序入口
// ============================================================================

int main(int argc, char **argv) {
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
