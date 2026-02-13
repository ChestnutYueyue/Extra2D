// ============================================================================
// GameOverLayer.cpp - 游戏结束层实现
// ============================================================================

#include "GameOverLayer.h"
#include "BaseScene.h"
#include "GameScene.h"
#include "Number.h"
#include "ResLoader.h"
#include "StartScene.h"

namespace flappybird {

GameOverLayer::GameOverLayer(int score) : score_(score) {
  // 注意：不要在构造函数中创建子节点
  // 因为此时 weak_from_this() 还不能使用
}

void GameOverLayer::onEnter() {
  Node::onEnter();

  // 在 onEnter 中初始化，此时 weak_from_this() 可用
  // 使用游戏逻辑分辨率
  float screenWidth = GAME_WIDTH;
  float screenHeight = GAME_HEIGHT;

  // 整体居中（x 坐标相对于屏幕中心）
  setPosition(extra2d::Vec2(screenWidth / 2.0f, screenHeight));

  // 显示 "Game Over" 文字（y=120，从顶部开始）
  auto gameOverFrame = ResLoader::getKeyFrame("text_game_over");
  if (gameOverFrame) {
    auto gameOver = extra2d::Sprite::create(gameOverFrame->getTexture(),
                                            gameOverFrame->getRect());
    gameOver->setAnchor(extra2d::Vec2(0.5f, 0.0f));
    gameOver->setPosition(extra2d::Vec2(0.0f, 120.0f)); // x=0 表示相对于中心点
    addChild(gameOver);
  }

  // 初始化得分面板
  initPanel(score_, screenHeight);

  // 初始化按钮
  initButtons();

  // 创建向上移动的动画（从屏幕底部移动到正常位置）
  auto moveAction = extra2d::makePtr<extra2d::MoveBy>(
      1.0f, extra2d::Vec2(0.0f, -screenHeight));
  moveAction->setCompletionCallback([this]() {
    animationDone_ = true;
    if (restartBtn_)
      restartBtn_->setEnabled(true);
    if (menuBtn_)
      menuBtn_->setEnabled(true);
    if (shareBtn_)
      shareBtn_->setEnabled(true);
  });
  runAction(moveAction);
}

void GameOverLayer::initPanel(int score, float screenHeight) {
  // 显示得分板（在屏幕中间）
  auto panelFrame = ResLoader::getKeyFrame("score_panel");
  if (!panelFrame)
    return;

  auto panel =
      extra2d::Sprite::create(panelFrame->getTexture(), panelFrame->getRect());
  panel->setAnchor(extra2d::Vec2(0.5f, 0.5f));
  panel->setPosition(
      extra2d::Vec2(0.0f, screenHeight / 2.0f)); // x=0 表示相对于中心点
  addChild(panel);

  // 获取最高分（从存储中读取）
  static int bestScore = 0;
  if (score > bestScore) {
    bestScore = score;
  }

  // 显示 "New" 标记（如果破了记录）
  if (score >= bestScore && score > 0) {
    auto newFrame = ResLoader::getKeyFrame("new");
    if (newFrame) {
      auto newSprite =
          extra2d::Sprite::create(newFrame->getTexture(), newFrame->getRect());
      newSprite->setAnchor(extra2d::Vec2(0.5f, 0.5f));
      // 调整位置使其在面板内部，靠近 BEST 分数
      newSprite->setPosition(
          extra2d::Vec2(30.0f, 25.0f)); // 相对于面板的坐标，在 BEST 右侧
      panel->addChild(newSprite);
    }
  }

  // 显示奖牌
  auto medalFrame = getMedal(score);
  if (medalFrame) {
    auto medal = extra2d::Sprite::create(medalFrame->getTexture(),
                                         medalFrame->getRect());
    medal->setAnchor(extra2d::Vec2(0.5f, 0.5f));
    medal->setPosition(extra2d::Vec2(54.0f, 68.0f)); // 相对于面板的坐标
    panel->addChild(medal);
  }

  // 显示本局得分
  auto scoreNumber = extra2d::makePtr<Number>();
  scoreNumber->setLittleNumber(score);
  scoreNumber->setPosition(
      extra2d::Vec2(80.0f, -15.0f)); // 相对于面板的坐标，右侧对齐
  panel->addChild(scoreNumber);

  // 显示最高分
  auto bestNumber = extra2d::makePtr<Number>();
  bestNumber->setLittleNumber(bestScore);
  bestNumber->setPosition(
      extra2d::Vec2(80.0f, 25.0f)); // 相对于面板的坐标，右侧对齐
  panel->addChild(bestNumber);
}

void GameOverLayer::initButtons() {
  auto restartFrame = ResLoader::getKeyFrame("button_restart");
  if (restartFrame) {
    restartBtn_ = extra2d::Button::create();
    restartBtn_->setBackgroundImage(restartFrame->getTexture(),
                                    restartFrame->getRect());
    restartBtn_->setAnchor(extra2d::Vec2(0.5f, 0.5f));
    restartBtn_->setPosition(extra2d::Vec2(0.0f, 360.0f));
    restartBtn_->setEnabled(false);
    restartBtn_->setOnClick([]() {
      ResLoader::playMusic(MusicType::Click);
      auto &app = extra2d::Application::instance();
      app.scenes().replaceScene(extra2d::makePtr<GameScene>(),
                                extra2d::TransitionType::Fade, 0.5f);
    });
    addChild(restartBtn_);
  }

  auto menuFrame = ResLoader::getKeyFrame("button_menu");
  if (menuFrame) {
    menuBtn_ = extra2d::Button::create();
    menuBtn_->setBackgroundImage(menuFrame->getTexture(), menuFrame->getRect());
    menuBtn_->setAnchor(extra2d::Vec2(0.5f, 0.5f));
    menuBtn_->setPosition(extra2d::Vec2(0.0f, 420.0f));
    menuBtn_->setEnabled(false);
    menuBtn_->setOnClick([]() {
      ResLoader::playMusic(MusicType::Click);
      auto &app = extra2d::Application::instance();
      app.scenes().replaceScene(extra2d::makePtr<StartScene>(),
                                extra2d::TransitionType::Fade, 0.5f);
    });
    addChild(menuBtn_);
  }

  auto shareFrame = ResLoader::getKeyFrame("button_share");
  if (shareFrame) {
    shareBtn_ = extra2d::Button::create();
    shareBtn_->setBackgroundImage(shareFrame->getTexture(),
                                  shareFrame->getRect());
    shareBtn_->setAnchor(extra2d::Vec2(0.5f, 0.5f));
    shareBtn_->setPosition(extra2d::Vec2(0.0f, 460.0f));
    shareBtn_->setEnabled(false);
    shareBtn_->setOnClick([]() { ResLoader::playMusic(MusicType::Click); });
    addChild(shareBtn_);
  }
}

void GameOverLayer::onUpdate(float dt) {
  Node::onUpdate(dt);

  if (!animationDone_)
    return;

  auto &input = extra2d::Application::instance().input();

  if (input.isButtonPressed(extra2d::GamepadButton::A)) {
    ResLoader::playMusic(MusicType::Click);
    auto &app = extra2d::Application::instance();
    app.scenes().replaceScene(extra2d::makePtr<GameScene>(),
                              extra2d::TransitionType::Fade, 0.5f);
  }

  if (input.isButtonPressed(extra2d::GamepadButton::B)) {
    ResLoader::playMusic(MusicType::Click);
    auto &app = extra2d::Application::instance();
    app.scenes().replaceScene(extra2d::makePtr<StartScene>(),
                              extra2d::TransitionType::Fade, 0.5f);
  }
}

extra2d::Ptr<extra2d::SpriteFrame> GameOverLayer::getMedal(int score) {
  if (score < 10) {
    return nullptr; // 无奖牌
  } else if (score < 20) {
    return ResLoader::getKeyFrame("medals_0"); // 铜牌
  } else if (score < 30) {
    return ResLoader::getKeyFrame("medals_1"); // 银牌
  } else if (score < 50) {
    return ResLoader::getKeyFrame("medals_2"); // 金牌
  } else {
    return ResLoader::getKeyFrame("medals_3"); // 钻石奖牌
  }
}

} // namespace flappybird
