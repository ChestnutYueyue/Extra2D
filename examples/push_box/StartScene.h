#pragma once

#include <extra2d/extra2d.h>

namespace pushbox {

class StartScene : public extra2d::Scene {
public:
    StartScene();
    void onEnter() override;
    void onUpdate(float dt) override;

private:
    void updateMenuColors();
    void updateSoundIcon();
    void executeMenuItem();
    void startNewGame();
    void continueGame();
    void exitGame();

    extra2d::Ptr<extra2d::FontAtlas> font_;
    extra2d::Ptr<extra2d::Button> startBtn_;
    extra2d::Ptr<extra2d::Button> resumeBtn_;
    extra2d::Ptr<extra2d::Button> exitBtn_;
    extra2d::Ptr<extra2d::Sprite> soundIcon_;
    int selectedIndex_ = 0;
    int menuCount_ = 3;
};

} // namespace pushbox
