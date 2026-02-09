#include <cmath>
#include <extra2d/extra2d.h>
#include <sstream>

using namespace extra2d;

// ============================================================================
// 碰撞测试节点 - 有实际边界框
// ============================================================================
class CollisionBox : public Node {
public:
  CollisionBox(float width, float height, const Color &color)
      : width_(width), height_(height), color_(color), isColliding_(false) {
    // 启用空间索引，这是碰撞检测的关键
    setSpatialIndexed(true);
  }

  void setColliding(bool colliding) { isColliding_ = colliding; }

  Rect getBoundingBox() const override {
    // 返回实际的矩形边界
    Vec2 pos = getPosition();
    return Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_);
  }

  void onRender(RenderBackend &renderer) override {
    Vec2 pos = getPosition();

    // 绘制填充矩形
    Color fillColor = isColliding_ ? Color(1.0f, 0.2f, 0.2f, 0.8f) : color_;
    renderer.fillRect(
        Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_),
        fillColor);

    // 绘制边框
    Color borderColor = isColliding_ ? Color(1.0f, 0.0f, 0.0f, 1.0f)
                                     : Color(1.0f, 1.0f, 1.0f, 0.5f);
    float borderWidth = isColliding_ ? 3.0f : 2.0f;
    renderer.drawRect(
        Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_),
        borderColor, borderWidth);
  }

private:
  float width_, height_;
  Color color_;
  bool isColliding_;
};

// ============================================================================
// 碰撞检测场景
// ============================================================================
class CollisionDemoScene : public Scene {
public:
  void onEnter() override {
    E2D_LOG_INFO("CollisionDemoScene::onEnter - 碰撞检测演示");

    // 设置背景色
    setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));

    // 获取屏幕中心
    auto &app = Application::instance();
    float centerX = app.getConfig().width / 2.0f;
    float centerY = app.getConfig().height / 2.0f;

    // 创建静态碰撞框
    createStaticBoxes(centerX, centerY);

    // 创建移动的中心方块
    centerBox_ =
        makePtr<CollisionBox>(80.0f, 80.0f, Color(0.2f, 0.6f, 1.0f, 0.8f));
    centerBox_->setPosition(Vec2(centerX, centerY));
    addChild(centerBox_);

    // 加载字体
    loadFonts();

    E2D_LOG_INFO("创建了 {} 个碰撞框", boxes_.size() + 1);
  }

  void onUpdate(float dt) override {
    Scene::onUpdate(dt);

    // 旋转中心方块
    rotationAngle_ += rotationSpeed_ * dt;
    if (rotationAngle_ >= 360.0f)
      rotationAngle_ -= 360.0f;

    // 让中心方块沿圆形路径移动
    float radius = 150.0f;
    float rad = rotationAngle_ * 3.14159f / 180.0f;

    auto &app = Application::instance();
    Vec2 center =
        Vec2(app.getConfig().width / 2.0f, app.getConfig().height / 2.0f);

    centerBox_->setPosition(Vec2(center.x + std::cos(rad) * radius,
                                 center.y + std::sin(rad) * radius));
    centerBox_->setRotation(rotationAngle_);

    // 执行碰撞检测
    performCollisionDetection();

    // 检查退出按键
    auto &input = Application::instance().input();
    if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_START)) {
      E2D_LOG_INFO("退出应用");
      Application::instance().quit();
    }
  }

  void onRender(RenderBackend &renderer) override {
    Scene::onRender(renderer);

    // 绘制说明文字
    drawUI(renderer);
  }

private:
  /**
   * @brief 加载字体资源
   */
  void loadFonts() {
    auto &resources = Application::instance().resources();

    // 使用后备字体加载功能
    std::vector<std::string> fontPaths = {
        "romfs:/assets/font.ttf" // 备选字体
    };

    titleFont_ = resources.loadFontWithFallbacks(fontPaths, 60, true);
    infoFont_ = resources.loadFontWithFallbacks(fontPaths, 28, true);

    if (!titleFont_) {
      E2D_LOG_WARN("无法加载标题字体");
    }
    if (!infoFont_) {
      E2D_LOG_WARN("无法加载信息字体");
    }
  }

  /**
   * @brief 创建静态碰撞框
   */
  void createStaticBoxes(float centerX, float centerY) {
    // 创建围绕中心的静态碰撞框
    std::vector<std::pair<Vec2, Color>> positions = {
        {Vec2(centerX - 200, centerY - 150), Color(0.3f, 1.0f, 0.3f, 0.7f)},
        {Vec2(centerX + 200, centerY - 150), Color(1.0f, 0.3f, 0.3f, 0.7f)},
        {Vec2(centerX - 200, centerY + 150), Color(0.3f, 0.3f, 1.0f, 0.7f)},
        {Vec2(centerX + 200, centerY + 150), Color(1.0f, 1.0f, 0.3f, 0.7f)},
        {Vec2(centerX, centerY - 220), Color(1.0f, 0.3f, 1.0f, 0.7f)},
        {Vec2(centerX, centerY + 220), Color(0.3f, 1.0f, 1.0f, 0.7f)},
    };

    for (const auto &[pos, color] : positions) {
      auto box = makePtr<CollisionBox>(70.0f, 70.0f, color);
      box->setPosition(pos);
      addChild(box);
      boxes_.push_back(box);
    }
  }

  /**
   * @brief 执行碰撞检测
   */
  void performCollisionDetection() {
    // 清除之前的碰撞状态
    centerBox_->setColliding(false);
    for (auto &box : boxes_) {
      box->setColliding(false);
    }

    // 使用空间索引进行碰撞检测
    auto collisions = queryCollisions();

    collisionCount_ = collisions.size();

    // 标记碰撞的节点
    for (const auto &[nodeA, nodeB] : collisions) {
      if (auto boxA = dynamic_cast<CollisionBox *>(nodeA)) {
        boxA->setColliding(true);
      }
      if (auto boxB = dynamic_cast<CollisionBox *>(nodeB)) {
        boxB->setColliding(true);
      }
    }
  }

  /**
   * @brief 绘制UI界面
   */
  void drawUI(RenderBackend &renderer) {
    if (!titleFont_ || !infoFont_)
      return;

    auto &app = Application::instance();

    // 绘制标题
    renderer.drawText(*titleFont_, "碰撞检测演示", Vec2(50.0f, 30.0f),
                      Color(1.0f, 1.0f, 1.0f, 1.0f));

    // 绘制说明文字
    renderer.drawText(*infoFont_, "蓝色方块旋转并检测碰撞", Vec2(50.0f, 80.0f),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    renderer.drawText(*infoFont_, "红色 = 检测到碰撞", Vec2(50.0f, 105.0f),
                      Color(1.0f, 0.5f, 0.5f, 1.0f));

    // 绘制碰撞统计
    std::stringstream ss;
    ss << "碰撞数: " << collisionCount_;
    renderer.drawText(*infoFont_, ss.str(), Vec2(50.0f, 150.0f),
                      Color(1.0f, 1.0f, 0.5f, 1.0f));

    // 绘制 FPS
    ss.str("");
    ss << "FPS: " << app.fps();
    renderer.drawText(*infoFont_, ss.str(), Vec2(50.0f, 175.0f),
                      Color(0.8f, 1.0f, 0.8f, 1.0f));

    // 绘制操作提示
    float screenHeight = static_cast<float>(app.getConfig().height);
    renderer.drawText(*infoFont_, "按 + 键退出",
                      Vec2(50.0f, screenHeight - 50.0f),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
  }

  Ptr<CollisionBox> centerBox_;
  std::vector<Ptr<CollisionBox>> boxes_;
  float rotationAngle_ = 0.0f;
  float rotationSpeed_ = 60.0f; // 旋转速度（度/秒）
  size_t collisionCount_ = 0;

  // 字体资源
  Ptr<FontAtlas> titleFont_;
  Ptr<FontAtlas> infoFont_;
};

// ============================================================================
// 程序入口
// ============================================================================

extern "C" int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  // 初始化日志系统
  Logger::init();
  Logger::setLevel(LogLevel::Debug);

  E2D_LOG_INFO("========================");
  E2D_LOG_INFO("Easy2D 碰撞检测演示");
  E2D_LOG_INFO("========================");

  // 获取应用实例
  auto &app = Application::instance();

  // 配置应用
  AppConfig config;
  config.title = "Easy2D - 碰撞检测演示";
  config.width = 1280;
  config.height = 720;
  config.vsync = true;
  config.fpsLimit = 60;

  // 初始化应用
  if (!app.init(config)) {
    E2D_LOG_ERROR("应用初始化失败！");
    return -1;
  }

  // 进入场景
  app.enterScene(makePtr<CollisionDemoScene>());

  E2D_LOG_INFO("开始主循环...");

  // 运行应用
  app.run();

  E2D_LOG_INFO("应用结束");

  return 0;
}
