// ============================================================================
// FlappyBird - Extra2D 示例程序
// 作者: Extra2D Team
// 描述: 经典的 Flappy Bird 游戏实现
// ============================================================================

#include "ResLoader.h"
#include "SplashScene.h"
#include <extra2d/extra2d.h>

using namespace extra2d;

/**
 * @brief 程序入口
 */
int main(int argc, char **argv) {
  // 初始化日志系统
  Logger::init();
  Logger::setLevel(LogLevel::Debug);

  E2D_LOG_INFO("========================");
  E2D_LOG_INFO("Extra2D FlappyBird");
  E2D_LOG_INFO("========================");

  // 获取应用实例
  auto &app = Application::instance();

  // 配置应用
  AppConfig config;
  config.title = "Extra2D - FlappyBird";
  config.width = 1280; // 窗口宽度 (720P 分辨率)
  config.height = 720; // 窗口高度 (720P 分辨率)
  config.vsync = true;
  config.fpsLimit = 60;

  // 初始化应用
  if (!app.init(config)) {
    E2D_LOG_ERROR("应用初始化失败！");
    return -1;
  }

  // 初始化资源加载器
  flappybird::ResLoader::init();

  // 进入启动场景
  app.enterScene(makePtr<flappybird::SplashScene>());

  E2D_LOG_INFO("开始主循环...");
  app.run();

  E2D_LOG_INFO("应用结束");
  return 0;
}
