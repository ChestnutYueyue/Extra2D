// ============================================================================
// SuccessScene.cpp - Push Box 通关场景实现
// ============================================================================

#include "SuccessScene.h"

#include <extra2d/extra2d.h>

namespace pushbox {

SuccessScene::SuccessScene() : BaseScene() {
    // BaseScene 已处理视口设置
}

/**
 * @brief 加载菜单字体
 */
static extra2d::Ptr<extra2d::FontAtlas> loadMenuFont() {
    auto& resources = extra2d::Application::instance().resources();
    auto font = resources.loadFont("assets/font.ttf", 28);
    return font;
}

void SuccessScene::onEnter() {
    BaseScene::onEnter();

    auto& app = extra2d::Application::instance();
    auto& resources = app.resources();

    if (getChildren().empty()) {
        // 使用游戏逻辑分辨率
        float screenW = GAME_WIDTH;
        float screenH = GAME_HEIGHT;

        auto bgTex = resources.loadTexture("assets/images/success.jpg");
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
            
            auto font = loadMenuFont();
            if (font) {
                // 创建按钮文本（仅显示，不响应鼠标）
                auto backText = extra2d::Text::create("回主菜单", font);
                backText->setPosition(centerX, offsetY + 350.0f);
                backText->setTextColor(extra2d::Colors::Black);
                addChild(backText);

                // 创建选择指示器（箭头）
                selectorText_ = extra2d::Text::create(">", font);
                selectorText_->setTextColor(extra2d::Colors::Red);
                selectorText_->setPosition(centerX - 80.0f, offsetY + 350.0f);
                addChild(selectorText_);
            }
        }
    }
}

void SuccessScene::onUpdate(float dt) {
    BaseScene::onUpdate(dt);

    auto& app = extra2d::Application::instance();
    auto& input = app.input();

    // A键确认返回主菜单
    if (input.isButtonPressed(extra2d::GamepadButton::A)) {
        auto& scenes = extra2d::Application::instance().scenes();
        scenes.popScene(extra2d::TransitionType::Fade, 0.5f);
        scenes.popScene(extra2d::TransitionType::Fade, 0.5f);
    }
}

} // namespace pushbox
