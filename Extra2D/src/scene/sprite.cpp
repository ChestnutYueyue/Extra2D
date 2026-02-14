#include <algorithm>
#include <cmath>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_command.h>
#include <extra2d/graphics/texture.h>
#include <extra2d/scene/sprite.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 创建一个空的精灵对象
 */
Sprite::Sprite() = default;

/**
 * @brief 带纹理的构造函数
 * @param texture 精灵使用的纹理智能指针
 *
 * 创建精灵并设置纹理，纹理区域默认为整个纹理
 */
Sprite::Sprite(Ptr<Texture> texture) { setTexture(texture); }

/**
 * @brief 设置精灵纹理
 * @param texture 要设置的纹理智能指针
 *
 * 设置纹理并将纹理区域初始化为整个纹理大小
 */
void Sprite::setTexture(Ptr<Texture> texture) {
  texture_ = texture;
  if (texture_) {
    textureRect_ = Rect(0, 0, static_cast<float>(texture_->getWidth()),
                        static_cast<float>(texture_->getHeight()));
  }
}

/**
 * @brief 设置纹理区域
 * @param rect 纹理上的矩形区域
 *
 * 设置精灵显示纹理的哪一部分
 */
void Sprite::setTextureRect(const Rect &rect) { textureRect_ = rect; }

/**
 * @brief 设置精灵颜色
 * @param color 要设置的颜色
 *
 * 颜色会与纹理颜色混合
 */
void Sprite::setColor(const Color &color) { color_ = color; }

/**
 * @brief 设置水平翻转
 * @param flip 是否水平翻转
 */
void Sprite::setFlipX(bool flip) { flipX_ = flip; }

/**
 * @brief 设置垂直翻转
 * @param flip 是否垂直翻转
 */
void Sprite::setFlipY(bool flip) { flipY_ = flip; }

/**
 * @brief 创建空精灵
 * @return 新创建的精灵智能指针
 */
Ptr<Sprite> Sprite::create() { return makePtr<Sprite>(); }

/**
 * @brief 创建带纹理的精灵
 * @param texture 精灵使用的纹理
 * @return 新创建的精灵智能指针
 */
Ptr<Sprite> Sprite::create(Ptr<Texture> texture) {
  return makePtr<Sprite>(texture);
}

/**
 * @brief 创建带纹理和纹理区域的精灵
 * @param texture 精灵使用的纹理
 * @param rect 纹理区域
 * @return 新创建的精灵智能指针
 */
Ptr<Sprite> Sprite::create(Ptr<Texture> texture, const Rect &rect) {
  auto sprite = makePtr<Sprite>(texture);
  sprite->setTextureRect(rect);
  return sprite;
}

/**
 * @brief 获取精灵的边界矩形
 * @return 精灵在世界坐标系中的轴对齐边界矩形
 *
 * 考虑位置、锚点、缩放等因素计算边界框
 */
Rect Sprite::getBounds() const {
  if (!texture_ || !texture_->isValid()) {
    return Rect();
  }

  float width = textureRect_.width();
  float height = textureRect_.height();

  auto pos = getPosition();
  auto anchor = getAnchor();
  auto scale = getScale();

  float w = width * scale.x;
  float h = height * scale.y;
  float x0 = pos.x - width * anchor.x * scale.x;
  float y0 = pos.y - height * anchor.y * scale.y;
  float x1 = x0 + w;
  float y1 = y0 + h;

  float l = std::min(x0, x1);
  float t = std::min(y0, y1);
  return Rect(l, t, std::abs(w), std::abs(h));
}

/**
 * @brief 绘制精灵
 * @param renderer 渲染后端引用
 *
 * 使用世界变换计算最终位置、缩放和旋转，然后绘制精灵
 */
void Sprite::onDraw(RenderBackend &renderer) {
  if (!texture_ || !texture_->isValid()) {
    return;
  }

  // Calculate destination rectangle based on texture rect
  float width = textureRect_.width();
  float height = textureRect_.height();

  // 使用世界变换来获取最终的位置
  auto worldTransform = getWorldTransform();

  // 从世界变换矩阵中提取位置（第四列）
  float worldX = worldTransform[3][0];
  float worldY = worldTransform[3][1];

  // 从世界变换矩阵中提取缩放
  float worldScaleX =
      glm::length(glm::vec2(worldTransform[0][0], worldTransform[0][1]));
  float worldScaleY =
      glm::length(glm::vec2(worldTransform[1][0], worldTransform[1][1]));

  auto anchor = getAnchor();

  // 锚点由 RenderBackend 在绘制时处理，这里只传递位置和尺寸
  Rect destRect(worldX, worldY, width * worldScaleX, height * worldScaleY);

  // Adjust source rect for flipping
  Rect srcRect = textureRect_;
  if (flipX_) {
    srcRect.origin.x = srcRect.right();
    srcRect.size.width = -srcRect.size.width;
  }
  if (flipY_) {
    srcRect.origin.y = srcRect.bottom();
    srcRect.size.height = -srcRect.size.height;
  }

  // 从世界变换矩阵中提取旋转角度
  float worldRotation = std::atan2(worldTransform[0][1], worldTransform[0][0]);

  renderer.drawSprite(*texture_, destRect, srcRect, color_, worldRotation,
                      anchor);
}

/**
 * @brief 生成渲染命令
 * @param commands 渲染命令输出向量
 * @param zOrder 渲染层级
 *
 * 根据精灵的纹理、变换和颜色生成精灵渲染命令
 */
void Sprite::generateRenderCommand(std::vector<RenderCommand> &commands,
                                   int zOrder) {
  if (!texture_ || !texture_->isValid()) {
    return;
  }

  // 计算目标矩形（与 onDraw 一致，使用世界变换）
  float width = textureRect_.width();
  float height = textureRect_.height();

  // 使用世界变换来获取最终的位置
  auto worldTransform = getWorldTransform();

  // 从世界变换矩阵中提取位置（第四列）
  float worldX = worldTransform[3][0];
  float worldY = worldTransform[3][1];

  // 从世界变换矩阵中提取缩放
  float worldScaleX =
      glm::length(glm::vec2(worldTransform[0][0], worldTransform[0][1]));
  float worldScaleY =
      glm::length(glm::vec2(worldTransform[1][0], worldTransform[1][1]));

  auto anchor = getAnchor();

  // 锚点由 RenderBackend 在绘制时处理，这里只传递位置和尺寸
  Rect destRect(worldX, worldY, width * worldScaleX, height * worldScaleY);

  // 调整源矩形（翻转）
  Rect srcRect = textureRect_;
  if (flipX_) {
    srcRect.origin.x = srcRect.right();
    srcRect.size.width = -srcRect.size.width;
  }
  if (flipY_) {
    srcRect.origin.y = srcRect.bottom();
    srcRect.size.height = -srcRect.size.height;
  }

  // 从世界变换矩阵中提取旋转角度
  float worldRotation = std::atan2(worldTransform[0][1], worldTransform[0][0]);

  // 创建渲染命令
  RenderCommand cmd;
  cmd.type = RenderCommandType::Sprite;
  cmd.layer = zOrder;
  cmd.data = SpriteCommandData{texture_.get(), destRect, srcRect, color_,
                               worldRotation,  anchor,   0};

  commands.push_back(std::move(cmd));
}

} // namespace extra2d
