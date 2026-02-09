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
    E2D_LOG_INFO("SpatialIndexDemoScene::onEnter - 引擎空间索引演示");

    auto &app = Application::instance();
    screenWidth_ = static_cast<float>(app.getConfig().width);
    screenHeight_ = static_cast<float>(app.getConfig().height);

    // 设置背景色
    setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));

    // 创建1000个碰撞节点
    createNodes(1000);

    // 加载字体
    loadFonts();

    E2D_LOG_INFO("创建了 {} 个碰撞节点", nodes_.size());
    E2D_LOG_INFO("空间索引已启用: {}", isSpatialIndexingEnabled());
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
    if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_START)) {
      E2D_LOG_INFO("退出应用");
      Application::instance().quit();
    }

    // 按A键添加节点
    if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_A)) {
      addNodes(100);
    }

    // 按B键减少节点
    if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_B)) {
      removeNodes(100);
    }

    // 按X键切换空间索引策略
    if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_X)) {
      toggleSpatialStrategy();
    }
  }

  void onRender(RenderBackend &renderer) override {
    Scene::onRender(renderer);

    auto renderStart = std::chrono::high_resolution_clock::now();

    // 节点渲染由Scene自动处理

    auto renderEnd = std::chrono::high_resolution_clock::now();
    stats_.renderTime =
        std::chrono::duration<double, std::milli>(renderEnd - renderStart)
            .count();

    // 绘制UI
    drawUI(renderer);
  }

private:
  /**
   * @brief 加载字体资源
   */
  void loadFonts() {
    auto &resources = Application::instance().resources();

    std::vector<std::string> fontPaths = {
        "romfs:/assets/msjh.ttf",
        "romfs:/assets/default.ttf",
        "romfs:/assets/font.ttf",
    };

    titleFont_ = resources.loadFontWithFallbacks(fontPaths, 28, true);
    infoFont_ = resources.loadFontWithFallbacks(fontPaths, 16, true);
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
   * @brief 绘制UI界面
   */
  void drawUI(RenderBackend &renderer) {
    if (!titleFont_ || !infoFont_)
      return;

    auto &app = Application::instance();

    // 绘制标题
    renderer.drawText(*titleFont_, "引擎空间索引演示", Vec2(30.0f, 20.0f),
                      Color(1.0f, 1.0f, 1.0f, 1.0f));

    // 绘制性能统计
    std::stringstream ss;
    float x = 30.0f;
    float y = 60.0f;
    float lineHeight = 22.0f;

    ss << "节点数量: " << stats_.nodeCount;
    renderer.drawText(*infoFont_, ss.str(), Vec2(x, y),
                      Color(0.9f, 0.9f, 0.9f, 1.0f));
    y += lineHeight;

    ss.str("");
    ss << "索引策略: " << stats_.strategyName;
    renderer.drawText(*infoFont_, ss.str(), Vec2(x, y),
                      Color(0.5f, 1.0f, 0.5f, 1.0f));
    y += lineHeight;

    ss.str("");
    ss << "碰撞对数: " << stats_.collisionCount;
    renderer.drawText(*infoFont_, ss.str(), Vec2(x, y),
                      Color(1.0f, 0.5f, 0.5f, 1.0f));
    y += lineHeight;

    ss.str("");
    ss << std::fixed << std::setprecision(2);
    ss << "更新时间: " << stats_.updateTime << " ms";
    renderer.drawText(*infoFont_, ss.str(), Vec2(x, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    y += lineHeight;

    ss.str("");
    ss << "碰撞检测: " << stats_.collisionTime << " ms";
    renderer.drawText(*infoFont_, ss.str(), Vec2(x, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    y += lineHeight;

    ss.str("");
    ss << "渲染时间: " << stats_.renderTime << " ms";
    renderer.drawText(*infoFont_, ss.str(), Vec2(x, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    y += lineHeight;

    ss.str("");
    ss << "FPS: " << app.fps();
    renderer.drawText(*infoFont_, ss.str(), Vec2(x, y),
                      Color(0.5f, 1.0f, 0.5f, 1.0f));
    y += lineHeight * 1.5f;

    // 绘制操作说明
    renderer.drawText(*infoFont_, "操作说明:", Vec2(x, y),
                      Color(1.0f, 1.0f, 0.5f, 1.0f));
    y += lineHeight;
    renderer.drawText(*infoFont_, "A键 - 添加100个节点", Vec2(x + 10, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    y += lineHeight;
    renderer.drawText(*infoFont_, "B键 - 移除100个节点", Vec2(x + 10, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    y += lineHeight;
    renderer.drawText(*infoFont_, "X键 - 切换索引策略", Vec2(x + 10, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    y += lineHeight;
    renderer.drawText(*infoFont_, "+键 - 退出程序", Vec2(x + 10, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));

    // 绘制图例
    float legendX = screenWidth_ - 200.0f;
    float legendY = 20.0f;
    renderer.drawText(*infoFont_, "图例:", Vec2(legendX, legendY),
                      Color(1.0f, 1.0f, 1.0f, 1.0f));
    legendY += 25.0f;

    renderer.fillRect(Rect(legendX, legendY, 15.0f, 15.0f),
                      Color(0.5f, 0.5f, 0.9f, 0.7f));
    renderer.drawText(*infoFont_, "- 正常", Vec2(legendX + 20.0f, legendY),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    legendY += 25.0f;

    renderer.fillRect(Rect(legendX, legendY, 15.0f, 15.0f),
                      Color(1.0f, 0.2f, 0.2f, 0.9f));
    renderer.drawText(*infoFont_, "- 碰撞中", Vec2(legendX + 20.0f, legendY),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
  }

  std::vector<Ptr<PhysicsNode>> nodes_;
  PerformanceStats stats_;
  float screenWidth_ = 1280.0f;
  float screenHeight_ = 720.0f;

  Ptr<FontAtlas> titleFont_;
  Ptr<FontAtlas> infoFont_;
};

// ============================================================================
// 程序入口
// ============================================================================

extern "C" int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

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
