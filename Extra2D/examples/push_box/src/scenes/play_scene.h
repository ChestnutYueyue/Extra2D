#pragma once

#include "../core/data.h"
#include <extra2d/extra2d.h>

namespace pushbox {

class PlayScene : public extra2d::Scene {
public:
  explicit PlayScene(int level);

  void onEnter() override;
  void onUpdate(float dt) override;

private:
  void flush();
  void setLevel(int level);
  void setStep(int step);
  void move(int dx, int dy, int direct);
  void gameOver();

  int step_ = 0;
  Map map_{};

  extra2d::Ptr<extra2d::FontAtlas> font28_;
  extra2d::Ptr<extra2d::FontAtlas> font20_;

  extra2d::Ptr<extra2d::Text> levelText_;
  extra2d::Ptr<extra2d::Text> stepText_;
  extra2d::Ptr<extra2d::Text> bestText_;
  extra2d::Ptr<extra2d::Node> mapLayer_;

  extra2d::Ptr<extra2d::ToggleImageButton> soundBtn_;

  extra2d::Ptr<extra2d::Texture> texWall_;
  extra2d::Ptr<extra2d::Texture> texPoint_;
  extra2d::Ptr<extra2d::Texture> texFloor_;
  extra2d::Ptr<extra2d::Texture> texBox_;
  extra2d::Ptr<extra2d::Texture> texBoxInPoint_;

  extra2d::Ptr<extra2d::Texture> texMan_[5];
  extra2d::Ptr<extra2d::Texture> texManPush_[5];
};

} // namespace pushbox
