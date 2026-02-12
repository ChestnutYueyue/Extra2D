// ============================================================================
// Ground.cpp - 地面类实现
// ============================================================================

#include "Ground.h"
#include "ResLoader.h"

namespace flappybird {

Ground::Ground() {
    moving_ = true;

    auto& app = extra2d::Application::instance();
    float screenHeight = static_cast<float>(app.getConfig().height);

    // 获取地面纹理帧
    auto landFrame = ResLoader::getKeyFrame("land");
    if (!landFrame) return;

    // 获取地面纹理和矩形
    auto texture = landFrame->getTexture();
    auto rect = landFrame->getRect();
    float groundWidth = rect.size.width;
    float groundHeight = rect.size.height;

    // 创建第一块地面
    ground1_ = extra2d::Sprite::create(texture, rect);
    ground1_->setAnchor(extra2d::Vec2(0.0f, 1.0f));  // 锚点设在左下角
    ground1_->setPosition(extra2d::Vec2(0.0f, screenHeight));
    addChild(ground1_);

    // 创建第二块地面，紧挨在第一块右边
    ground2_ = extra2d::Sprite::create(texture, rect);
    ground2_->setAnchor(extra2d::Vec2(0.0f, 1.0f));
    ground2_->setPosition(extra2d::Vec2(groundWidth - 1.0f, screenHeight));
    addChild(ground2_);
}

void Ground::onUpdate(float dt) {
    extra2d::Node::onUpdate(dt);

    if (!moving_) return;
    if (!ground1_ || !ground2_) return;

    // 获取地面宽度（从纹理矩形获取）
    float groundWidth = ground1_->getTextureRect().size.width;

    // 移动两块地面
    extra2d::Vec2 pos1 = ground1_->getPosition();
    extra2d::Vec2 pos2 = ground2_->getPosition();

    pos1.x -= speed * dt;
    pos2.x -= speed * dt;

    // 当地面完全移出屏幕左侧时，重置到右侧
    if (pos1.x <= -groundWidth) {
        pos1.x = pos2.x + groundWidth - 1.0f;
    }
    if (pos2.x <= -groundWidth) {
        pos2.x = pos1.x + groundWidth - 1.0f;
    }

    ground1_->setPosition(pos1);
    ground2_->setPosition(pos2);
}

void Ground::stop() {
    moving_ = false;
}

float Ground::getHeight() const {
    auto landFrame = ResLoader::getKeyFrame("land");
    return landFrame ? landFrame->getRect().size.height : 112.0f;
}

} // namespace flappybird
