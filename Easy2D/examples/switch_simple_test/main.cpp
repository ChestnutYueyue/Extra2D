#include <easy2d/easy2d.h>
#include <iostream>

// Nintendo Switch 平台支持
#ifdef __SWITCH__
#include <switch.h>
#endif

using namespace easy2d;

// 加载系统字体的辅助函数
easy2d::Ptr<easy2d::FontAtlas> loadSystemFont(int size) {
  auto &resources = Application::instance().resources();
  easy2d::Ptr<easy2d::FontAtlas> font = nullptr;

#ifdef __SWITCH__
  // Nintendo Switch 系统字体路径
  const char *switchFontPaths[] = {
      "romfs:/font.TTF",               // RomFS 中的字体（注意大小写）
      "romfs:/font.ttf",               // 小写备选
      "sdmc:/switch/pushbox/font.ttf", // SD 卡字体
      "/switch/pushbox/font.ttf",      // 绝对路径
  };

  for (auto *path : switchFontPaths) {
    font = resources.loadFont(path, size);
    if (font) {
      E2D_LOG_INFO("Loaded Switch font: %s", path);
      return font;
    }
  }
#else
  // Windows 系统字体
  const char *winFontPaths[] = {
      "C:/Windows/Fonts/arial.ttf",
      "C:/Windows/Fonts/segoeui.ttf",
      "C:/Windows/Fonts/simsun.ttc",
      "C:/Windows/Fonts/simhei.ttf",
  };

  for (auto *path : winFontPaths) {
    font = resources.loadFont(path, size);
    if (font) {
      E2D_LOG_INFO("Loaded Windows font: %s", path);
      return font;
    }
  }
#endif

  E2D_LOG_WARN("Failed to load any system font!");
  return nullptr;
}

class SimpleScene : public Scene {
public:
  SimpleScene() {
    // 设置背景颜色为深蓝色 (使用 RGB 值)
    setBackgroundColor(Color(0.0f, 0.0f, 0.5f, 1.0f));

    // 创建一个红色填充矩形（用于测试渲染）
    // 矩形在屏幕左上角，大小 200x200
    Rect rectBounds(50, 50, 200, 200);  // x, y, width, height
    auto rect = ShapeNode::createFilledRect(rectBounds, Colors::Red);
    addChild(rect);

    // 创建一个黄色圆形
    auto circle = ShapeNode::createFilledCircle(Vec2(400, 300), 100, Colors::Yellow);
    addChild(circle);

    // 创建一个绿色三角形
    auto triangle = ShapeNode::createFilledTriangle(
        Vec2(700, 200), Vec2(600, 400), Vec2(800, 400), Colors::Green);
    addChild(triangle);

    // 创建一个简单的标签
    auto label = Text::create("Hello Switch!");

    // 加载系统字体
    auto font = loadSystemFont(48);
    if (font) {
      label->setFont(font);
      E2D_LOG_INFO("Font loaded successfully!");
    } else {
      E2D_LOG_WARN("Font loading failed!");
    }

    label->setTextColor(Colors::White);
    label->setPosition(640, 100);  // 屏幕上方居中
    label->setAnchor(0.5f, 0.5f);
    addChild(label);

    E2D_LOG_INFO("SimpleScene created successfully!");
  }
};

int main(int argc, char **argv) {
  // Nintendo Switch 初始化
#ifdef __SWITCH__
  Result rc;

  // 初始化 nxlink 调试输出（可选）
  rc = socketInitializeDefault();
  if (R_FAILED(rc)) {
    std::cout << "socketInitializeDefault failed" << std::endl;
  } else {
    nxlinkStdio();
    std::cout << "nxlink initialized!" << std::endl;
  }

  // 初始化 RomFS（可选）
  rc = romfsInit();
  if (R_FAILED(rc)) {
    std::cout << "romfsInit failed" << std::endl;
  }
#endif

  std::cout << "Starting Easy2D Simple Test..." << std::endl;

  // 配置应用
  AppConfig config;
  config.title = "Switch Simple Test";
  config.width = 1280;
  config.height = 720;

  // 初始化 Easy2D
  if (!Application::instance().init(config)) {
    std::cerr << "Failed to initialize Easy2D!" << std::endl;
    return -1;
  }

  std::cout << "Easy2D initialized successfully!" << std::endl;

  // 创建场景并设置到场景管理器
  auto scene = std::make_shared<SimpleScene>();
  Application::instance().scenes().pushScene(scene);

  std::cout << "Scene started!" << std::endl;

  // 运行主循环
  Application::instance().run();

  // 清理
  Application::instance().shutdown();

#ifdef __SWITCH__
  romfsExit();
  socketExit();
#endif

  return 0;
}
