// ============================================================================
// GameScene.cpp - 游戏主场景实现
// ============================================================================

#include "GameScene.h"
#include "GameOverLayer.h"
#include "ResLoader.h"
#include "input.h"

namespace flappybird {

GameScene::GameScene() {
  auto &app = extra2d::Application::instance();
  auto &config = app.getConfig();
  setViewportSize(static_cast<float>(config.width),
                  static_cast<float>(config.height));
  // 设置背景颜色为黑色
  setBackgroundColor(extra2d::Color(0.0f, 0.0f, 0.0f, 1.0f));
}

void GameScene::onEnter() {
  extra2d::Scene::onEnter();

  auto &app = extra2d::Application::instance();
  float screenWidth = static_cast<float>(app.getConfig().width);
  float screenHeight = static_cast<float>(app.getConfig().height);

  // 添加背景（使用左上角锚点，与原游戏一致）
  auto bgFrame = ResLoader::getKeyFrame("bg_day");
  if (bgFrame) {
    auto background =
        extra2d::Sprite::create(bgFrame->getTexture(), bgFrame->getRect());
    background->setAnchor(extra2d::Vec2(0.0f, 0.0f));
    background->setPosition(extra2d::Vec2(0.0f, 0.0f));
    addChild(background);
  }

  // 添加水管（初始时隐藏，游戏开始后才显示）
  auto pipes = extra2d::makePtr<Pipes>();
  pipes_ = pipes.get();
  pipes->setVisible(false);
  addChild(pipes);

  // 添加小鸟（在屏幕中间偏左位置）
  auto bird = extra2d::makePtr<Bird>();
  bird->setPosition(
      extra2d::Vec2(screenWidth / 2.0f - 50.0f, screenHeight / 2.0f));
  bird_ = bird.get();
  addChild(bird);

  // 添加地面
  auto ground = extra2d::makePtr<Ground>();
  ground_ = ground.get();
  addChild(ground);

  // 添加得分（屏幕顶部中央）
  auto scoreNumber = extra2d::makePtr<Number>();
  scoreNumber->setPosition(extra2d::Vec2(screenWidth / 2.0f, 50.0f));
  scoreNumber->setNumber(0);
  scoreNumber_ = scoreNumber.get();
  addChild(scoreNumber);

  // 添加 ready 图片（屏幕中央偏上）
  auto readyFrame = ResLoader::getKeyFrame("text_ready");
  if (readyFrame) {
    readySprite_ = extra2d::Sprite::create(readyFrame->getTexture(),
                                           readyFrame->getRect());
    readySprite_->setAnchor(extra2d::Vec2(0.5f, 0.5f));
    readySprite_->setPosition(
        extra2d::Vec2(screenWidth / 2.0f, screenHeight / 2.0f - 70.0f));
    addChild(readySprite_);
  }

  // 添加教程图片（屏幕中央偏下）
  auto tutorialFrame = ResLoader::getKeyFrame("tutorial");
  if (tutorialFrame) {
    tutorialSprite_ = extra2d::Sprite::create(tutorialFrame->getTexture(),
                                              tutorialFrame->getRect());
    tutorialSprite_->setAnchor(extra2d::Vec2(0.5f, 0.5f));
    tutorialSprite_->setPosition(
        extra2d::Vec2(screenWidth / 2.0f, screenHeight / 2.0f + 30.0f));
    addChild(tutorialSprite_);
  }

  // 播放转场音效
  ResLoader::playMusic(MusicType::Swoosh);

  // 初始化状态
  started_ = false;
  score_ = 0;
}

void GameScene::onUpdate(float dt) {
  // 注意：这里要先调用父类的 onUpdate，以确保 GameOverLayer 的动画能播放
  extra2d::Scene::onUpdate(dt);

  // 游戏结束后不再更新游戏逻辑（但子节点的动画继续）
  if (gameOver_)
    return;

  if (!bird_)
    return;

  auto &input = extra2d::Application::instance().input();

  // 检测跳跃按键（A键或空格）
  if (input.isButtonPressed(extra2d::GamepadButton::A) ||
      input.isMousePressed(extra2d::MouseButton::Left)) {
    if (!started_) {
      // 游戏还没开始，开始游戏
      started_ = true;
      startGame();
    }
    bird_->jump();
  }

  // 游戏已经开始
  if (started_) {
    // 模拟小鸟下落
    bird_->fall(dt);

    // 检查得分（小鸟飞过水管）
    if (pipes_) {
      Pipe *firstPipe = pipes_->getPipe(0);
      if (firstPipe && !firstPipe->scored) {
        float birdX = bird_->getPosition().x;
        float pipeX = firstPipe->getPosition().x;
        if (pipeX <= birdX) {
          // 小鸟飞过了水管
          score_++;
          scoreNumber_->setNumber(score_);
          firstPipe->scored = true;
          ResLoader::playMusic(MusicType::Point);
        }
      }
    }

    // 检查碰撞
    if (bird_->isLiving() && checkCollision()) {
      onHit();
    }

    // 检查是否撞到地面（原游戏使用 123 作为地面高度）
    auto &app = extra2d::Application::instance();
    float screenHeight = static_cast<float>(app.getConfig().height);

    if (screenHeight - bird_->getPosition().y <= 123.0f) {
      // 小鸟撞到地面
      bird_->setPosition(
          extra2d::Vec2(bird_->getPosition().x, screenHeight - 123.0f));
      bird_->setStatus(Bird::Status::Still);
      onHit();

      gameOver();
    }
  }
}

void GameScene::startGame() {
  // 隐藏 ready 和 tutorial 图片
  if (readySprite_) {
    readySprite_->setVisible(false);
  }
  if (tutorialSprite_) {
    tutorialSprite_->setVisible(false);
  }

  // 显示并开始移动水管
  if (pipes_) {
    pipes_->setVisible(true);
    pipes_->start();
  }

  // 设置小鸟状态
  if (bird_) {
    bird_->setStatus(Bird::Status::StartToFly);
  }
}

bool GameScene::checkCollision() {
  if (!bird_ || !pipes_)
    return false;

  extra2d::Rect birdBox = bird_->getBoundingBox();

  // 检查与每个水管的碰撞
  for (int i = 0; i < 3; ++i) {
    Pipe *pipe = pipes_->getPipe(i);
    if (!pipe)
      continue;

    // 检查与上水管的碰撞
    extra2d::Rect topBox = pipe->getTopPipeBox();
    if (birdBox.intersects(topBox)) {
      return true;
    }

    // 检查与下水管的碰撞
    extra2d::Rect bottomBox = pipe->getBottomPipeBox();
    if (birdBox.intersects(bottomBox)) {
      return true;
    }
  }

  return false;
}

void GameScene::onHit() {
  if (!bird_->isLiving())
    return;

  // 小鸟死亡
  bird_->die();

  // 停止地面滚动
  if (ground_) {
    ground_->stop();
  }

  // 停止水管移动
  if (pipes_) {
    pipes_->stop();
  }

  // 停止小鸟动画
  if (bird_) {
    bird_->setStatus(Bird::Status::Still);
  }

  // 隐藏得分
  if (scoreNumber_) {
    scoreNumber_->setVisible(false);
  }

  // 设置游戏结束标志
  gameOver_ = true;

  // 延迟显示游戏结束界面
  gameOver();
}

void GameScene::gameOver() {
  started_ = false;
  gameOver_ = true;

  // 显示游戏结束层
  auto gameOverLayer = extra2d::makePtr<GameOverLayer>(score_);
  addChild(gameOverLayer);
}

} // namespace flappybird
