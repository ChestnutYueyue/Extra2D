// ============================================================================
// StartScene.cpp - 开始菜单场景实现
// ============================================================================

#include "StartScene.h"
#include "Bird.h"
#include "GameScene.h"
#include "Ground.h"
#include "ResLoader.h"
#include "extra2d/event/input_codes.h"

namespace flappybird {

StartScene::StartScene() {
  auto &app = extra2d::Application::instance();
  auto &config = app.getConfig();
  setViewportSize(static_cast<float>(config.width),
                  static_cast<float>(config.height));
}

void StartScene::onEnter() {
  extra2d::Scene::onEnter();

  // 设置背景颜色为黑色（防止透明）
  setBackgroundColor(extra2d::Color(0.0f, 0.0f, 0.0f, 1.0f));

  auto &app = extra2d::Application::instance();
  float screenWidth = static_cast<float>(app.getConfig().width);
  float screenHeight = static_cast<float>(app.getConfig().height);

  // 添加背景（使用左上角锚点）
  auto bgFrame = ResLoader::getKeyFrame("bg_day");
  if (bgFrame) {
    auto background =
        extra2d::Sprite::create(bgFrame->getTexture(), bgFrame->getRect());
    background->setAnchor(0.0f, 0.0f);
    background->setPosition(0.0f, 0.0f);
    addChild(background);
    E2D_LOG_INFO("背景已添加: size={} x {}", bgFrame->getRect().size.width,
                 bgFrame->getRect().size.height);
  } else {
    E2D_LOG_ERROR("无法加载背景图片");
  }

  // 添加地面
  auto ground = extra2d::makePtr<Ground>();
  addChild(ground);

  // 添加标题图片（在上方）
  auto titleFrame = ResLoader::getKeyFrame("title");
  if (titleFrame) {
    auto title = extra2d::Sprite::create(titleFrame->getTexture(),
                                         titleFrame->getRect());
    title->setAnchor(0.5f, 0.5f);
    // 标题在屏幕上方
    title->setPosition(screenWidth / 2.0f, 150.0f);
    addChild(title);
    E2D_LOG_INFO("标题已添加: size={} x {}", titleFrame->getRect().size.width,
                 titleFrame->getRect().size.height);
  } else {
    E2D_LOG_ERROR("无法加载标题图片");
  }

  // 添加小鸟（在标题下方）
  auto bird = extra2d::makePtr<Bird>();
  bird->setAnchor(0.5f, 0.5f);
  bird->setPosition(screenWidth / 2.0f, screenHeight / 2.0f);
  bird->setStatus(Bird::Status::Idle);
  addChild(bird);

  // 添加开始按钮 - 在小鸟下方
  auto playFrame = ResLoader::getKeyFrame("button_play");
  if (playFrame) {
    float btnWidth = playFrame->getRect().size.width;
    float btnHeight = playFrame->getRect().size.height;

    playBtn_ = extra2d::Button::create();
    playBtn_->setBackgroundImage(playFrame->getTexture(), playFrame->getRect());
    // 使用世界坐标，中心锚点
    playBtn_->setAnchor(0.5f, 0.5f);
    // PLAY 按钮在小鸟下方
    playBtn_->setPosition(screenWidth / 2.0f,
                          screenHeight - playBtn_->getSize().height - 100.0f);
    playBtn_->setOnClick([this]() {
      ResLoader::playMusic(MusicType::Click);
      startGame();
    });
    addChild(playBtn_);
  }

  // 添加分享按钮 - 在 PLAY 按钮下方，靠近地面
  auto shareFrame = ResLoader::getKeyFrame("button_share");
  if (shareFrame) {
    float btnWidth = shareFrame->getRect().size.width;
    float btnHeight = shareFrame->getRect().size.height;

    shareBtn_ = extra2d::Button::create();
    shareBtn_->setBackgroundImage(shareFrame->getTexture(),
                                  shareFrame->getRect());
    // 使用世界坐标，中心锚点
    shareBtn_->setAnchor(0.5f, 0.5f);
    // SHARE 按钮在 PLAY 按钮下方，靠近地面
    shareBtn_->setPosition(screenWidth / 2.0f,
                           screenHeight - shareBtn_->getSize().height - 80.0f);
    shareBtn_->setOnClick([this]() {
      ResLoader::playMusic(MusicType::Click);
      // 分享功能暂不实现
    });
    addChild(shareBtn_);
  }

  // 添加 copyright 图片（在底部）
  auto copyrightFrame = ResLoader::getKeyFrame("brand_copyright");
  if (copyrightFrame) {
    auto copyright = extra2d::Sprite::create(copyrightFrame->getTexture(),
                                             copyrightFrame->getRect());
    copyright->setAnchor(0.5f, 0.5f);
    // Copyright 在屏幕底部
    copyright->setPosition(screenWidth / 2.0f, screenHeight - 20.0f);
    addChild(copyright);
  }

  // 播放转场音效
  ResLoader::playMusic(MusicType::Swoosh);
}

void StartScene::onUpdate(float dt) {
  extra2d::Scene::onUpdate(dt);

  // 检测 A 键或空格开始游戏
  auto &input = extra2d::Application::instance().input();
  if (input.isButtonPressed(extra2d::GamepadButton::A)) {
    ResLoader::playMusic(MusicType::Click);
    startGame();
  }

  // 检测 BACK 键退出游戏
  if (input.isButtonPressed(extra2d::GamepadButton::Start)) {
    ResLoader::playMusic(MusicType::Click);
    auto &app = extra2d::Application::instance();
    app.quit();
  }
}

void StartScene::startGame() {
  auto &app = extra2d::Application::instance();
  app.scenes().replaceScene(extra2d::makePtr<GameScene>(),
                            extra2d::TransitionType::Fade, 0.5f);
}

} // namespace flappybird
