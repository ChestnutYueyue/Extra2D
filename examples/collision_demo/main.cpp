#include <cmath>
#include <extra2d/extra2d.h>

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

    // 加载字体并创建UI
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

    // 更新UI文本
    updateUI();

    // 检查退出按键
    auto &input = Application::instance().input();
    if (input.isButtonPressed(GamepadButton::Start)) {
      E2D_LOG_INFO("退出应用");
      Application::instance().quit();
    }
  }

private:
  /**
   * @brief 加载字体资源并创建UI文本
   */
  void loadFonts() {
    auto &resources = Application::instance().resources();
    titleFont_ = resources.loadFont("assets/font.ttf", 60, true);
    infoFont_ = resources.loadFont("assets/font.ttf", 28, true);

    // 创建标题文本
    titleText_ = Text::create("碰撞检测演示", titleFont_);
    titleText_->setPosition(50.0f, 30.0f);
    titleText_->setTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    addChild(titleText_);

    // 创建说明文本
    descText_ = Text::create("蓝色方块旋转并检测碰撞", infoFont_);
    descText_->setPosition(50.0f, 80.0f);
    descText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(descText_);

    collideHintText_ = Text::create("红色 = 检测到碰撞", infoFont_);
    collideHintText_->setPosition(50.0f, 105.0f);
    collideHintText_->setTextColor(Color(1.0f, 0.5f, 0.5f, 1.0f));
    addChild(collideHintText_);

    // 创建动态统计文本
    collisionText_ = Text::create("", infoFont_);
    collisionText_->setPosition(50.0f, 150.0f);
    collisionText_->setTextColor(Color(1.0f, 1.0f, 0.5f, 1.0f));
    addChild(collisionText_);

    fpsText_ = Text::create("", infoFont_);
    fpsText_->setPosition(50.0f, 175.0f);
    fpsText_->setTextColor(Color(0.8f, 1.0f, 0.8f, 1.0f));
    addChild(fpsText_);

    // 创建退出提示文本
    float screenHeight = static_cast<float>(Application::instance().getConfig().height);
    exitHintText_ = Text::create("按 + 键退出", infoFont_);
    exitHintText_->setPosition(50.0f, screenHeight - 50.0f);
    exitHintText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(exitHintText_);
  }

  /**
   * @brief 更新UI文本
   */
  void updateUI() {
    auto &app = Application::instance();

    // 使用 setFormat 更新动态文本
    collisionText_->setFormat("碰撞数: %zu", collisionCount_);
    fpsText_->setFormat("FPS: %u", app.fps());
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

  Ptr<CollisionBox> centerBox_;
  std::vector<Ptr<CollisionBox>> boxes_;
  float rotationAngle_ = 0.0f;
  float rotationSpeed_ = 60.0f; // 旋转速度（度/秒）
  size_t collisionCount_ = 0;

  // 字体资源
  Ptr<FontAtlas> titleFont_;
  Ptr<FontAtlas> infoFont_;

  // UI 文本组件
  Ptr<Text> titleText_;
  Ptr<Text> descText_;
  Ptr<Text> collideHintText_;
  Ptr<Text> collisionText_;
  Ptr<Text> fpsText_;
  Ptr<Text> exitHintText_;
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
