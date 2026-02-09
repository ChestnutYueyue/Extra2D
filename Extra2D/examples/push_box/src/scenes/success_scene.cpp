#include "success_scene.h"

#include "../ui/menu_button.h"
#include <extra2d/app/application.h>
#include <extra2d/resource/resource_manager.h>
#include <extra2d/scene/scene_manager.h>
#include <extra2d/scene/sprite.h>

namespace pushbox {

SuccessScene::SuccessScene() {
  // 设置视口大小为窗口尺寸
  auto &app = extra2d::Application::instance();
  auto &config = app.getConfig();
  setViewportSize(static_cast<float>(config.width),
                  static_cast<float>(config.height));
}

static extra2d::Ptr<extra2d::FontAtlas> loadMenuFont() {
  auto &resources = extra2d::Application::instance().resources();
  return resources.loadFont("assets/font.ttf", 28);
}

void SuccessScene::onEnter() {
  Scene::onEnter();

  auto &app = extra2d::Application::instance();
  auto &resources = app.resources();
  setBackgroundColor(extra2d::Colors::Black);

  if (getChildren().empty()) {
    auto bgTex = resources.loadTexture("assets/images/success.jpg");
    if (bgTex) {
      auto background = extra2d::Sprite::create(bgTex);
      background->setAnchor(0.0f, 0.0f);
      background->setPosition(0.0f, 0.0f);
      float sx = static_cast<float>(app.getConfig().width) /
                 static_cast<float>(bgTex->getWidth());
      float sy = static_cast<float>(app.getConfig().height) /
                 static_cast<float>(bgTex->getHeight());
      background->setScale(sx, sy);
      addChild(background);
    }

    auto font = loadMenuFont();
    if (font) {
      auto backBtn = MenuButton::create(font, "回主菜单", []() {
        auto &scenes = extra2d::Application::instance().scenes();
        scenes.popScene(extra2d::TransitionType::Fade, 0.2f);
        scenes.popScene(extra2d::TransitionType::Fade, 0.2f);
      });
      backBtn->setPosition(app.getConfig().width / 2.0f, 350.0f);
      addChild(backBtn);
    }
  }
}

} // namespace pushbox
