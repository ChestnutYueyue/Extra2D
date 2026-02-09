#include <easy2d/animation/ani_parser.h>
#include <easy2d/animation/sprite_frame_cache.h>
#include <easy2d/utils/logger.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace easy2d {

namespace {

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string stripBackticks(const std::string& s) {
    std::string t = trim(s);
    if (t.size() >= 2 && t.front() == '`' && t.back() == '`') {
        return t.substr(1, t.size() - 2);
    }
    return t;
}

std::vector<std::string> splitWhitespace(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool lineStartsWith(const std::string& trimmed, const std::string& tag) {
    return trimmed.find(tag) == 0;
}

std::string valueAfterTag(const std::string& trimmed, const std::string& tag) {
    auto pos = trimmed.find(tag);
    if (pos == std::string::npos) return "";
    return trim(trimmed.substr(pos + tag.size()));
}

// 读取标签后的值，如果同行没有则从流中读取下一行
std::string readValue(const std::string& trimmed, const std::string& tag,
                      std::istringstream& stream) {
    std::string val = valueAfterTag(trimmed, tag);
    if (val.empty()) {
        std::string nextLine;
        if (std::getline(stream, nextLine)) {
            val = trim(nextLine);
        }
    }
    return val;
}

} // anonymous namespace

AniParseResult AniParser::parse(const std::string& filePath) {
    AniParseResult result;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.success = false;
        result.errorMessage = "Cannot open ANI file: " + filePath;
        return result;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    // 提取基础路径
    auto lastSlash = filePath.find_last_of("/\\");
    std::string basePath;
    if (lastSlash != std::string::npos) {
        basePath = filePath.substr(0, lastSlash);
    }

    if (basePath_.empty() && !basePath.empty()) {
        basePath_ = basePath;
    }

    result = parseFromMemory(content, basePath_.empty() ? basePath : basePath_);

    if (result.clip) {
        result.clip->setSourcePath(filePath);
        std::string name = (lastSlash != std::string::npos)
            ? filePath.substr(lastSlash + 1)
            : filePath;
        result.clip->setName(name);
    }

    return result;
}

AniParseResult AniParser::parseFromMemory(const std::string& content,
                                            const std::string& basePath) {
    AniParseResult result;
    result.clip = AnimationClip::create();

    if (!basePath.empty() && basePath_.empty()) {
        basePath_ = basePath;
    }

    std::istringstream stream(content);
    std::string line;

    AnimationFrame currentFrame;
    bool inFrame = false;
    bool hasCurrentFrame = false;

    // [IMAGE] 标签的多行读取状态
    bool pendingImage = false;
    std::string imagePath;

    auto finalizeFrame = [&]() {
        if (!hasCurrentFrame) return;
        if (!currentFrame.texturePath.empty()) {
            std::string resolvedPath = resolvePath(currentFrame.texturePath);
            auto spriteFrame = SpriteFrameCache::getInstance()
                .getOrCreateFromFile(resolvedPath, currentFrame.textureIndex);
            currentFrame.spriteFrame = spriteFrame;
        }
        result.clip->addFrame(std::move(currentFrame));
        currentFrame = AnimationFrame();
        hasCurrentFrame = false;
    };

    while (std::getline(stream, line)) {
        std::string trimmed = trim(line);

        if (trimmed.empty()) continue;
        if (trimmed[0] == '#') continue;

        // [FRAME MAX]
        if (lineStartsWith(trimmed, "[FRAME MAX]")) {
            readValue(trimmed, "[FRAME MAX]", stream);
            continue;
        }

        // [FRAMEXXX] - 新帧开始
        if (trimmed.size() >= 7 && trimmed.substr(0, 6) == "[FRAME" &&
            trimmed.back() == ']' && !lineStartsWith(trimmed, "[FRAME MAX]")) {
            finalizeFrame();
            currentFrame = AnimationFrame();
            inFrame = true;
            hasCurrentFrame = true;
            pendingImage = false;
            continue;
        }

        if (!inFrame) {
            // 全局属性
            if (lineStartsWith(trimmed, "[LOOP]")) {
                result.clip->setLooping(true);
            } else if (lineStartsWith(trimmed, "[SHADOW]")) {
                result.clip->globalProperties().set(FramePropertyKey::Shadow, true);
            }
            continue;
        }

        // === 帧内属性解析 ===

        // [IMAGE] - 图片路径和索引（跨行）
        if (lineStartsWith(trimmed, "[IMAGE]")) {
            std::string val = valueAfterTag(trimmed, "[IMAGE]");
            if (!val.empty()) {
                auto tokens = splitWhitespace(val);
                imagePath = stripBackticks(tokens[0]);
                if (tokens.size() >= 2) {
                    currentFrame.texturePath = imagePath;
                    currentFrame.textureIndex = std::stoi(tokens[1]);
                    pendingImage = false;
                } else {
                    pendingImage = true;
                }
            } else {
                pendingImage = true;
                imagePath.clear();
            }
            continue;
        }

        // [IMAGE] 后续行
        if (pendingImage) {
            if (imagePath.empty()) {
                imagePath = stripBackticks(trimmed);
            } else {
                currentFrame.texturePath = imagePath;
                currentFrame.textureIndex = std::stoi(trimmed);
                pendingImage = false;
            }
            continue;
        }

        // [IMAGE POS]
        if (lineStartsWith(trimmed, "[IMAGE POS]")) {
            std::string val = readValue(trimmed, "[IMAGE POS]", stream);
            auto tokens = splitWhitespace(val);
            if (tokens.size() >= 2) {
                currentFrame.offset = Vec2(
                    static_cast<float>(std::stoi(tokens[0])),
                    static_cast<float>(std::stoi(tokens[1]))
                );
            }
            continue;
        }

        // [DELAY]
        if (lineStartsWith(trimmed, "[DELAY]")) {
            std::string val = readValue(trimmed, "[DELAY]", stream);
            currentFrame.delay = static_cast<float>(std::stoi(trim(val)));
            continue;
        }

        // [DAMAGE TYPE]
        if (lineStartsWith(trimmed, "[DAMAGE TYPE]")) {
            std::string val = readValue(trimmed, "[DAMAGE TYPE]", stream);
            val = stripBackticks(val);
            int damageType = 0;
            if (val == "SUPERARMOR") damageType = 1;
            else if (val == "UNBREAKABLE") damageType = 2;
            currentFrame.properties.set(FramePropertyKey::DamageType, damageType);
            continue;
        }

        // [DAMAGE BOX]
        if (lineStartsWith(trimmed, "[DAMAGE BOX]")) {
            std::string val = readValue(trimmed, "[DAMAGE BOX]", stream);
            auto tokens = splitWhitespace(val);
            if (tokens.size() >= 6) {
                std::array<int32_t, 6> box;
                for (int i = 0; i < 6; ++i) {
                    box[i] = std::stoi(tokens[i]);
                }
                currentFrame.damageBoxes.push_back(box);
            }
            continue;
        }

        // [ATTACK BOX]
        if (lineStartsWith(trimmed, "[ATTACK BOX]")) {
            std::string val = readValue(trimmed, "[ATTACK BOX]", stream);
            auto tokens = splitWhitespace(val);
            if (tokens.size() >= 6) {
                std::array<int32_t, 6> box;
                for (int i = 0; i < 6; ++i) {
                    box[i] = std::stoi(tokens[i]);
                }
                currentFrame.attackBoxes.push_back(box);
            }
            continue;
        }

        // [SET FLAG]
        if (lineStartsWith(trimmed, "[SET FLAG]")) {
            std::string val = readValue(trimmed, "[SET FLAG]", stream);
            currentFrame.properties.withSetFlag(std::stoi(trim(val)));
            continue;
        }

        // [PLAY SOUND]
        if (lineStartsWith(trimmed, "[PLAY SOUND]")) {
            std::string val = readValue(trimmed, "[PLAY SOUND]", stream);
            currentFrame.properties.withPlaySound(resolvePath(stripBackticks(val)));
            continue;
        }

        // [IMAGE RATE]
        if (lineStartsWith(trimmed, "[IMAGE RATE]")) {
            std::string val = readValue(trimmed, "[IMAGE RATE]", stream);
            auto tokens = splitWhitespace(val);
            if (tokens.size() >= 2) {
                currentFrame.properties.withImageRate(Vec2(
                    std::stof(tokens[0]), std::stof(tokens[1])
                ));
            }
            continue;
        }

        // [IMAGE ROTATE]
        if (lineStartsWith(trimmed, "[IMAGE ROTATE]")) {
            std::string val = readValue(trimmed, "[IMAGE ROTATE]", stream);
            currentFrame.properties.withImageRotate(std::stof(trim(val)));
            continue;
        }

        // [RGBA]
        if (lineStartsWith(trimmed, "[RGBA]")) {
            std::string val = readValue(trimmed, "[RGBA]", stream);
            auto tokens = splitWhitespace(val);
            if (tokens.size() >= 4) {
                Color color = Color::fromRGBA(
                    static_cast<uint8_t>(std::stoi(tokens[0])),
                    static_cast<uint8_t>(std::stoi(tokens[1])),
                    static_cast<uint8_t>(std::stoi(tokens[2])),
                    static_cast<uint8_t>(std::stoi(tokens[3]))
                );
                currentFrame.properties.withColorTint(color);
            }
            continue;
        }

        // [INTERPOLATION]
        if (lineStartsWith(trimmed, "[INTERPOLATION]")) {
            currentFrame.properties.withInterpolation(true);
            continue;
        }

        // [LOOP]
        if (lineStartsWith(trimmed, "[LOOP]")) {
            currentFrame.properties.set(FramePropertyKey::Loop, true);
            continue;
        }

        // [SHADOW]
        if (lineStartsWith(trimmed, "[SHADOW]")) {
            currentFrame.properties.set(FramePropertyKey::Shadow, true);
            continue;
        }

        // [FLIP TYPE]
        if (lineStartsWith(trimmed, "[FLIP TYPE]")) {
            std::string val = readValue(trimmed, "[FLIP TYPE]", stream);
            val = stripBackticks(val);
            int flipType = 0;
            if (val == "HORIZON") flipType = 1;
            else if (val == "VERTICAL") flipType = 2;
            else if (val == "ALL") flipType = 3;
            else flipType = std::stoi(val);
            currentFrame.properties.set(FramePropertyKey::FlipType, flipType);
            continue;
        }

        // [COORD]
        if (lineStartsWith(trimmed, "[COORD]")) {
            std::string val = readValue(trimmed, "[COORD]", stream);
            currentFrame.properties.set(FramePropertyKey::Coord, std::stoi(trim(val)));
            continue;
        }

        // [GRAPHIC EFFECT]
        if (lineStartsWith(trimmed, "[GRAPHIC EFFECT]")) {
            std::string val = readValue(trimmed, "[GRAPHIC EFFECT]", stream);
            val = stripBackticks(val);
            int effectType = 0;
            if (val == "DODGE") effectType = 1;
            else if (val == "LINEARDODGE") effectType = 2;
            else if (val == "DARK") effectType = 3;
            else if (val == "XOR") effectType = 4;
            else if (val == "MONOCHROME") effectType = 5;
            else if (val == "SPACEDISTORT") effectType = 6;
            else effectType = std::stoi(val);
            currentFrame.properties.set(FramePropertyKey::GraphicEffect, effectType);
            continue;
        }

        // [CLIP]
        if (lineStartsWith(trimmed, "[CLIP]")) {
            std::string val = readValue(trimmed, "[CLIP]", stream);
            auto tokens = splitWhitespace(val);
            if (tokens.size() >= 4) {
                std::vector<int> clipRegion = {
                    std::stoi(tokens[0]), std::stoi(tokens[1]),
                    std::stoi(tokens[2]), std::stoi(tokens[3])
                };
                currentFrame.properties.set(FramePropertyKey::ClipRegion, clipRegion);
            }
            continue;
        }

        // [LOOP START]
        if (lineStartsWith(trimmed, "[LOOP START]")) {
            currentFrame.properties.set(FramePropertyKey::LoopStart, true);
            continue;
        }

        // [LOOP END]
        if (lineStartsWith(trimmed, "[LOOP END]")) {
            std::string val = readValue(trimmed, "[LOOP END]", stream);
            currentFrame.properties.set(FramePropertyKey::LoopEnd, std::stoi(trim(val)));
            continue;
        }

        // 未识别标签 - 静默忽略
    }

    // 保存最后一帧
    finalizeFrame();

    result.success = true;
    return result;
}

std::string AniParser::resolvePath(const std::string& relativePath) const {
    std::string resolved = relativePath;
    if (pathResolver_) {
        resolved = pathResolver_(relativePath);
    }

    if (!basePath_.empty() && !resolved.empty() && resolved[0] != '/') {
        if (resolved.size() < 2 || resolved[1] != ':') {
            resolved = basePath_ + "/" + resolved;
        }
    }

    return resolved;
}

} // namespace easy2d
