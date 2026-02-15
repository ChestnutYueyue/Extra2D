/**
 * @file main.cpp
 * @brief Extra2D 基础示例程序
 * 
 * 演示如何使用 Extra2D 引擎创建一个简单的窗口应用程序。
 */

#include <extra2d/extra2d.h>
#include <iostream>

using namespace extra2d;

/**
 * @brief 主函数
 * 
 * 初始化应用程序，创建场景，运行主循环。
 */
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "Extra2D Demo - Starting..." << std::endl;

    AppConfig config = AppConfig::createDefault();
    config.appName = "Extra2D Demo";
    config.appVersion = "1.0.0";
    config.window.title = "Extra2D Demo";
    config.window.width = 800;
    config.window.height = 600;
    config.window.mode = WindowMode::Windowed;
    config.window.resizable = true;
    config.window.vsync = true;
    config.render.targetFPS = 60;

    Application& app = Application::get();

    if (!app.init(config)) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }

    std::cout << "Application initialized successfully!" << std::endl;
    std::cout << "Window: " << app.window().width() << "x" << app.window().height() << std::endl;
    std::cout << "Running main loop. Press ESC or close window to exit." << std::endl;

    auto scene = Scene::create();
    scene->setBackgroundColor(Colors::SkyBlue);
    scene->setViewportSize(static_cast<float>(config.window.width),
                           static_cast<float>(config.window.height));
    app.enterScene(scene);

    app.run();

    std::cout << "Shutting down..." << std::endl;
    app.shutdown();

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
