#pragma once

#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <glm/mat4x4.hpp>

namespace extra2d {

// ============================================================================
// 视口适配模式枚举
// ============================================================================
enum class ViewportMode {
  AspectRatio,
  Stretch,
  Center,
  Custom
};

// ============================================================================
// 黑边位置枚举
// ============================================================================
enum class LetterboxPosition {
  Center,
  LeftTop,
  RightTop,
  LeftBottom,
  RightBottom
};

// ============================================================================
// 视口配置结构体
// ============================================================================
struct ViewportConfig {
  float logicWidth = 1920.0f;
  float logicHeight = 1080.0f;
  ViewportMode mode = ViewportMode::AspectRatio;
  LetterboxPosition letterboxPosition = LetterboxPosition::Center;
  Color letterboxColor = Colors::Black;
  bool autoScaleInCenterMode = true;
  float customScale = 1.0f;
  Vec2 customOffset = Vec2::Zero();
  Rect customViewport = Rect::Zero();
};

// ============================================================================
// 视口计算结果结构体
// ============================================================================
struct ViewportResult {
  Rect viewport;
  float scaleX = 1.0f;
  float scaleY = 1.0f;
  float uniformScale = 1.0f;
  Vec2 offset;
  bool hasLetterbox = false;

  struct Letterbox {
    Rect top;
    Rect bottom;
    Rect left;
    Rect right;
  } letterbox;
};

// ============================================================================
// 视口适配器类
// ============================================================================
class ViewportAdapter {
public:
  ViewportAdapter();
  ViewportAdapter(float logicWidth, float logicHeight);
  ~ViewportAdapter() = default;

  // ------------------------------------------------------------------------
  // 配置设置
  // ------------------------------------------------------------------------
  
  /**
   * @brief 设置视口配置
   * @param config 视口配置结构体
   */
  void setConfig(const ViewportConfig &config);

  /**
   * @brief 获取当前视口配置
   * @return 当前视口配置
   */
  const ViewportConfig &getConfig() const { return config_; }

  /**
   * @brief 设置逻辑分辨率
   * @param width 逻辑宽度
   * @param height 逻辑高度
   */
  void setLogicSize(float width, float height);

  /**
   * @brief 设置视口适配模式
   * @param mode 适配模式
   */
  void setMode(ViewportMode mode);

  /**
   * @brief 设置黑边位置
   * @param position 黑边位置
   */
  void setLetterboxPosition(LetterboxPosition position);

  /**
   * @brief 设置黑边颜色
   * @param color 黑边颜色
   */
  void setLetterboxColor(const Color &color);

  // ------------------------------------------------------------------------
  // 更新和计算
  // ------------------------------------------------------------------------

  /**
   * @brief 更新视口适配计算
   * @param screenWidth 屏幕宽度
   * @param screenHeight 屏幕高度
   */
  void update(int screenWidth, int screenHeight);

  /**
   * @brief 获取计算结果
   * @return 视口计算结果
   */
  const ViewportResult &getResult() const { return result_; }

  // ------------------------------------------------------------------------
  // 坐标转换
  // ------------------------------------------------------------------------

  /**
   * @brief 屏幕坐标转逻辑坐标
   * @param screenPos 屏幕坐标
   * @return 逻辑坐标
   */
  Vec2 screenToLogic(const Vec2 &screenPos) const;

  /**
   * @brief 逻辑坐标转屏幕坐标
   * @param logicPos 逻辑坐标
   * @return 屏幕坐标
   */
  Vec2 logicToScreen(const Vec2 &logicPos) const;

  /**
   * @brief 屏幕坐标转逻辑坐标（分量形式）
   * @param x 屏幕X坐标
   * @param y 屏幕Y坐标
   * @return 逻辑坐标
   */
  Vec2 screenToLogic(float x, float y) const;

  /**
   * @brief 逻辑坐标转屏幕坐标（分量形式）
   * @param x 逻辑X坐标
   * @param y 逻辑Y坐标
   * @return 屏幕坐标
   */
  Vec2 logicToScreen(float x, float y) const;

  // ------------------------------------------------------------------------
  // 矩阵获取
  // ------------------------------------------------------------------------

  /**
   * @brief 获取视口变换矩阵
   * @return 视口变换矩阵（从逻辑坐标到屏幕坐标）
   */
  glm::mat4 getMatrix() const;

  /**
   * @brief 获取反向视口变换矩阵
   * @return 反向视口变换矩阵（从屏幕坐标到逻辑坐标）
   */
  glm::mat4 getInvMatrix() const;

  // ------------------------------------------------------------------------
  // 区域检测
  // ------------------------------------------------------------------------

  /**
   * @brief 检查屏幕坐标是否在视口内
   * @param screenPos 屏幕坐标
   * @return 如果在视口内返回 true
   */
  bool isInViewport(const Vec2 &screenPos) const;

  /**
   * @brief 检查屏幕坐标是否在黑边区域
   * @param screenPos 屏幕坐标
   * @return 如果在黑边区域返回 true
   */
  bool isInLetterbox(const Vec2 &screenPos) const;

  // ------------------------------------------------------------------------
  // Getter 方法
  // ------------------------------------------------------------------------

  /**
   * @brief 获取逻辑宽度
   * @return 逻辑宽度
   */
  float getLogicWidth() const { return config_.logicWidth; }

  /**
   * @brief 获取逻辑高度
   * @return 逻辑高度
   */
  float getLogicHeight() const { return config_.logicHeight; }

  /**
   * @brief 获取逻辑尺寸
   * @return 逻辑尺寸
   */
  Size getLogicSize() const {
    return Size(config_.logicWidth, config_.logicHeight);
  }

  /**
   * @brief 获取屏幕宽度
   * @return 屏幕宽度
   */
  int getScreenWidth() const { return screenWidth_; }

  /**
   * @brief 获取屏幕高度
   * @return 屏幕高度
   */
  int getScreenHeight() const { return screenHeight_; }

  /**
   * @brief 获取屏幕尺寸
   * @return 屏幕尺寸
   */
  Size getScreenSize() const {
    return Size(static_cast<float>(screenWidth_),
                static_cast<float>(screenHeight_));
  }

  /**
   * @brief 获取X方向缩放比例
   * @return X方向缩放比例
   */
  float getScaleX() const { return result_.scaleX; }

  /**
   * @brief 获取Y方向缩放比例
   * @return Y方向缩放比例
   */
  float getScaleY() const { return result_.scaleY; }

  /**
   * @brief 获取统一缩放比例
   * @return 统一缩放比例
   */
  float getUniformScale() const { return result_.uniformScale; }

  /**
   * @brief 获取视口偏移
   * @return 视口偏移
   */
  Vec2 getOffset() const { return result_.offset; }

  /**
   * @brief 获取视口矩形
   * @return 视口矩形
   */
  Rect getViewport() const { return result_.viewport; }

  /**
   * @brief 检查是否有黑边
   * @return 如果有黑边返回 true
   */
  bool hasLetterbox() const { return result_.hasLetterbox; }

  /**
   * @brief 获取黑边信息
   * @return 黑边信息结构体
   */
  const ViewportResult::Letterbox &getLetterbox() const {
    return result_.letterbox;
  }

private:
  /**
   * @brief 计算宽高比适配模式
   */
  void calculateAspectRatio();

  /**
   * @brief 计算拉伸适配模式
   */
  void calculateStretch();

  /**
   * @brief 计算居中适配模式
   */
  void calculateCenter();

  /**
   * @brief 计算自定义适配模式
   */
  void calculateCustom();

  /**
   * @brief 计算黑边区域
   */
  void calculateLetterbox();

  /**
   * @brief 根据黑边位置调整偏移
   * @param extraWidth 额外宽度
   * @param extraHeight 额外高度
   */
  void applyLetterboxPosition(float extraWidth, float extraHeight);

  ViewportConfig config_;
  ViewportResult result_;
  int screenWidth_ = 0;
  int screenHeight_ = 0;

  mutable glm::mat4 viewportMatrix_;
  mutable glm::mat4 inverseViewportMatrix_;
  mutable bool matrixDirty_ = true;
};

} // namespace extra2d
