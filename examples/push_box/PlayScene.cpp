// ============================================================================
// PlayScene.cpp - Push Box 游戏场景实现
// ============================================================================

#include "PlayScene.h"

#include "StartScene.h"
#include "SuccessScene.h"
#include "audio_manager.h"
#include "storage.h"
#include <extra2d/extra2d.h>

namespace pushbox {

/**
 * @brief 加载字体
 * @param size 字体大小
 */
static extra2d::Ptr<extra2d::FontAtlas> loadFont(int size) {
  auto &resources = extra2d::Application::instance().resources();
  auto font = resources.loadFont("assets/font.ttf", size);
  return font;
}

PlayScene::PlayScene(int level) : BaseScene() {
  auto &app = extra2d::Application::instance();
  auto &resources = app.resources();

  E2D_LOG_INFO("PlayScene: Loading textures...");

  texWall_ = resources.loadTexture("assets/images/wall.gif");
  texPoint_ = resources.loadTexture("assets/images/point.gif");
  texFloor_ = resources.loadTexture("assets/images/floor.gif");
  texBox_ = resources.loadTexture("assets/images/box.gif");
  texBoxInPoint_ = resources.loadTexture("assets/images/boxinpoint.gif");

  texMan_[1] = resources.loadTexture("assets/images/player/manup.gif");
  texMan_[2] = resources.loadTexture("assets/images/player/mandown.gif");
  texMan_[3] = resources.loadTexture("assets/images/player/manleft.gif");
  texMan_[4] = resources.loadTexture("assets/images/player/manright.gif");

  texManPush_[1] = resources.loadTexture("assets/images/player/manhandup.gif");
  texManPush_[2] =
      resources.loadTexture("assets/images/player/manhanddown.gif");
  texManPush_[3] =
      resources.loadTexture("assets/images/player/manhandleft.gif");
  texManPush_[4] =
      resources.loadTexture("assets/images/player/manhandright.gif");

  font28_ = loadFont(28);
  font20_ = loadFont(20);

  // 使用游戏逻辑分辨率
  float screenW = GAME_WIDTH;
  float screenH = GAME_HEIGHT;

  // 计算游戏区域居中偏移
  float offsetX = (screenW - GAME_WIDTH) / 2.0f;
  float offsetY = (screenH - GAME_HEIGHT) / 2.0f;

  // 音效开关按钮（使用 Button 的切换模式）
  auto soundOn = resources.loadTexture("assets/images/soundon.png");
  auto soundOff = resources.loadTexture("assets/images/soundoff.png");
  if (soundOn && soundOff) {
    soundBtn_ = extra2d::Button::create();
    soundBtn_->setToggleMode(true);
    soundBtn_->setStateBackgroundImage(soundOff, soundOn);
    soundBtn_->setOn(g_SoundOpen);
    soundBtn_->setAnchor(0.0f, 0.0f);
    soundBtn_->setPosition(offsetX + 50.0f, offsetY + 50.0f);
    soundBtn_->setOnStateChange([](bool isOn) {
      g_SoundOpen = isOn;
      AudioManager::instance().setEnabled(isOn);
    });
    addChild(soundBtn_);
  }

  levelText_ = extra2d::Text::create("", font28_);
  levelText_->setPosition(offsetX + 520.0f, offsetY + 30.0f);
  levelText_->setTextColor(extra2d::Colors::White);
  addChild(levelText_);

  stepText_ = extra2d::Text::create("", font20_);
  stepText_->setPosition(offsetX + 520.0f, offsetY + 100.0f);
  stepText_->setTextColor(extra2d::Colors::White);
  addChild(stepText_);

  bestText_ = extra2d::Text::create("", font20_);
  bestText_->setPosition(offsetX + 520.0f, offsetY + 140.0f);
  bestText_->setTextColor(extra2d::Colors::White);
  addChild(bestText_);

  // 创建菜单文本（使用颜色变化指示选中）
  restartText_ = extra2d::Text::create("Y键重开", font20_);
  restartText_->setPosition(offsetX + 520.0f, offsetY + 290.0f);
  addChild(restartText_);

  soundToggleText_ = extra2d::Text::create("X键切换音效", font20_);
  soundToggleText_->setPosition(offsetX + 520.0f, offsetY + 330.0f);
  addChild(soundToggleText_);

  mapLayer_ = extra2d::makePtr<extra2d::Node>();
  mapLayer_->setAnchor(0.0f, 0.0f);
  mapLayer_->setPosition(0.0f, 0.0f);
  addChild(mapLayer_);

  setLevel(level);
}

void PlayScene::onEnter() {
  BaseScene::onEnter();
  if (soundBtn_) {
    soundBtn_->setOn(g_SoundOpen);
  }
  updateMenuColors();
}

/**
 * @brief 更新菜单颜色
 */
void PlayScene::updateMenuColors() {
  // 选中的项用红色，未选中的用白色
  if (restartText_) {
    restartText_->setTextColor(menuIndex_ == 0 ? extra2d::Colors::Red
                                               : extra2d::Colors::White);
  }
  if (soundToggleText_) {
    soundToggleText_->setTextColor(menuIndex_ == 1 ? extra2d::Colors::Red
                                                   : extra2d::Colors::White);
  }
}

void PlayScene::onUpdate(float dt) {
  BaseScene::onUpdate(dt);

  auto &app = extra2d::Application::instance();
  auto &input = app.input();

  // B 键返回主菜单
  if (input.isButtonPressed(extra2d::GamepadButton::B)) {
    app.scenes().replaceScene(extra2d::makePtr<StartScene>(),
                              extra2d::TransitionType::Fade, 0.5f);
    return;
  }

  // Y 键重开
  if (input.isButtonPressed(extra2d::GamepadButton::Y)) {
    setLevel(g_CurrentLevel);
    return;
  }

  // X键直接切换音效（备用，按钮也可点击切换）
  if (input.isButtonPressed(extra2d::GamepadButton::X)) {
    g_SoundOpen = !g_SoundOpen;
    AudioManager::instance().setEnabled(g_SoundOpen);
    if (soundBtn_) {
      soundBtn_->setOn(g_SoundOpen);
    }
    return;
  }

  // A 键执行选中的菜单项
  if (input.isButtonPressed(extra2d::GamepadButton::A)) {
    executeMenuItem();
    return;
  }

  // 方向键移动
  if (input.isButtonPressed(extra2d::GamepadButton::DPadUp)) {
    move(0, -1, 1);
    flush();
  } else if (input.isButtonPressed(extra2d::GamepadButton::DPadDown)) {
    move(0, 1, 2);
    flush();
  } else if (input.isButtonPressed(extra2d::GamepadButton::DPadLeft)) {
    move(-1, 0, 3);
    flush();
  } else if (input.isButtonPressed(extra2d::GamepadButton::DPadRight)) {
    move(1, 0, 4);
    flush();
  } else {
    return;
  }

  // 检查是否通关
  for (int i = 0; i < map_.width; i++) {
    for (int j = 0; j < map_.height; j++) {
      Piece p = map_.value[j][i];
      if (p.type == TYPE::Box && p.isPoint == false) {
        return;
      }
    }
  }

  gameOver();
}

/**
 * @brief 执行选中的菜单项
 */
void PlayScene::executeMenuItem() {
  switch (menuIndex_) {
  case 0: // 重开
    setLevel(g_CurrentLevel);
    break;
  case 1: // 切换音效
    g_SoundOpen = !g_SoundOpen;
    AudioManager::instance().setEnabled(g_SoundOpen);
    if (soundBtn_) {
      soundBtn_->setOn(g_SoundOpen);
    }
    break;
  }
}

/**
 * @brief 刷新地图显示
 */
void PlayScene::flush() {
  mapLayer_->removeAllChildren();

  int tileW = texFloor_ ? texFloor_->getWidth() : 32;
  int tileH = texFloor_ ? texFloor_->getHeight() : 32;

  // 使用游戏逻辑分辨率
  float gameWidth = GAME_WIDTH;
  float gameHeight = GAME_HEIGHT;
  float baseOffsetX = 0.0f;
  float baseOffsetY = 0.0f;

  // 在 12x12 网格中居中地图
  float mapOffsetX = static_cast<float>((12.0f - map_.width) / 2.0f) * tileW;
  float mapOffsetY = static_cast<float>((12.0f - map_.height) / 2.0f) * tileH;

  float offsetX = baseOffsetX + mapOffsetX;
  float offsetY = baseOffsetY + mapOffsetY;

  for (int i = 0; i < map_.width; i++) {
    for (int j = 0; j < map_.height; j++) {
      Piece piece = map_.value[j][i];

      extra2d::Ptr<extra2d::Texture> tex;

      if (piece.type == TYPE::Wall) {
        tex = texWall_;
      } else if (piece.type == TYPE::Ground && piece.isPoint) {
        tex = texPoint_;
      } else if (piece.type == TYPE::Ground) {
        tex = texFloor_;
      } else if (piece.type == TYPE::Box && piece.isPoint) {
        tex = texBoxInPoint_;
      } else if (piece.type == TYPE::Box) {
        tex = texBox_;
      } else if (piece.type == TYPE::Man && g_Pushing) {
        tex = texManPush_[g_Direct];
      } else if (piece.type == TYPE::Man) {
        tex = texMan_[g_Direct];
      } else {
        continue;
      }

      if (!tex) {
        continue;
      }

      auto sprite = extra2d::Sprite::create(tex);
      sprite->setAnchor(0.0f, 0.0f);
      sprite->setPosition(offsetX + static_cast<float>(i * tileW),
                          offsetY + static_cast<float>(j * tileH));
      mapLayer_->addChild(sprite);
    }
  }
}

/**
 * @brief 设置关卡
 * @param level 关卡编号
 */
void PlayScene::setLevel(int level) {
  g_CurrentLevel = level;
  saveCurrentLevel(g_CurrentLevel);

  if (levelText_) {
    levelText_->setText("第" + std::to_string(level) + "关");
  }

  setStep(0);

  int bestStep = loadBestStep(level, 0);
  if (bestText_) {
    if (bestStep != 0) {
      bestText_->setText("最佳" + std::to_string(bestStep) + "步");
    } else {
      bestText_->setText("");
    }
  }

  // 深拷贝地图数据
  Map &sourceMap = g_Maps[level - 1];
  map_.width = sourceMap.width;
  map_.height = sourceMap.height;
  map_.roleX = sourceMap.roleX;
  map_.roleY = sourceMap.roleY;
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 12; j++) {
      map_.value[i][j] = sourceMap.value[i][j];
    }
  }

  g_Direct = 2;
  g_Pushing = false;
  flush();
}

/**
 * @brief 设置步数
 * @param step 步数
 */
void PlayScene::setStep(int step) {
  step_ = step;
  if (stepText_) {
    stepText_->setText("当前" + std::to_string(step) + "步");
  }
}

/**
 * @brief 移动玩家
 * @param dx X方向偏移
 * @param dy Y方向偏移
 * @param direct 方向（1=上，2=下，3=左，4=右）
 */
void PlayScene::move(int dx, int dy, int direct) {
  int targetX = dx + map_.roleX;
  int targetY = dy + map_.roleY;
  g_Direct = direct;

  if (targetX < 0 || targetX >= map_.width || targetY < 0 ||
      targetY >= map_.height) {
    return;
  }

  if (map_.value[targetY][targetX].type == TYPE::Wall) {
    return;
  }

  if (map_.value[targetY][targetX].type == TYPE::Ground) {
    g_Pushing = false;
    map_.value[map_.roleY][map_.roleX].type = TYPE::Ground;
    map_.value[targetY][targetX].type = TYPE::Man;
    AudioManager::instance().playManMove();
  } else if (map_.value[targetY][targetX].type == TYPE::Box) {
    g_Pushing = true;

    int boxX = 0;
    int boxY = 0;
    switch (g_Direct) {
    case 1:
      boxX = targetX;
      boxY = targetY - 1;
      break;
    case 2:
      boxX = targetX;
      boxY = targetY + 1;
      break;
    case 3:
      boxX = targetX - 1;
      boxY = targetY;
      break;
    case 4:
      boxX = targetX + 1;
      boxY = targetY;
      break;
    default:
      return;
    }

    if (boxX < 0 || boxX >= map_.width || boxY < 0 || boxY >= map_.height) {
      return;
    }

    if (map_.value[boxY][boxX].type == TYPE::Wall ||
        map_.value[boxY][boxX].type == TYPE::Box) {
      return;
    }

    map_.value[boxY][boxX].type = TYPE::Box;
    map_.value[targetY][targetX].type = TYPE::Man;
    map_.value[map_.roleY][map_.roleX].type = TYPE::Ground;

    AudioManager::instance().playBoxMove();
  } else {
    return;
  }

  map_.roleX = targetX;
  map_.roleY = targetY;
  setStep(step_ + 1);
}

/**
 * @brief 游戏通关
 */
void PlayScene::gameOver() {
  int bestStep = loadBestStep(g_CurrentLevel, 0);
  if (bestStep == 0 || step_ < bestStep) {
    saveBestStep(g_CurrentLevel, step_);
  }

  if (g_CurrentLevel == MAX_LEVEL) {
    extra2d::Application::instance().scenes().pushScene(
        extra2d::makePtr<SuccessScene>(), extra2d::TransitionType::Fade, 0.5f);
    return;
  }

  setLevel(g_CurrentLevel + 1);
}

} // namespace pushbox
