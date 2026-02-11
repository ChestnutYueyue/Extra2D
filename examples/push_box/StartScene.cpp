#include "StartScene.h"

#include "audio_manager.h"
#include "data.h"
#include "PlayScene.h"
#include <extra2d/extra2d.h>

namespace pushbox {

StartScene::StartScene() {
    auto& app = extra2d::Application::instance();
    auto& config = app.getConfig();
    setViewportSize(static_cast<float>(config.width), static_cast<float>(config.height));
}

static extra2d::Ptr<extra2d::FontAtlas> loadMenuFont() {
    auto& resources = extra2d::Application::instance().resources();
    auto font = resources.loadFont("assets/font.ttf", 28,true);
    return font;
}

void StartScene::onEnter() {
    Scene::onEnter();

    auto& app = extra2d::Application::instance();
    auto& resources = app.resources();
    setBackgroundColor(extra2d::Colors::Black);

    if (getChildren().empty()) {

        float screenW = static_cast<float>(app.getConfig().width);
        float screenH = static_cast<float>(app.getConfig().height);

        auto bgTex = resources.loadTexture("assets/images/start.jpg");
        if (bgTex) {
            auto background = extra2d::Sprite::create(bgTex);
            float bgWidth = static_cast<float>(bgTex->getWidth());
            float bgHeight = static_cast<float>(bgTex->getHeight());
            float offsetX = (screenW - bgWidth) / 2.0f;
            float offsetY = (screenH - bgHeight) / 2.0f;
            
            background->setAnchor(0.0f, 0.0f);
            background->setPosition(offsetX, offsetY);
            addChild(background);
            
            float centerX = screenW / 2.0f;
            
            font_ = loadMenuFont();
            if (!font_) {
                return;
            }

            // 创建菜单按钮（使用 Button 实现文本居中）
            // 设置按钮锚点为中心点，位置设为屏幕中心，实现真正的居中
            startBtn_ = extra2d::Button::create();
            startBtn_->setFont(font_);
            startBtn_->setText("新游戏");
            startBtn_->setTextColor(extra2d::Colors::Black);
            startBtn_->setBackgroundColor(extra2d::Colors::Transparent, 
                                          extra2d::Colors::Transparent, 
                                          extra2d::Colors::Transparent);
            startBtn_->setBorder(extra2d::Colors::Transparent, 0.0f);
            startBtn_->setPadding(extra2d::Vec2(0.0f, 0.0f));
            startBtn_->setCustomSize(200.0f, 40.0f);
            startBtn_->setAnchor(0.5f, 0.5f);
            startBtn_->setPosition(centerX, offsetY + 260.0f);
            addChild(startBtn_);

            resumeBtn_ = extra2d::Button::create();
            resumeBtn_->setFont(font_);
            resumeBtn_->setText("继续关卡");
            resumeBtn_->setTextColor(extra2d::Colors::Black);
            resumeBtn_->setBackgroundColor(extra2d::Colors::Transparent, 
                                           extra2d::Colors::Transparent, 
                                           extra2d::Colors::Transparent);
            resumeBtn_->setBorder(extra2d::Colors::Transparent, 0.0f);
            resumeBtn_->setPadding(extra2d::Vec2(0.0f, 0.0f));
            resumeBtn_->setCustomSize(200.0f, 40.0f);
            resumeBtn_->setAnchor(0.5f, 0.5f);
            resumeBtn_->setPosition(centerX, offsetY + 300.0f);
            addChild(resumeBtn_);

            exitBtn_ = extra2d::Button::create();
            exitBtn_->setFont(font_);
            exitBtn_->setText("退出");
            exitBtn_->setTextColor(extra2d::Colors::Black);
            exitBtn_->setBackgroundColor(extra2d::Colors::Transparent, 
                                         extra2d::Colors::Transparent, 
                                         extra2d::Colors::Transparent);
            exitBtn_->setBorder(extra2d::Colors::Transparent, 0.0f);
            exitBtn_->setPadding(extra2d::Vec2(0.0f, 0.0f));
            exitBtn_->setCustomSize(200.0f, 40.0f);
            exitBtn_->setAnchor(0.5f, 0.5f);
            exitBtn_->setPosition(centerX, offsetY + 340.0f);
            addChild(exitBtn_);

            // 音效开关图标（相对于背景图左上角）
            auto soundOn = resources.loadTexture("assets/images/soundon.png");
            auto soundOff = resources.loadTexture("assets/images/soundoff.png");
            if (soundOn && soundOff) {
                soundIcon_ = extra2d::Sprite::create(g_SoundOpen ? soundOn : soundOff);
                soundIcon_->setPosition(offsetX + 50.0f, offsetY + 50.0f);
                addChild(soundIcon_);
            }
        }
    }

    // 始终有3个菜单项
    menuCount_ = 3;
    updateMenuColors();
}

void StartScene::onUpdate(float dt) {
    Scene::onUpdate(dt);

    auto& app = extra2d::Application::instance();
    auto& input = app.input();

    // 方向键上下切换选择
    if (input.isButtonPressed(extra2d::GamepadButton::DPadUp)) {
        selectedIndex_ = (selectedIndex_ - 1 + menuCount_) % menuCount_;
        updateMenuColors();
    } else if (input.isButtonPressed(extra2d::GamepadButton::DPadDown)) {
        selectedIndex_ = (selectedIndex_ + 1) % menuCount_;
        updateMenuColors();
    }

    // A键确认
    if (input.isButtonPressed(extra2d::GamepadButton::A)) {
        executeMenuItem();
    }

    // X键切换音效
    if (input.isButtonPressed(extra2d::GamepadButton::X)) {
        g_SoundOpen = !g_SoundOpen;
        AudioManager::instance().setEnabled(g_SoundOpen);
        updateSoundIcon();
    }
}

void StartScene::updateMenuColors() {
    // 根据选中状态更新按钮文本颜色
    // 选中的项用红色，未选中的用黑色，禁用的项用深灰色
    
    if (startBtn_) {
        startBtn_->setTextColor(selectedIndex_ == 0 ? extra2d::Colors::Red : extra2d::Colors::Black);
    }
    
    if (resumeBtn_) {
        // "继续关卡"始终显示，但当 g_CurrentLevel == 1 时禁用（深灰色）
        if (g_CurrentLevel > 1) {
            // 可用状态：选中为红色，未选中为黑色
            resumeBtn_->setTextColor(selectedIndex_ == 1 ? extra2d::Colors::Red : extra2d::Colors::Black);
        } else {
            // 禁用状态：深灰色 (RGB: 80, 80, 80)
            resumeBtn_->setTextColor(extra2d::Color(80, 80, 80, 255));
        }
    }
    
    if (exitBtn_) {
        exitBtn_->setTextColor(selectedIndex_ == 2 ? extra2d::Colors::Red : extra2d::Colors::Black);
    }
}

void StartScene::updateSoundIcon() {
    if (!soundIcon_) return;
    
    auto& app = extra2d::Application::instance();
    auto& resources = app.resources();
    auto soundOn = resources.loadTexture("assets/images/soundon.png");
    auto soundOff = resources.loadTexture("assets/images/soundoff.png");
    
    if (soundOn && soundOff) {
        soundIcon_->setTexture(g_SoundOpen ? soundOn : soundOff);
    }
}

void StartScene::executeMenuItem() {
    // 始终有3个选项，但"继续关卡"(索引1)在 g_CurrentLevel == 1 时禁用
    switch (selectedIndex_) {
    case 0:
        startNewGame();
        break;
    case 1:
        // 只有当 g_CurrentLevel > 1 时才能选择"继续关卡"
        if (g_CurrentLevel > 1) {
            continueGame();
        }
        break;
    case 2:
        exitGame();
        break;
    }
}

void StartScene::startNewGame() {
    extra2d::Application::instance().scenes().replaceScene(
        extra2d::makePtr<PlayScene>(1), extra2d::TransitionType::Fade, 0.25f);
}

void StartScene::continueGame() {
    extra2d::Application::instance().scenes().replaceScene(
        extra2d::makePtr<PlayScene>(g_CurrentLevel), extra2d::TransitionType::Fade, 0.25f);
}

void StartScene::exitGame() { 
    extra2d::Application::instance().quit(); 
}

} // namespace pushbox
