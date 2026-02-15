/**
 * @file main.cpp
 * @brief Extra2D 场景图测试示例
 *
 * 演示场景图功能：
 * - 节点层级关系
 * - 变换（位置、旋转、缩放）
 * - 形状节点渲染
 * - 输入事件处理
 */

#include <extra2d/extra2d.h>
#include <iostream>

using namespace extra2d;

/**
 * @brief 创建场景图测试
 */
void createSceneGraph(Scene *scene) {
  float width = scene->getWidth();
  float height = scene->getHeight();

  auto root = makeShared<Node>();
  root->setName("Root");
  root->setPos(width / 2, height / 2);
  scene->addChild(root);

  auto parent1 = makeShared<Node>();
  parent1->setName("Parent1");
  parent1->setPos(-200, 0);
  root->addChild(parent1);

  auto rect1 = ShapeNode::createFilledRect(Rect(-50, -50, 100, 100),
                                           Color(1.0f, 0.4f, 0.4f, 1.0f));
  rect1->setName("RedRect");
  parent1->addChild(rect1);

  auto child1 = makeShared<Node>();
  child1->setName("Child1");
  child1->setPos(80, 0);
  child1->setRotation(45);
  child1->setScale(0.5f);
  parent1->addChild(child1);

  auto smallRect = ShapeNode::createFilledRect(Rect(-30, -30, 60, 60),
                                               Color(1.0f, 0.8f, 0.4f, 1.0f));
  smallRect->setName("OrangeRect");
  child1->addChild(smallRect);

  auto parent2 = makeShared<Node>();
  parent2->setName("Parent2");
  parent2->setPos(200, 0);
  root->addChild(parent2);

  auto circle1 = ShapeNode::createFilledCircle(Vec2(0, 0), 60,
                                               Color(0.4f, 0.4f, 1.0f, 1.0f));
  circle1->setName("BlueCircle");
  parent2->addChild(circle1);

  auto child2 = makeShared<Node>();
  child2->setName("Child2");
  child2->setPos(0, 100);
  parent2->addChild(child2);

  auto triangle = ShapeNode::createFilledTriangle(
      Vec2(0, -40), Vec2(-35, 30), Vec2(35, 30), Color(0.4f, 1.0f, 0.4f, 1.0f));
  triangle->setName("GreenTriangle");
  child2->addChild(triangle);

  auto line = ShapeNode::createLine(Vec2(-300, -200), Vec2(300, -200),
                                    Color(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);
  line->setName("BottomLine");
  root->addChild(line);

  auto polygon = ShapeNode::createFilledPolygon(
      {Vec2(0, -50), Vec2(50, 0), Vec2(30, 50), Vec2(-30, 50), Vec2(-50, 0)},
      Color(1.0f, 0.4f, 1.0f, 1.0f));
  polygon->setName("PurplePolygon");
  polygon->setPos(0, -150);
  root->addChild(polygon);

  std::cout << "\n=== Scene Graph Structure ===" << std::endl;
  std::cout << "Scene (root)" << std::endl;
  std::cout << "  └── Root (center)" << std::endl;
  std::cout << "      ├── Parent1 (left)" << std::endl;
  std::cout << "      │   ├── RedRect (100x100)" << std::endl;
  std::cout << "      │   └── Child1 (rotated 45°, scaled 0.5)" << std::endl;
  std::cout << "      │       └── OrangeRect (60x60)" << std::endl;
  std::cout << "      ├── Parent2 (right)" << std::endl;
  std::cout << "      │   ├── BlueCircle (radius 60)" << std::endl;
  std::cout << "      │   └── Child2 (below)" << std::endl;
  std::cout << "      │       └── GreenTriangle" << std::endl;
  std::cout << "      ├── BottomLine" << std::endl;
  std::cout << "      └── PurplePolygon (pentagon)" << std::endl;
  std::cout << "=============================\n" << std::endl;
}

/**
 * @brief 主函数
 */
int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  std::cout << "Extra2D Scene Graph Demo - Starting..." << std::endl;

  AppConfig config = AppConfig::createDefault();
  config.appName = "Extra2D Scene Graph Demo";
  config.appVersion = "1.0.0";

  Application &app = Application::get();

  if (!app.init(config)) {
    std::cerr << "Failed to initialize application!" << std::endl;
    return -1;
  }

  std::cout << "Application initialized successfully!" << std::endl;
  std::cout << "Window: " << app.window().width() << "x"
            << app.window().height() << std::endl;

  auto eventService = app.events();
  if (eventService) {
    eventService->addListener(EventType::KeyPressed, [](Event &e) {
      auto &keyEvent = std::get<KeyEvent>(e.data);

      if (keyEvent.keyCode == static_cast<int>(Key::Escape)) {
        e.handled = true;
        Application::get().quit();
      }
    });

    eventService->addListener(EventType::MouseButtonPressed, [](Event &e) {
      auto &mouseEvent = std::get<MouseButtonEvent>(e.data);
      std::cout << "[Click] Button " << mouseEvent.button << " at ("
                << mouseEvent.position.x << ", " << mouseEvent.position.y << ")"
                << std::endl;
    });
  }

  auto scene = Scene::create();
  scene->setBackgroundColor(Color(0.12f, 0.12f, 0.16f, 1.0f));
  scene->setViewportSize(static_cast<float>(app.window().width()),
                         static_cast<float>(app.window().height()));

  auto cameraService = app.camera();
  if (cameraService) {
    ViewportConfig vpConfig;
    vpConfig.logicWidth = static_cast<float>(app.window().width());
    vpConfig.logicHeight = static_cast<float>(app.window().height());
    vpConfig.mode = ViewportMode::AspectRatio;
    cameraService->setViewportConfig(vpConfig);
    cameraService->updateViewport(app.window().width(), app.window().height());
    cameraService->applyViewportAdapter();
  }

  createSceneGraph(scene.get());

  app.enterScene(scene);

  std::cout << "\nControls:" << std::endl;
  std::cout << "  ESC - Exit" << std::endl;
  std::cout << "  Mouse Click - Print position" << std::endl;
  std::cout << "\nRunning main loop...\n" << std::endl;

  app.run();

  std::cout << "Shutting down..." << std::endl;
  app.shutdown();

  std::cout << "Goodbye!" << std::endl;
  return 0;
}
