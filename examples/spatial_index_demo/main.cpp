#include <cmath>
#include <extra2d/extra2d.h>
#include <iomanip>
#include <random>
#include <sstream>

using namespace extra2d;

// ============================================================================
// 性能统计
// ============================================================================
struct PerformanceStats {
  double updateTime = 0.0;
  double collisionTime = 0.0;
  double renderTime = 0.0;
  size_t collisionCount = 0;
  size_t nodeCount = 0;
  const char *strategyName = "Unknown";
};

// ============================================================================
// 碰撞节点 - 使用引擎自带的空间索引功能
// ============================================================================
class PhysicsNode : public Node {
public:
  PhysicsNode(float size, const Color &color, int id)
      : size_(size), color_(color), id_(id), isColliding_(false) {
    // 启用引擎自带的空间索引功能
    // 这是关键：设置 spatialIndexed_ = true 让节点参与空间索引
    setSpatialIndexed(true);

    // 随机速度
    std::random_device rd;
    std::mt19937 gen(rd() + id);
    std::uniform_real_distribution<float> velDist(-150.0f, 150.0f);
    velocity_ = Vec2(velDist(gen), velDist(gen));
  }

  void setColliding(bool colliding) { isColliding_ = colliding; }
  bool isColliding() const { return isColliding_; }
  int getId() const { return id_; }

  // 必须实现 getBoundingBox() 才能参与空间索引碰撞检测
  Rect getBoundingBox() const override {
    Vec2 pos = getPosition();
    return Rect(pos.x - size_ / 2, pos.y - size_ / 2, size_, size_);
  }

  void update(float dt, float screenWidth, float screenHeight) {
    Vec2 pos = getPosition();
    pos = pos + velocity_ * dt;

    // 边界反弹
    if (pos.x < size_ / 2 || pos.x > screenWidth - size_ / 2) {
      velocity_.x = -velocity_.x;
      pos.x = std::clamp(pos.x, size_ / 2, screenWidth - size_ / 2);
    }
    if (pos.y < size_ / 2 || pos.y > screenHeight - size_ / 2) {
      velocity_.y = -velocity_.y;
      pos.y = std::clamp(pos.y, size_ / 2, screenHeight - size_ / 2);
    }

    setPosition(pos);
  }

  void onRender(RenderBackend &renderer) override {
    Vec2 pos = getPosition();

    // 碰撞时变红色
    Color fillColor = isColliding_ ? Color(1.0f, 0.2f, 0.2f, 0.9f) : color_;
    renderer.fillRect(Rect(pos.x - size_ / 2, pos.y - size_ / 2, size_, size_),
                      fillColor);

    // 绘制边框
    Color borderColor = isColliding_ ? Color(1.0f, 0.0f, 0.0f, 1.0f)
                                     : Color(0.3f, 0.3f, 0.3f, 0.5f);
    renderer.drawRect(Rect(pos.x - size_ / 2, pos.y - size_ / 2, size_, size_),
                      borderColor, 1.0f);
  }

private:
  float size_;
  Color color_;
  int id_;
  bool isColliding_;
  Vec2 velocity_;
};

// ============================================================================
// 空间索引演示场景
// ============================================================================
class SpatialIndexDemoScene : public Scene {
public:
  void onEnter() override {
    // 必须先调用父类的 onEnter()，这样才能正确设置 running_ 状态
    // 并触发子节点的 onAttachToScene，将节点注册到空间索引
    Scene::onEnter();

    E2D_LOG_INFO("SpatialIndexDemoScene::onEnter - 引擎空间索引演示");

    auto &app = Application::instance();
    screenWidth_ = static_cast<float>(app.getConfig().width);
    screenHeight_ = static_cast<float>(app.getConfig().height);

    // 设置背景色
    setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));

    // 创建100个碰撞节点
    createNodes(100);

    // 加载字体
    loadFonts();

    E2D_LOG_INFO("创建了 {} 个碰撞节点", nodes_.size());
    E2D_LOG_INFO("空间索引已启用: {}", isSpatialIndexingEnabled());
  }

  void onExit() override {
    // 先清理 nodes_ 向量
    nodes_.clear();

    // 显式移除所有子节点，确保在场景析构前正确清理空间索引
    // 这必须在 Scene::onExit() 之前调用，因为 onExit() 会将 running_ 设为 false
    removeAllChildren();

    Scene::onExit();
  }

  void onUpdate(float dt) override {
    Scene::onUpdate(dt);

    auto startTime = std::chrono::high_resolution_clock::now();

    // 更新所有节点位置
    for (auto &node : nodes_) {
      node->update(dt, screenWidth_, screenHeight_);
    }

    auto updateEndTime = std::chrono::high_resolution_clock::now();
    stats_.updateTime =
        std::chrono::duration<double, std::milli>(updateEndTime - startTime)
            .count();

    // 使用引擎自带的空间索引进行碰撞检测
    performCollisionDetection();

    auto collisionEndTime = std::chrono::high_resolution_clock::now();
    stats_.collisionTime = std::chrono::duration<double, std::milli>(
                               collisionEndTime - updateEndTime)
                               .count();

    stats_.nodeCount = nodes_.size();

    // 获取当前使用的空间索引策略
    stats_.strategyName = getSpatialManager().getStrategyName();

    // 检查退出按键
    auto &input = Application::instance().input();
    if (input.isButtonPressed(GamepadButton::Start)) {
      E2D_LOG_INFO("退出应用");
      Application::instance().quit();
    }

    // 按 A 键添加节点
    if (input.isButtonPressed(GamepadButton::A)) {
      addNodes(100);
    }

    // 按 B 键减少节点
    if (input.isButtonPressed(GamepadButton::B)) {
      removeNodes(100);
    }

    // 按 X 键切换空间索引策略
    if (input.isButtonPressed(GamepadButton::X)) {
      toggleSpatialStrategy();
    }
  }

  void onRender(RenderBackend &renderer) override {
    auto renderStart = std::chrono::high_resolution_clock::now();

    Scene::onRender(renderer);

    auto renderEnd = std::chrono::high_resolution_clock::now();
    stats_.renderTime =
        std::chrono::duration<double, std::milli>(renderEnd - renderStart)
            .count();

    // 更新UI文本内容
    updateUI();

    // 绘制图例方块（Text组件会自动渲染）
    drawLegend(renderer);
  }

private:
  /**
   * @brief 加载字体资源并创建UI文本组件
   */
  void loadFonts() {
    auto &resources = Application::instance().resources();

    titleFont_ = resources.loadFont("assets/font.ttf", 28, true);
    infoFont_ = resources.loadFont("assets/font.ttf", 16, true);

    // 创建标题文本
    titleText_ = Text::create("引擎空间索引演示", titleFont_);
    titleText_->setPosition(30.0f, 20.0f);
    titleText_->setTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    addChild(titleText_);

    float x = 30.0f;
    float y = 60.0f;
    float lineHeight = 22.0f;

    // 创建统计信息文本
    nodeCountText_ = Text::create("", infoFont_);
    nodeCountText_->setPosition(x, y);
    nodeCountText_->setTextColor(Color(0.9f, 0.9f, 0.9f, 1.0f));
    addChild(nodeCountText_);
    y += lineHeight;

    strategyText_ = Text::create("", infoFont_);
    strategyText_->setPosition(x, y);
    strategyText_->setTextColor(Color(0.5f, 1.0f, 0.5f, 1.0f));
    addChild(strategyText_);
    y += lineHeight;

    collisionText_ = Text::create("", infoFont_);
    collisionText_->setPosition(x, y);
    collisionText_->setTextColor(Color(1.0f, 0.5f, 0.5f, 1.0f));
    addChild(collisionText_);
    y += lineHeight;

    updateTimeText_ = Text::create("", infoFont_);
    updateTimeText_->setPosition(x, y);
    updateTimeText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(updateTimeText_);
    y += lineHeight;

    collisionTimeText_ = Text::create("", infoFont_);
    collisionTimeText_->setPosition(x, y);
    collisionTimeText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(collisionTimeText_);
    y += lineHeight;

    renderTimeText_ = Text::create("", infoFont_);
    renderTimeText_->setPosition(x, y);
    renderTimeText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(renderTimeText_);
    y += lineHeight;

    fpsText_ = Text::create("", infoFont_);
    fpsText_->setPosition(x, y);
    fpsText_->setTextColor(Color(0.5f, 1.0f, 0.5f, 1.0f));
    addChild(fpsText_);
    y += lineHeight * 1.5f;

    // 创建操作说明文本
    helpTitleText_ = Text::create("操作说明:", infoFont_);
    helpTitleText_->setPosition(x, y);
    helpTitleText_->setTextColor(Color(1.0f, 1.0f, 0.5f, 1.0f));
    addChild(helpTitleText_);
    y += lineHeight;

    helpAddText_ = Text::create("A键 - 添加100个节点", infoFont_);
    helpAddText_->setPosition(x + 10, y);
    helpAddText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(helpAddText_);
    y += lineHeight;

    helpRemoveText_ = Text::create("B键 - 移除100个节点", infoFont_);
    helpRemoveText_->setPosition(x + 10, y);
    helpRemoveText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(helpRemoveText_);
    y += lineHeight;

    helpToggleText_ = Text::create("X键 - 切换索引策略", infoFont_);
    helpToggleText_->setPosition(x + 10, y);
    helpToggleText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(helpToggleText_);
    y += lineHeight;

    helpExitText_ = Text::create("+键 - 退出程序", infoFont_);
    helpExitText_->setPosition(x + 10, y);
    helpExitText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(helpExitText_);

    // 创建图例文本
    float legendX = screenWidth_ - 200.0f;
    float legendY = 20.0f;

    legendTitleText_ = Text::create("图例:", infoFont_);
    legendTitleText_->setPosition(legendX, legendY);
    legendTitleText_->setTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    addChild(legendTitleText_);
    legendY += 25.0f;

    legendNormalText_ = Text::create("- 正常", infoFont_);
    legendNormalText_->setPosition(legendX + 20.0f, legendY);
    legendNormalText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(legendNormalText_);
    legendY += 25.0f;

    legendCollidingText_ = Text::create("- 碰撞中", infoFont_);
    legendCollidingText_->setPosition(legendX + 20.0f, legendY);
    legendCollidingText_->setTextColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    addChild(legendCollidingText_);
  }

  /**
   * @brief 创建指定数量的节点
   */
  void createNodes(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posX(50.0f, screenWidth_ - 50.0f);
    std::uniform_real_distribution<float> posY(50.0f, screenHeight_ - 50.0f);
    std::uniform_real_distribution<float> colorR(0.2f, 0.9f);
    std::uniform_real_distribution<float> colorG(0.2f, 0.9f);
    std::uniform_real_distribution<float> colorB(0.2f, 0.9f);

    for (size_t i = 0; i < count; ++i) {
      Color color(colorR(gen), colorG(gen), colorB(gen), 0.7f);
      auto node = makePtr<PhysicsNode>(20.0f, color, static_cast<int>(i));
      node->setPosition(Vec2(posX(gen), posY(gen)));
      addChild(node);
      nodes_.push_back(node);
    }
  }

  /**
   * @brief 添加节点
   */
  void addNodes(size_t count) {
    size_t currentCount = nodes_.size();
    if (currentCount + count > 5000) {
      E2D_LOG_WARN("节点数量已达上限(5000)");
      return;
    }
    createNodes(count);
    E2D_LOG_INFO("添加 {} 个节点，当前总数: {}", count, nodes_.size());
  }

  /**
   * @brief 移除节点
   */
  void removeNodes(size_t count) {
    if (count >= nodes_.size()) {
      count = nodes_.size();
    }
    if (count == 0)
      return;

    for (size_t i = 0; i < count; ++i) {
      removeChild(nodes_.back());
      nodes_.pop_back();
    }
    E2D_LOG_INFO("移除 {} 个节点，当前总数: {}", count, nodes_.size());
  }

  /**
   * @brief 切换空间索引策略
   */
  void toggleSpatialStrategy() {
    auto &spatialManager = getSpatialManager();
    SpatialStrategy currentStrategy = spatialManager.getCurrentStrategy();

    if (currentStrategy == SpatialStrategy::QuadTree) {
      spatialManager.setStrategy(SpatialStrategy::SpatialHash);
      E2D_LOG_INFO("切换到空间哈希策略");
    } else {
      spatialManager.setStrategy(SpatialStrategy::QuadTree);
      E2D_LOG_INFO("切换到四叉树策略");
    }
  }

  /**
   * @brief 使用引擎自带的空间索引进行碰撞检测
   *
   * 关键方法：
   * - Scene::queryCollisions() - 查询场景中所有碰撞的节点对
   * - SpatialManager::queryCollisions() - 空间管理器的碰撞检测
   */
  void performCollisionDetection() {
    // 清除之前的碰撞状态
    for (auto &node : nodes_) {
      node->setColliding(false);
    }

    // 使用引擎自带的空间索引进行碰撞检测
    // 这是核心：Scene::queryCollisions() 会自动使用 SpatialManager
    auto collisions = queryCollisions();

    stats_.collisionCount = collisions.size();

    // 标记碰撞的节点
    for (const auto &[nodeA, nodeB] : collisions) {
      if (auto boxA = dynamic_cast<PhysicsNode *>(nodeA)) {
        boxA->setColliding(true);
      }
      if (auto boxB = dynamic_cast<PhysicsNode *>(nodeB)) {
        boxB->setColliding(true);
      }
    }
  }

  /**
   * @brief 更新UI文本内容
   */
  void updateUI() {
    if (!nodeCountText_)
      return;

    auto &app = Application::instance();

    // 使用 setFormat 格式化文本
    nodeCountText_->setFormat("节点数量: %zu", stats_.nodeCount);
    strategyText_->setFormat("索引策略: %s", stats_.strategyName);
    collisionText_->setFormat("碰撞对数: %zu", stats_.collisionCount);
    updateTimeText_->setFormat("更新时间: %.2f ms", stats_.updateTime);
    collisionTimeText_->setFormat("碰撞检测: %.2f ms", stats_.collisionTime);
    renderTimeText_->setFormat("渲染时间: %.2f ms", stats_.renderTime);
    fpsText_->setFormat("FPS: %u", app.fps());
  }

  /**
   * @brief 绘制图例方块
   */
  void drawLegend(RenderBackend &renderer) {
    float legendX = screenWidth_ - 200.0f;
    float legendY = 20.0f + 25.0f; // 在标题下方

    // 绘制正常状态方块
    renderer.fillRect(Rect(legendX, legendY, 15.0f, 15.0f),
                      Color(0.5f, 0.5f, 0.9f, 0.7f));
    legendY += 25.0f;

    // 绘制碰撞状态方块
    renderer.fillRect(Rect(legendX, legendY, 15.0f, 15.0f),
                      Color(1.0f, 0.2f, 0.2f, 0.9f));
  }

  std::vector<Ptr<PhysicsNode>> nodes_;
  PerformanceStats stats_;
  float screenWidth_ = 1280.0f;
  float screenHeight_ = 720.0f;

  Ptr<FontAtlas> titleFont_;
  Ptr<FontAtlas> infoFont_;

  // UI 文本组件
  Ptr<Text> titleText_;
  Ptr<Text> nodeCountText_;
  Ptr<Text> strategyText_;
  Ptr<Text> collisionText_;
  Ptr<Text> updateTimeText_;
  Ptr<Text> collisionTimeText_;
  Ptr<Text> renderTimeText_;
  Ptr<Text> fpsText_;
  Ptr<Text> helpTitleText_;
  Ptr<Text> helpAddText_;
  Ptr<Text> helpRemoveText_;
  Ptr<Text> helpToggleText_;
  Ptr<Text> helpExitText_;
  Ptr<Text> legendTitleText_;
  Ptr<Text> legendNormalText_;
  Ptr<Text> legendCollidingText_;
};

// ============================================================================
// 程序入口
// ============================================================================

int main(int argc, char **argv)
{
  Logger::init();
  Logger::setLevel(LogLevel::Debug);

  E2D_LOG_INFO("========================");
  E2D_LOG_INFO("Easy2D 引擎空间索引演示");
  E2D_LOG_INFO("========================");

  auto &app = Application::instance();

  AppConfig config;
  config.title = "Easy2D - 引擎空间索引演示";
  config.width = 1280;
  config.height = 720;
  config.vsync = true;
  config.fpsLimit = 60;

  if (!app.init(config)) {
    E2D_LOG_ERROR("应用初始化失败！");
    return -1;
  }

  app.enterScene(makePtr<SpatialIndexDemoScene>());

  E2D_LOG_INFO("开始主循环...");

  app.run();

  E2D_LOG_INFO("应用结束");

  return 0;
}
