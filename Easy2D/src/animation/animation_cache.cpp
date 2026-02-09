#include <easy2d/animation/animation_cache.h>
#include <easy2d/animation/ani_parser.h>
#include <easy2d/animation/ani_binary_parser.h>
#include <easy2d/utils/logger.h>
#include <fstream>

namespace easy2d {

namespace {

// 检测文件是否为文本格式 ANI
// 文本格式以 '#' 或 '[' 开头（跳过空白后）
bool isTextFormat(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return false;

    // 读取前几个字节判断
    char buf[16] = {};
    file.read(buf, sizeof(buf));
    auto bytesRead = file.gcount();
    file.close();

    // 跳过 BOM 和空白
    size_t pos = 0;
    // UTF-8 BOM
    if (bytesRead >= 3 &&
        static_cast<uint8_t>(buf[0]) == 0xEF &&
        static_cast<uint8_t>(buf[1]) == 0xBB &&
        static_cast<uint8_t>(buf[2]) == 0xBF) {
        pos = 3;
    }

    // 跳过空白
    while (pos < static_cast<size_t>(bytesRead) &&
           (buf[pos] == ' ' || buf[pos] == '\t' ||
            buf[pos] == '\r' || buf[pos] == '\n')) {
        ++pos;
    }

    if (pos >= static_cast<size_t>(bytesRead)) return false;

    // 文本 ANI 文件以 '#' (注释/PVF_File) 或 '[' (标签) 开头
    return buf[pos] == '#' || buf[pos] == '[';
}

} // anonymous namespace

Ptr<AnimationClip> AnimationCache::loadClip(const std::string& aniFilePath) {
    // 先检查缓存
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = clips_.find(aniFilePath);
        if (it != clips_.end()) {
            return it->second;
        }
    }

    // 提取基础路径
    std::string basePath;
    auto lastSlash = aniFilePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        basePath = aniFilePath.substr(0, lastSlash);
    }

    AniParseResult result;

    if (isTextFormat(aniFilePath)) {
        // 文本格式
        AniParser parser;
        if (pathResolver_) {
            parser.setPathResolver(pathResolver_);
        }
        if (!basePath.empty()) {
            parser.setBasePath(basePath);
        }
        result = parser.parse(aniFilePath);
    } else {
        // 二进制格式
        AniBinaryParser parser;
        if (pathResolver_) {
            parser.setPathResolver(pathResolver_);
        }
        if (!basePath.empty()) {
            parser.setBasePath(basePath);
        }
        result = parser.parseFromFile(aniFilePath);
    }

    if (!result.success || !result.clip) {
        return nullptr;
    }

    // 添加到缓存
    {
        std::lock_guard<std::mutex> lock(mutex_);
        clips_[aniFilePath] = result.clip;
    }

    return result.clip;
}

} // namespace easy2d
