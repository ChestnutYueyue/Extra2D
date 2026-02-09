#include <extra2d/extra2d.h>
#include <extra2d/platform/platform_compat.h>

#ifdef PLATFORM_SWITCH
#include <switch.h>
#endif

using namespace extra2d;

// ============================================================================
// 字体配置
// ============================================================================

/**
 * @brief 获取字体候选列表
 * @return 按优先级排序的字体路径列表
 */
static std::vector<std::string> getFontCandidates() {
  return {
      FileSystem::resolvePath("font.ttf"),     // 微软雅黑（中文支持）
      FileSystem::resolvePath("Gasinamu.ttf"), // 备选字体
      FileSystem::resolvePath("default.ttf"),  // 默认字体
  };
}

/**
 * @brief 加载字体，支持多种字体后备
 * @param resources 资源管理器引用
 * @param fontSize 字体大小
 * @param useSDF 是否使用SDF渲染
 * @return 成功加载的字体图集，失败返回nullptr
 */
static Ptr<FontAtlas> loadFontWithFallbacks(ResourceManager &resources,
                                            int fontSize, bool useSDF) {
  auto candidates = getFontCandidates();

  for (const auto &fontPath : candidates) {
    auto font = resources.loadFont(fontPath, fontSize, useSDF);
    if (font) {
      E2D_LOG_INFO("成功加载字体: {}", fontPath);
      return font;
    }
    E2D_LOG_WARN("字体加载失败，尝试下一个: {}", fontPath);
  }

  E2D_LOG_ERROR("所有字体候选都加载失败！");
  return nullptr;
}

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
    font_ = loadFontWithFallbacks(resources, 48, true);

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

#ifdef PLATFORM_SWITCH
    // Switch: 使用手柄 START 按钮
    if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_START)) {
      E2D_LOG_INFO("退出应用 (START 按钮)");
      Application::instance().quit();
    }
#else
    // PC: 支持 ESC 键或手柄 START 按钮
    if (input.isKeyPressed(Key::Escape) ||
        input.isButtonPressed(SDL_CONTROLLER_BUTTON_START)) {
      E2D_LOG_INFO("退出应用 (ESC 键或 START 按钮)");
      Application::instance().quit();
    }
#endif
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
    Color white(1.0f, 1.0f, 1.0f, 1.0f);
    renderer.drawText(*font_, "你好世界", Vec2(centerX - 100.0f, centerY),
                      white);

    // 绘制提示文字（黄色）
    Color yellow(1.0f, 1.0f, 0.0f, 1.0f);
#ifdef PLATFORM_SWITCH
    renderer.drawText(*font_, "退出按键（START 按钮）",
                      Vec2(centerX - 80.0f, centerY + 50.0f), yellow);
#else
    renderer.drawText(*font_, "退出按键（ESC 或 START 按钮）",
                      Vec2(centerX - 80.0f, centerY + 50.0f), yellow);
#endif
  }

private:
  Ptr<FontAtlas> font_; // 字体图集
};

// ============================================================================
// 程序入口
// ============================================================================

/**
 * @brief 初始化应用配置
 * @return 应用配置结构体
 */
static AppConfig createAppConfig() {
  AppConfig config;
  config.title = "Easy2D - Hello World";
  config.width = 1280;
  config.height = 720;
  config.vsync = true;
  config.fpsLimit = 60;

#ifdef PLATFORM_PC
  // PC 端默认窗口模式
  config.fullscreen = false;
  config.resizable = true;
#endif

  return config;
}

/**
 * @brief 程序入口
 */
#ifdef _WIN32
int main(int argc, char *argv[])
#else
extern "C" int main(int argc, char *argv[])
#endif
{
  (void)argc;
  (void)argv;

  // 初始化日志系统
  Logger::init();
  Logger::setLevel(LogLevel::Debug);

  E2D_LOG_INFO("========================");
  E2D_LOG_INFO("Easy2D Hello World Demo");
  E2D_LOG_INFO("Platform: {}", platform::getPlatformName());
  E2D_LOG_INFO("========================");

  // 获取应用实例
  auto &app = Application::instance();

  // 配置应用
  auto config = createAppConfig();

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
