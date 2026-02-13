// ============================================================================
// BaseScene.cpp - Flappy Bird 基础场景实现
// ============================================================================

#include "BaseScene.h"
#include <extra2d/utils/logger.h>

namespace flappybird {

BaseScene::BaseScene() {
  // 设置背景颜色为黑色（窗口四周会显示这个颜色）
  setBackgroundColor(extra2d::Color(0.0f, 0.0f, 0.0f, 1.0f));
}

void BaseScene::onEnter() {
  extra2d::Scene::onEnter();
  // 计算并更新视口
  updateViewport();
}

void BaseScene::updateViewport() {
  auto &app = extra2d::Application::instance();
  float windowWidth = static_cast<float>(app.window().getWidth());
  float windowHeight = static_cast<float>(app.window().getHeight());

  // 计算游戏内容在窗口中的居中位置
  // 保持游戏原始宽高比，进行"黑边"适配
  float scaleX = windowWidth / GAME_WIDTH;
  float scaleY = windowHeight / GAME_HEIGHT;
  // 使用较小的缩放比例，确保游戏内容完整显示在窗口中
  float scale = std::min(scaleX, scaleY);

  scaledGameWidth_ = GAME_WIDTH * scale;
  scaledGameHeight_ = GAME_HEIGHT * scale;
  // 计算居中偏移，使游戏内容在窗口中水平和垂直居中
  viewportOffsetX_ = (windowWidth - scaledGameWidth_) * 0.5f;
  viewportOffsetY_ = (windowHeight - scaledGameHeight_) * 0.5f;

  // 设置视口大小为游戏逻辑分辨率
  setViewportSize(GAME_WIDTH, GAME_HEIGHT);

  // 创建并设置相机
  auto camera = extra2d::makePtr<extra2d::Camera>();
  // 设置正交投影，覆盖整个游戏逻辑区域
  // 注意：对于2D游戏，Y轴向下增长，所以bottom > top
  camera->setViewport(0.0f, GAME_WIDTH, GAME_HEIGHT, 0.0f);
  setCamera(camera);
}

void BaseScene::onRender(extra2d::RenderBackend &renderer) {
  // 检查窗口大小是否改变，如果改变则更新视口
  auto &app = extra2d::Application::instance();
  float currentWindowWidth = static_cast<float>(app.window().getWidth());
  float currentWindowHeight = static_cast<float>(app.window().getHeight());

  // 如果窗口大小改变，重新计算视口
  float expectedWidth = scaledGameWidth_ + viewportOffsetX_ * 2.0f;
  float expectedHeight = scaledGameHeight_ + viewportOffsetY_ * 2.0f;
  if (std::abs(currentWindowWidth - expectedWidth) > 1.0f ||
      std::abs(currentWindowHeight - expectedHeight) > 1.0f) {
    E2D_LOG_INFO("BaseScene::onRender - window size changed from ({} x {}) to "
                 "({} x {}), updating viewport",
                 expectedWidth, expectedHeight, currentWindowWidth,
                 currentWindowHeight);
    updateViewport();
  }

  // 设置视口为居中区域
  E2D_LOG_INFO(
      "BaseScene::onRender - setting viewport: x={}, y={}, width={}, height={}",
      static_cast<int>(viewportOffsetX_), static_cast<int>(viewportOffsetY_),
      static_cast<int>(scaledGameWidth_), static_cast<int>(scaledGameHeight_));
  renderer.setViewport(
      static_cast<int>(viewportOffsetX_), static_cast<int>(viewportOffsetY_),
      static_cast<int>(scaledGameWidth_), static_cast<int>(scaledGameHeight_));

  // 调用父类的 onRender 进行实际渲染
  extra2d::Scene::onRender(renderer);
}

} // namespace flappybird
