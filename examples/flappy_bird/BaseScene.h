// ============================================================================
// BaseScene.h - Flappy Bird 基础场景类
// 描述: 提供统一的居中视口适配功能，所有游戏场景都应继承此类
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>

namespace flappybird {

// 游戏逻辑分辨率（原始 Flappy Bird 尺寸）
static constexpr float GAME_WIDTH = 288.0f;
static constexpr float GAME_HEIGHT = 512.0f;

/**
 * @brief Flappy Bird 基础场景类
 * 所有游戏场景都应继承此类，以获得统一的居中视口适配功能
 */
class BaseScene : public extra2d::Scene {
public:
  /**
   * @brief 构造函数
   */
  BaseScene();

  /**
   * @brief 场景进入时调用
   */
  void onEnter() override;

  /**
   * @brief 渲染时调用，设置居中视口
   * @param renderer 渲染后端
   */
  void onRender(extra2d::RenderBackend &renderer) override;

  /**
   * @brief 渲染场景内容，确保视口正确设置
   * @param renderer 渲染后端
   */
  void renderContent(extra2d::RenderBackend &renderer) override;

protected:
  /**
   * @brief 更新视口计算，使游戏内容在窗口中居中显示
   */
  void updateViewport();

  // 视口适配参数（用于在窗口中居中显示游戏内容）
  float scaledGameWidth_ = 0.0f;  // 缩放后的游戏宽度
  float scaledGameHeight_ = 0.0f; // 缩放后的游戏高度
  float viewportOffsetX_ = 0.0f;  // 视口水平偏移
  float viewportOffsetY_ = 0.0f;  // 视口垂直偏移
};

} // namespace flappybird
