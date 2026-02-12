// ============================================================================
// SplashScene.cpp - 启动场景实现
// ============================================================================

#include "SplashScene.h"
#include "StartScene.h"
#include "ResLoader.h"

namespace flappybird {

SplashScene::SplashScene() {
    // 设置视口大小
    auto& app = extra2d::Application::instance();
    auto& config = app.getConfig();
    setViewportSize(static_cast<float>(config.width), static_cast<float>(config.height));
}

void SplashScene::onEnter() {
    extra2d::Scene::onEnter();

    // 设置黑色背景
    setBackgroundColor(extra2d::Color(0.0f, 0.0f, 0.0f, 1.0f));

    auto viewport = getViewportSize();
    float centerX = viewport.width / 2.0f;
    float centerY = viewport.height / 2.0f;

    // 尝试加载 splash 图片
    auto splashFrame = ResLoader::getKeyFrame("splash");
    if (splashFrame) {
        auto splash = extra2d::Sprite::create(splashFrame->getTexture(), splashFrame->getRect());
        splash->setAnchor(0.5f, 0.5f);
        splash->setPosition(centerX, centerY);
        addChild(splash);
    } else {
        // 如果 splash 加载失败，尝试加载 title 图片作为备用
        auto titleFrame = ResLoader::getKeyFrame("title");
        if (titleFrame) {
            auto title = extra2d::Sprite::create(titleFrame->getTexture(), titleFrame->getRect());
            title->setAnchor(0.5f, 0.5f);
            title->setPosition(centerX, centerY);
            addChild(title);
        }
    }

    // 播放转场音效
    ResLoader::playMusic(MusicType::Swoosh);
}

void SplashScene::onUpdate(float dt) {
    extra2d::Scene::onUpdate(dt);

    // 计时
    timer_ += dt;
    if (timer_ >= delay_) {
        gotoStartScene();
    }
}

void SplashScene::gotoStartScene() {
    auto& app = extra2d::Application::instance();
    app.scenes().replaceScene(
        extra2d::makePtr<StartScene>(),
        extra2d::TransitionType::Fade,
        0.5f
    );
}

} // namespace flappybird
