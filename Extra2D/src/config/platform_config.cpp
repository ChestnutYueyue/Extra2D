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

  int getRecommendedWidth() const override { return 1920; }
  int getRecommendedHeight() const override { return 1080; }
  bool isResolutionSupported(int width, int height) const override {
    return width >= 320 && height >= 240;
  }

private:
  PlatformCapabilities caps_;
};

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

  int getRecommendedWidth() const override { return 1920; }
  int getRecommendedHeight() const override { return 1080; }
  bool isResolutionSupported(int width, int height) const override {
    return (width == 1920 && height == 1080) ||
           (width == 1280 && height == 720);
  }

private:
  PlatformCapabilities caps_;
};

} 

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

} 
