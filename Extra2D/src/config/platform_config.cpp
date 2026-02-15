#include <extra2d/config/app_config.h>
#include <extra2d/config/platform_config.h>
#include <extra2d/utils/logger.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace extra2d {

namespace {

class WindowsPlatformConfig : public PlatformConfig {
public:
  WindowsPlatformConfig() {
    caps_.supportsWindowed = true;
    caps_.supportsFullscreen = true;
    caps_.supportsBorderless = true;
    caps_.supportsCursor = true;
    caps_.supportsCursorHide = true;
    caps_.supportsDPIAwareness = true;
    caps_.supportsVSync = true;
    caps_.supportsMultiMonitor = true;
    caps_.supportsClipboard = true;
    caps_.supportsGamepad = true;
    caps_.supportsTouch = false;
    caps_.supportsKeyboard = true;
    caps_.supportsMouse = true;
    caps_.supportsResize = true;
    caps_.supportsHighDPI = true;
    caps_.maxTextureSize = 16384;
    caps_.preferredScreenWidth = 1920;
    caps_.preferredScreenHeight = 1080;
    caps_.defaultDPI = 96.0f;
  }

  PlatformType platformType() const override { return PlatformType::Windows; }
  const char *platformName() const override { return "Windows"; }
  const PlatformCapabilities &capabilities() const override { return caps_; }

  void applyConstraints(AppConfig &config) const override {
    if (config.window.width < 320)
      config.window.width = 320;
    if (config.window.height < 240)
      config.window.height = 240;
    if (config.window.width > caps_.maxTextureSize)
      config.window.width = caps_.maxTextureSize;
    if (config.window.height > caps_.maxTextureSize)
      config.window.height = caps_.maxTextureSize;
  }

  void applyDefaults(AppConfig &config) const override {
    config.window.highDPI = true;
    config.window.resizable = true;
    config.render.vsync = true;
    config.render.targetFPS = 60;
  }

  bool validateConfig(AppConfig &config) const override {
    if (config.window.width <= 0 || config.window.height <= 0) {
      E2D_LOG_ERROR("Windows: Invalid window dimensions");
      return false;
    }
    return true;
  }

  int getRecommendedWidth() const override { return 1920; }
  int getRecommendedHeight() const override { return 1080; }
  bool isResolutionSupported(int width, int height) const override {
    return width >= 320 && height >= 240 && width <= caps_.maxTextureSize &&
           height <= caps_.maxTextureSize;
  }

private:
  PlatformCapabilities caps_;
};

class LinuxPlatformConfig : public PlatformConfig {
public:
  LinuxPlatformConfig() {
    caps_.supportsWindowed = true;
    caps_.supportsFullscreen = true;
    caps_.supportsBorderless = true;
    caps_.supportsCursor = true;
    caps_.supportsCursorHide = true;
    caps_.supportsDPIAwareness = true;
    caps_.supportsVSync = true;
    caps_.supportsMultiMonitor = true;
    caps_.supportsClipboard = true;
    caps_.supportsGamepad = true;
    caps_.supportsTouch = false;
    caps_.supportsKeyboard = true;
    caps_.supportsMouse = true;
    caps_.supportsResize = true;
    caps_.supportsHighDPI = true;
    caps_.maxTextureSize = 16384;
    caps_.preferredScreenWidth = 1920;
    caps_.preferredScreenHeight = 1080;
    caps_.defaultDPI = 96.0f;
  }

  PlatformType platformType() const override { return PlatformType::Linux; }
  const char *platformName() const override { return "Linux"; }
  const PlatformCapabilities &capabilities() const override { return caps_; }

  void applyConstraints(AppConfig &config) const override {
    if (config.window.width < 320)
      config.window.width = 320;
    if (config.window.height < 240)
      config.window.height = 240;
  }

  void applyDefaults(AppConfig &config) const override {
    config.window.resizable = true;
    config.render.vsync = true;
  }

  bool validateConfig(AppConfig &config) const override {
    if (config.window.width <= 0 || config.window.height <= 0) {
      E2D_LOG_ERROR("Linux: Invalid window dimensions");
      return false;
    }
    return true;
  }

  int getRecommendedWidth() const override { return 1920; }
  int getRecommendedHeight() const override { return 1080; }
  bool isResolutionSupported(int width, int height) const override {
    return width >= 320 && height >= 240;
  }

private:
  PlatformCapabilities caps_;
};

class MacOSPlatformConfig : public PlatformConfig {
public:
  MacOSPlatformConfig() {
    caps_.supportsWindowed = true;
    caps_.supportsFullscreen = true;
    caps_.supportsBorderless = true;
    caps_.supportsCursor = true;
    caps_.supportsCursorHide = true;
    caps_.supportsDPIAwareness = true;
    caps_.supportsVSync = true;
    caps_.supportsMultiMonitor = true;
    caps_.supportsClipboard = true;
    caps_.supportsGamepad = true;
    caps_.supportsTouch = false;
    caps_.supportsKeyboard = true;
    caps_.supportsMouse = true;
    caps_.supportsResize = true;
    caps_.supportsHighDPI = true;
    caps_.maxTextureSize = 16384;
    caps_.preferredScreenWidth = 1920;
    caps_.preferredScreenHeight = 1080;
    caps_.defaultDPI = 144.0f;
  }

  PlatformType platformType() const override { return PlatformType::macOS; }
  const char *platformName() const override { return "macOS"; }
  const PlatformCapabilities &capabilities() const override { return caps_; }

  void applyConstraints(AppConfig &config) const override {
    if (config.window.width < 320)
      config.window.width = 320;
    if (config.window.height < 240)
      config.window.height = 240;
  }

  void applyDefaults(AppConfig &config) const override {
    config.window.highDPI = true;
    config.window.resizable = true;
    config.render.vsync = true;
  }

  bool validateConfig(AppConfig &config) const override {
    if (config.window.width <= 0 || config.window.height <= 0) {
      E2D_LOG_ERROR("macOS: Invalid window dimensions");
      return false;
    }
    return true;
  }

  int getRecommendedWidth() const override { return 1920; }
  int getRecommendedHeight() const override { return 1080; }
  bool isResolutionSupported(int width, int height) const override {
    return width >= 320 && height >= 240;
  }

private:
  PlatformCapabilities caps_;
};

#ifdef __SWITCH__
class SwitchPlatformConfig : public PlatformConfig {
public:
  SwitchPlatformConfig() {
    caps_.supportsWindowed = false;
    caps_.supportsFullscreen = true;
    caps_.supportsBorderless = false;
    caps_.supportsCursor = false;
    caps_.supportsCursorHide = false;
    caps_.supportsDPIAwareness = false;
    caps_.supportsVSync = true;
    caps_.supportsMultiMonitor = false;
    caps_.supportsClipboard = false;
    caps_.supportsGamepad = true;
    caps_.supportsTouch = true;
    caps_.supportsKeyboard = false;
    caps_.supportsMouse = false;
    caps_.supportsResize = false;
    caps_.supportsHighDPI = false;
    caps_.maxTextureSize = 8192;
    caps_.preferredScreenWidth = 1920;
    caps_.preferredScreenHeight = 1080;
    caps_.defaultDPI = 96.0f;
  }

  PlatformType platformType() const override { return PlatformType::Switch; }
  const char *platformName() const override { return "Nintendo Switch"; }
  const PlatformCapabilities &capabilities() const override { return caps_; }

  void applyConstraints(AppConfig &config) const override {
    config.window.width = 1920;
    config.window.height = 1080;
    config.window.mode = WindowMode::Fullscreen;
    config.window.resizable = false;
    config.window.borderless = false;
    config.window.highDPI = false;
    config.render.vsync = true;
    config.render.targetFPS = 60;
    config.input.enableVibration = true;
    config.input.maxGamepads = 2;
  }

  void applyDefaults(AppConfig &config) const override {
    config.window.width = 1920;
    config.window.height = 1080;
    config.window.mode = WindowMode::Fullscreen;
    config.window.resizable = false;
    config.render.vsync = true;
    config.render.targetFPS = 60;
    config.input.enableVibration = true;
  }

  bool validateConfig(AppConfig &config) const override {
    if (config.window.mode != WindowMode::Fullscreen) {
      E2D_LOG_WARN("Switch: Only fullscreen mode is supported");
      config.window.mode = WindowMode::Fullscreen;
    }
    return true;
  }

  int getRecommendedWidth() const override { return 1920; }
  int getRecommendedHeight() const override { return 1080; }
  bool isResolutionSupported(int width, int height) const override {
    return (width == 1920 && height == 1080) ||
           (width == 1280 && height == 720);
  }

private:
  PlatformCapabilities caps_;
};
#else
class SwitchPlatformConfig : public PlatformConfig {
public:
  SwitchPlatformConfig() {
    caps_.supportsWindowed = false;
    caps_.supportsFullscreen = true;
    caps_.supportsBorderless = false;
    caps_.supportsCursor = false;
    caps_.supportsCursorHide = false;
    caps_.supportsDPIAwareness = false;
    caps_.supportsVSync = true;
    caps_.supportsMultiMonitor = false;
    caps_.supportsClipboard = false;
    caps_.supportsGamepad = true;
    caps_.supportsTouch = true;
    caps_.supportsKeyboard = false;
    caps_.supportsMouse = false;
    caps_.supportsResize = false;
    caps_.supportsHighDPI = false;
    caps_.maxTextureSize = 8192;
    caps_.preferredScreenWidth = 1920;
    caps_.preferredScreenHeight = 1080;
    caps_.defaultDPI = 96.0f;
  }

  PlatformType platformType() const override { return PlatformType::Switch; }
  const char *platformName() const override { return "Nintendo Switch"; }
  const PlatformCapabilities &capabilities() const override { return caps_; }

  void applyConstraints(AppConfig &config) const override {
    config.window.width = 1920;
    config.window.height = 1080;
    config.window.mode = WindowMode::Fullscreen;
    config.window.resizable = false;
    config.window.borderless = false;
    config.window.highDPI = false;
    config.render.vsync = true;
    config.render.targetFPS = 60;
    config.input.enableVibration = true;
    config.input.maxGamepads = 2;
  }

  void applyDefaults(AppConfig &config) const override {
    config.window.width = 1920;
    config.window.height = 1080;
    config.window.mode = WindowMode::Fullscreen;
    config.window.resizable = false;
    config.render.vsync = true;
    config.render.targetFPS = 60;
    config.input.enableVibration = true;
  }

  bool validateConfig(AppConfig &config) const override {
    if (config.window.mode != WindowMode::Fullscreen) {
      E2D_LOG_WARN("Switch: Only fullscreen mode is supported");
    }
    return true;
  }

  int getRecommendedWidth() const override { return 1920; }
  int getRecommendedHeight() const override { return 1080; }
  bool isResolutionSupported(int width, int height) const override {
    return (width == 1920 && height == 1080) ||
           (width == 1280 && height == 720);
  }

private:
  PlatformCapabilities caps_;
};
#endif

} // namespace

/**
 * @brief 创建平台配置实例
 * 根据 PlatformType 创建对应的平台配置对象
 * @param type 平台类型，默认为 Auto（自动检测）
 * @return 平台配置的智能指针
 */
UniquePtr<PlatformConfig> createPlatformConfig(PlatformType type) {
  if (type == PlatformType::Auto) {
#ifdef _WIN32
    type = PlatformType::Windows;
#elif defined(__SWITCH__)
    type = PlatformType::Switch;
#elif defined(__linux__)
    type = PlatformType::Linux;
#elif defined(__APPLE__)
    type = PlatformType::macOS;
#else
    type = PlatformType::Windows;
#endif
  }

  switch (type) {
  case PlatformType::Windows:
    E2D_LOG_INFO("Creating Windows platform config");
    return makeUnique<WindowsPlatformConfig>();
  case PlatformType::Switch:
    E2D_LOG_INFO("Creating Nintendo Switch platform config");
    return makeUnique<SwitchPlatformConfig>();
  case PlatformType::Linux:
    E2D_LOG_INFO("Creating Linux platform config");
    return makeUnique<LinuxPlatformConfig>();
  case PlatformType::macOS:
    E2D_LOG_INFO("Creating macOS platform config");
    return makeUnique<MacOSPlatformConfig>();
  default:
    E2D_LOG_WARN("Unknown platform type, defaulting to Windows");
    return makeUnique<WindowsPlatformConfig>();
  }
}

/**
 * @brief 获取平台类型名称
 * 将 PlatformType 枚举转换为可读的字符串
 * @param type 平台类型枚举值
 * @return 平台名称字符串
 */
const char *getPlatformTypeName(PlatformType type) {
  switch (type) {
  case PlatformType::Auto:
    return "Auto";
  case PlatformType::Windows:
    return "Windows";
  case PlatformType::Switch:
    return "Switch";
  case PlatformType::Linux:
    return "Linux";
  case PlatformType::macOS:
    return "macOS";
  default:
    return "Unknown";
  }
}

} // namespace extra2d
