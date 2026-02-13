// ============================================================================
// ResLoader.cpp - 资源加载器实现
// ============================================================================

#include "ResLoader.h"
#include <json/json.hpp>

namespace flappybird {

extra2d::Ptr<extra2d::Texture> ResLoader::atlasTexture_;
std::map<std::string, ResLoader::ImageInfo> ResLoader::imageMap_;
std::map<MusicType, extra2d::Ptr<extra2d::Sound>> ResLoader::soundMap_;

void ResLoader::init() {
  auto &resources = extra2d::Application::instance().resources();

  // 加载图集纹理
  atlasTexture_ = resources.loadTexture("assets/images/atlas.png");
  if (!atlasTexture_) {
    E2D_LOG_ERROR("无法加载图集纹理 atlas.png");
    return;
  }

  // 使用资源管理器加载 JSON 文件
  std::string jsonContent = resources.loadJsonFile("assets/images/atlas.json");
  if (jsonContent.empty()) {
    E2D_LOG_ERROR("无法加载 atlas.json 文件");
    return;
  }

  // 解析 JSON 图集数据
  try {
    nlohmann::json jsonData = nlohmann::json::parse(jsonContent);

    for (const auto &sprite : jsonData["sprites"]) {
      std::string name = sprite["name"];
      float x = sprite["x"];
      float y = sprite["y"];
      float width = sprite["width"];
      float height = sprite["height"];

      ImageInfo info = {width, height, x, y};
      imageMap_[name] = info;
    }

    E2D_LOG_INFO("成功加载 {} 个精灵帧", imageMap_.size());
  } catch (const std::exception &e) {
    E2D_LOG_ERROR("解析 atlas.json 失败: {}", e.what());
    return;
  }

  // 加载音效
  soundMap_[MusicType::Click] = resources.loadSound("assets/sound/click.wav");
  soundMap_[MusicType::Hit] = resources.loadSound("assets/sound/hit.wav");
  soundMap_[MusicType::Fly] = resources.loadSound("assets/sound/fly.wav");
  soundMap_[MusicType::Point] = resources.loadSound("assets/sound/point.wav");
  soundMap_[MusicType::Swoosh] = resources.loadSound("assets/sound/swoosh.wav");

  E2D_LOG_INFO("资源加载完成");
}

extra2d::Ptr<extra2d::SpriteFrame>
ResLoader::getKeyFrame(const std::string &name) {
  auto it = imageMap_.find(name);
  if (it == imageMap_.end()) {
    E2D_LOG_WARN("找不到精灵帧: %s", name.c_str());
    return nullptr;
  }

  const ImageInfo &info = it->second;
  E2D_LOG_INFO("加载精灵帧: name={}, w={}, h={}, x={}, y={}", name, info.width,
               info.height, info.x, info.y);

  // 检查纹理尺寸
  if (atlasTexture_) {
    E2D_LOG_INFO("图集纹理尺寸: {}x{}", atlasTexture_->getWidth(),
                 atlasTexture_->getHeight());
  }

  return extra2d::makePtr<extra2d::SpriteFrame>(
      atlasTexture_, extra2d::Rect(info.x, info.y, info.width, info.height));
}

void ResLoader::playMusic(MusicType type) {
  auto it = soundMap_.find(type);
  if (it == soundMap_.end()) {
    E2D_LOG_WARN("ResLoader::playMusic: sound type not found");
    return;
  }
  if (!it->second) {
    E2D_LOG_WARN("ResLoader::playMusic: sound pointer is null");
    return;
  }
  if (!it->second->play()) {
    E2D_LOG_WARN("ResLoader::playMusic: failed to play sound");
  }
}

} // namespace flappybird
