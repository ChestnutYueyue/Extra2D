#pragma once

#include <extra2d/extra2d.h>

namespace pushbox {

class MenuButton;

class StartScene : public extra2d::Scene {
public:
  StartScene();
  void onEnter() override;

private:
  void startNewGame();
  void continueGame();
  void exitGame();

  extra2d::Ptr<MenuButton> resumeBtn_;
  extra2d::Ptr<extra2d::ToggleImageButton> soundBtn_;
  extra2d::Ptr<extra2d::FontAtlas> font_;
};

} // namespace pushbox
