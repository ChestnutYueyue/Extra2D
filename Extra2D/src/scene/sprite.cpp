#include <algorithm>
#include <cmath>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_command.h>
#include <extra2d/graphics/texture.h>
#include <extra2d/scene/sprite.h>

namespace extra2d {

Sprite::Sprite() = default;

Sprite::Sprite(Ptr<Texture> texture) { setTexture(texture); }

void Sprite::setTexture(Ptr<Texture> texture) {
  texture_ = texture;
  if (texture_) {
    textureRect_ = Rect(0, 0, static_cast<float>(texture_->getWidth()),
                        static_cast<float>(texture_->getHeight()));
  }
}

void Sprite::setTextureRect(const Rect &rect) {
  textureRect_ = rect;
}

void Sprite::setColor(const Color &color) { color_ = color; }

void Sprite::setFlipX(bool flip) { flipX_ = flip; }

void Sprite::setFlipY(bool flip) { flipY_ = flip; }

Ptr<Sprite> Sprite::create() { return makePtr<Sprite>(); }

Ptr<Sprite> Sprite::create(Ptr<Texture> texture) {
  return makePtr<Sprite>(texture);
}

Ptr<Sprite> Sprite::create(Ptr<Texture> texture, const Rect &rect) {
  auto sprite = makePtr<Sprite>(texture);
  sprite->setTextureRect(rect);
  return sprite;
}

Rect Sprite::getBoundingBox() const {
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
