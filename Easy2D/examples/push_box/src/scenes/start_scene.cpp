#include "start_scene.h"

#include "../core/audio_context.h"
#include "../core/data.h"
#include "../nodes/audio_controller.h"
#include "../scenes/play_scene.h"
#include "../ui/menu_button.h"
#include <easy2d/easy2d.h>

namespace pushbox {

StartScene::StartScene() {
  // 设置视口大小为窗口尺寸
  auto& app = easy2d::Application::instance();
  auto& config = app.getConfig();
  setViewportSize(static_cast<float>(config.width), static_cast<float>(config.height));
}

static easy2d::Ptr<easy2d::FontAtlas> loadMenuFont() {
  auto& resources = easy2d::Application::instance().resources();
  return resources.loadFont("assets/font.ttf", 28);
}

void StartScene::onEnter() {
  Scene::onEnter();

  E2D_LOG_INFO("StartScene::onEnter() - BEGIN");

  auto& app = easy2d::Application::instance();
  auto& resources = app.resources();
  // 设置红色背景用于测试渲染
  setBackgroundColor(easy2d::Color(1.0f, 0.0f, 0.0f, 1.0f));
  E2D_LOG_INFO("StartScene: Background color set to RED for testing");

  if (getChildren().empty()) {
    E2D_LOG_INFO("StartScene: Creating audio controller...");
    auto audioNode = AudioController::create();
    audioNode->setName("audio_controller");
    addChild(audioNode);
    setAudioController(audioNode);
    E2D_LOG_INFO("StartScene: Audio controller created");

    E2D_LOG_INFO("StartScene: Loading background texture...");
    auto bgTex = resources.loadTexture("assets/images/start.jpg");
    if (bgTex) {
      E2D_LOG_INFO("StartScene: Background texture loaded successfully");
      auto background = easy2d::Sprite::create(bgTex);
      background->setAnchor(0.0f, 0.0f);
      background->setPosition(0.0f, 0.0f);
      float sx = static_cast<float>(app.getConfig().width) / static_cast<float>(bgTex->getWidth());
      float sy =
          static_cast<float>(app.getConfig().height) / static_cast<float>(bgTex->getHeight());
      background->setScale(sx, sy);
      addChild(background);
      E2D_LOG_INFO("StartScene: Background sprite added");
    } else {
      E2D_LOG_ERROR("StartScene: Failed to load background texture");
    }

    E2D_LOG_INFO("StartScene: Loading font...");
    font_ = loadMenuFont();
    if (font_) {
      E2D_LOG_INFO("StartScene: Font loaded successfully, creating menu buttons");
      // 字体加载成功，创建菜单按钮
      auto startBtn = MenuButton::create(font_, "新游戏", [this]() { startNewGame(); });
      startBtn->setPosition(app.getConfig().width / 2.0f, 260.0f);
      addChild(startBtn);

      resumeBtn_ = MenuButton::create(font_, "继续关卡", [this]() { continueGame(); });
      resumeBtn_->setPosition(app.getConfig().width / 2.0f, 300.0f);
      addChild(resumeBtn_);

      auto exitBtn = MenuButton::create(font_, "退出", [this]() { exitGame(); });
      exitBtn->setPosition(app.getConfig().width / 2.0f, 340.0f);
      addChild(exitBtn);
      E2D_LOG_INFO("StartScene: Menu buttons created");
    } else {
      E2D_LOG_ERROR("StartScene: Failed to load font, menu buttons will not be displayed");
    }

    E2D_LOG_INFO("StartScene: Loading sound icons...");
    auto soundOn = resources.loadTexture("assets/images/soundon.png");
    auto soundOff = resources.loadTexture("assets/images/soundoff.png");
    if (soundOn && soundOff) {
      E2D_LOG_INFO("StartScene: Sound icons loaded successfully");
      soundBtn_ = easy2d::ToggleImageButton::create();
      soundBtn_->setStateImages(soundOff, soundOn);
      soundBtn_->setCustomSize(static_cast<float>(soundOn->getWidth()),
                               static_cast<float>(soundOn->getHeight()));
      soundBtn_->setBorder(easy2d::Colors::Transparent, 0.0f);
      soundBtn_->setPosition(50.0f, 50.0f);
      soundBtn_->setOnStateChange([](bool on) {
        if (auto audio = getAudioController()) {
          audio->setEnabled(on);
        }
      });
      addChild(soundBtn_);
    } else {
      E2D_LOG_WARN("StartScene: Failed to load sound icons");
    }
  }

  if (resumeBtn_) {
    resumeBtn_->setEnabled(g_CurrentLevel != 1);
  }

  if (soundBtn_) {
    soundBtn_->setOn(g_SoundOpen);
  }

  E2D_LOG_INFO("StartScene::onEnter() - END");
}

void StartScene::startNewGame() {
  easy2d::Application::instance().scenes().replaceScene(
      easy2d::makePtr<PlayScene>(1), easy2d::TransitionType::Fade, 0.25f);
}

void StartScene::continueGame() {
  easy2d::Application::instance().scenes().replaceScene(
      easy2d::makePtr<PlayScene>(g_CurrentLevel), easy2d::TransitionType::Fade, 0.25f);
}

void StartScene::exitGame() { easy2d::Application::instance().quit(); }

} // namespace pushbox
