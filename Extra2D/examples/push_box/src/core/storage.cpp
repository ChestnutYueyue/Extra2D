#include "storage.h"

#include <extra2d/utils/data.h>
#include <fstream>
#include <sstream>
#include <string>

namespace pushbox {

static extra2d::DataStore g_store;
static std::filesystem::path g_filePath;
static bool g_loaded = false;

// 默认配置内容
static const char *DEFAULT_CONFIG = R"([game]
level = 1
sound = true

[best]
)";

/**
 * @brief 从 romfs 加载默认配置文件
 * @return 配置文件内容，如果失败则返回空字符串
 * @note Switch 平台上 romfs 路径格式为 romfs:/pushbox.ini
 */
static std::string loadDefaultConfigFromRomfs() {
  // 尝试多个可能的路径（按优先级排序）
  const char *paths[] = {
      "romfs:/pushbox.ini", // Switch romfs 正确路径格式
      "romfs/pushbox.ini",  // 开发环境相对路径
      "pushbox.ini",        // 当前目录
  };

  for (const char *path : paths) {
    if (std::filesystem::exists(path)) {
      std::ifstream file(path, std::ios::binary);
      if (file) {
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
      }
    }
  }

  // 如果找不到文件，返回内置的默认配置
  return DEFAULT_CONFIG;
}

/**
 * @brief 将配置字符串写入临时文件并加载
 * @param content 配置内容
 * @return 临时文件路径
 */
static std::filesystem::path writeConfigToTempFile(const std::string &content) {
  auto tempPath =
      std::filesystem::temp_directory_path() / "pushbox_default.ini";
  std::ofstream file(tempPath, std::ios::binary);
  if (file) {
    file << content;
    file.close();
  }
  return tempPath;
}

static void ensureLoaded() {
  if (g_loaded) {
    return;
  }

  // 首先尝试从可执行目录加载用户配置
  if (!g_filePath.empty() && std::filesystem::exists(g_filePath)) {
    g_store.load(g_filePath.string());
  } else {
    // 从 romfs 加载默认配置
    std::string defaultConfig = loadDefaultConfigFromRomfs();
    if (!defaultConfig.empty()) {
      auto tempPath = writeConfigToTempFile(defaultConfig);
      g_store.load(tempPath.string());
    }
  }
  g_loaded = true;
}

void initStorage(const std::filesystem::path &baseDir) {
  g_filePath = baseDir / "pushbox.ini";

  // 首先尝试从可执行目录加载用户配置
  if (std::filesystem::exists(g_filePath)) {
    g_store.load(g_filePath.string());
  } else {
    // 从 romfs 加载默认配置
    std::string defaultConfig = loadDefaultConfigFromRomfs();
    if (!defaultConfig.empty()) {
      auto tempPath = writeConfigToTempFile(defaultConfig);
      g_store.load(tempPath.string());
    }
  }
  g_loaded = true;
}

int loadCurrentLevel(int defaultValue) {
  ensureLoaded();
  int level = g_store.getInt("game", "level", defaultValue);
  if (level < 1) {
    level = 1;
  }
  return level;
}

void saveCurrentLevel(int level) {
  ensureLoaded();
  g_store.setInt("game", "level", level);
  if (!g_filePath.empty()) {
    g_store.save(g_filePath.string());
  }
}

bool loadSoundOpen(bool defaultValue) {
  ensureLoaded();
  return g_store.getBool("game", "sound", defaultValue);
}

void saveSoundOpen(bool open) {
  ensureLoaded();
  g_store.setBool("game", "sound", open);
  if (!g_filePath.empty()) {
    g_store.save(g_filePath.string());
  }
}

int loadBestStep(int level, int defaultValue) {
  ensureLoaded();
  std::string key = "level" + std::to_string(level);
  return g_store.getInt("best", key, defaultValue);
}

void saveBestStep(int level, int step) {
  ensureLoaded();
  std::string key = "level" + std::to_string(level);
  g_store.setInt("best", key, step);
  if (!g_filePath.empty()) {
    g_store.save(g_filePath.string());
  }
}

std::filesystem::path storageFilePath() { return g_filePath; }

} // namespace pushbox
