// ============================================================================
// SplashScene.cpp - 启动场景实现
// ============================================================================

#include "SplashScene.h"
#include "ResLoader.h"
#include "StartScene.h"
#include <extra2d/utils/logger.h>

namespace flappybird {

SplashScene::SplashScene() {
  // 基类 BaseScene 已经处理了视口设置和背景颜色
}

void SplashScene::onEnter() {
  BaseScene::onEnter();

  // 尝试加载 splash 图片
  auto splashFrame = ResLoader::getKeyFrame("splash");
  if (splashFrame) {
    auto splash = extra2d::Sprite::create(splashFrame->getTexture(),
                                          splashFrame->getRect());
    splash->setAnchor(0.5f, 0.5f);
    // splash 图片是全屏的(288x512)，将其中心放在游戏区域中心
    splash->setPosition(GAME_WIDTH / 2.0f, GAME_HEIGHT / 2.0f);
    addChild(splash);
  }
  // 播放转场音效
  ResLoader::playMusic(MusicType::Swoosh);
}

void SplashScene::onUpdate(float dt) {
  BaseScene::onUpdate(dt);

  // 计时
  timer_ += dt;
  if (timer_ >= delay_) {
    gotoStartScene();
  }
}

void SplashScene::gotoStartScene() {
  auto &app = extra2d::Application::instance();
  app.scenes().replaceScene(extra2d::makePtr<StartScene>(),
                            extra2d::TransitionType::Fade, 0.5f);
}

} // namespace flappybird
