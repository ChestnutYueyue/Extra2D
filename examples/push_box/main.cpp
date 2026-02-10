#include <extra2d/extra2d.h>
#include "StartScene.h"
#include "data.h"
#include "storage.h"

using namespace extra2d;

// ============================================================================
// 程序入口
// ============================================================================

int main(int argc, char **argv)
{
    Logger::init();
    Logger::setLevel(LogLevel::Debug);

    E2D_LOG_INFO("========================");
    E2D_LOG_INFO("Extra2D push_box");
    E2D_LOG_INFO("Platform: {}", platform::getPlatformName());
    E2D_LOG_INFO("========================");

    auto &app = Application::instance();

    AppConfig config;
    config.title = "Extra2D - push_box";
    config.width = 1280;
    config.height = 720;
    config.vsync = true;
    config.fpsLimit = 60;

    if (!app.init(config)) {
        E2D_LOG_ERROR("应用初始化失败！");
        return -1;
    }

    // 初始化存储系统
    pushbox::initStorage("sdmc:/");
    pushbox::g_CurrentLevel = pushbox::loadCurrentLevel(1);
    if (pushbox::g_CurrentLevel > MAX_LEVEL) {
        pushbox::g_CurrentLevel = 1;
    }
    pushbox::g_SoundOpen = pushbox::loadSoundOpen(true);

    // 进入开始场景（主界面）
    app.enterScene(makePtr<pushbox::StartScene>());

    E2D_LOG_INFO("开始主循环...");
    app.run();

    E2D_LOG_INFO("应用结束");
    return 0;
}
