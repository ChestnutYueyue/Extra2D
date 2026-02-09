#include <easy2d/animation/ani_binary_parser.h>
#include <easy2d/animation/sprite_frame_cache.h>
#include <easy2d/utils/logger.h>
#include <fstream>
#include <cstring>
#include <cassert>

namespace easy2d {

namespace {

// 简易二进制缓冲区读取器
class BufferReader {
public:
    BufferReader(const uint8_t* data, size_t length)
        : data_(data), length_(length), pos_(0) {}

    template<typename T>
    T read() {
        if (pos_ + sizeof(T) > length_) {
            return T{};
        }
        T value;
        std::memcpy(&value, data_ + pos_, sizeof(T));
        pos_ += sizeof(T);
        return value;
    }

    std::string readAsciiString(int32_t len) {
        if (len <= 0 || pos_ + static_cast<size_t>(len) > length_) {
            return "";
        }
        std::string result(reinterpret_cast<const char*>(data_ + pos_), len);
        pos_ += len;
        return result;
    }

    bool hasRemaining() const { return pos_ < length_; }
    size_t remaining() const { return length_ - pos_; }

private:
    const uint8_t* data_;
    size_t length_;
    size_t pos_;
};

// 字符串转小写
void toLower(std::string& s) {
    for (auto& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
}

} // anonymous namespace

AniParseResult AniBinaryParser::parse(const uint8_t* data, size_t length) {
    AniParseResult result;
    result.clip = AnimationClip::create();

    if (!data || length < 4) {
        result.success = false;
        result.errorMessage = "Invalid binary ANI data";
        return result;
    }

    BufferReader reader(data, length);

    // 读取帧数和资源数
    uint16_t frameCount = reader.read<uint16_t>();
    uint16_t resourceCount = reader.read<uint16_t>();

    // 读取精灵路径列表
    std::vector<std::string> sprites;
    sprites.reserve(resourceCount);
    for (uint16_t i = 0; i < resourceCount; ++i) {
        int32_t len = reader.read<int32_t>();
        std::string path = reader.readAsciiString(len);
        toLower(path);
        sprites.push_back(std::move(path));
    }

    // 读取全局参数
    uint16_t globalParamCount = reader.read<uint16_t>();
    for (uint16_t j = 0; j < globalParamCount; ++j) {
        uint16_t type = reader.read<uint16_t>();
        switch (type) {
        case static_cast<uint16_t>(AniNodeType::Loop):
            if (reader.read<int8_t>()) {
                result.clip->setLooping(true);
            }
            break;
        case static_cast<uint16_t>(AniNodeType::Shadow):
            if (reader.read<int8_t>()) {
                result.clip->globalProperties().set(FramePropertyKey::Shadow, true);
            }
            break;
        default:
            break;
        }
    }

    // 逐帧解析
    for (uint16_t i = 0; i < frameCount; ++i) {
        AnimationFrame frame;

        // 碰撞盒
        uint16_t boxCount = reader.read<uint16_t>();
        for (uint16_t j = 0; j < boxCount; ++j) {
            uint16_t boxType = reader.read<uint16_t>();
            std::array<int32_t, 6> box;
            for (int m = 0; m < 6; ++m) {
                box[m] = reader.read<int32_t>();
            }
            if (boxType == static_cast<uint16_t>(AniNodeType::DamageBox)) {
                frame.damageBoxes.push_back(box);
            } else if (boxType == static_cast<uint16_t>(AniNodeType::AttackBox)) {
                frame.attackBoxes.push_back(box);
            }
        }

        // 图片 ID 和参数
        uint16_t imgId = reader.read<uint16_t>();
        uint16_t imgParam = reader.read<uint16_t>();
        (void)imgParam;

        if (imgId < sprites.size()) {
            frame.texturePath = sprites[imgId];
            frame.textureIndex = imgId;

            std::string resolvedPath = resolvePath(frame.texturePath);
            auto spriteFrame = SpriteFrameCache::getInstance()
                .getOrCreateFromFile(resolvedPath, frame.textureIndex);
            frame.spriteFrame = spriteFrame;
        }

        // 位置
        frame.offset = Vec2(
            static_cast<float>(reader.read<int32_t>()),
            static_cast<float>(reader.read<int32_t>())
        );

        // 帧属性
        uint16_t propertyCount = reader.read<uint16_t>();
        for (uint16_t m = 0; m < propertyCount; ++m) {
            uint16_t propType = reader.read<uint16_t>();
            AniNodeType nodeType = static_cast<AniNodeType>(propType);

            switch (nodeType) {
            case AniNodeType::Loop:
                frame.properties.set(FramePropertyKey::Loop,
                    static_cast<bool>(reader.read<int8_t>()));
                break;

            case AniNodeType::Shadow:
                frame.properties.set(FramePropertyKey::Shadow,
                    static_cast<bool>(reader.read<int8_t>()));
                break;

            case AniNodeType::Interpolation:
                frame.properties.withInterpolation(
                    static_cast<bool>(reader.read<int8_t>()));
                break;

            case AniNodeType::Coord:
                frame.properties.set(FramePropertyKey::Coord,
                    static_cast<int>(reader.read<uint16_t>()));
                break;

            case AniNodeType::ImageRate: {
                float rateX = reader.read<float>();
                float rateY = reader.read<float>();
                frame.properties.withImageRate(Vec2(rateX, rateY));
                break;
            }

            case AniNodeType::ImageRotate:
                frame.properties.withImageRotate(
                    static_cast<float>(reader.read<int32_t>()));
                break;

            case AniNodeType::RGBA: {
                uint32_t rgba = reader.read<uint32_t>();
                uint8_t a = static_cast<uint8_t>((rgba >> 24) & 0xFF);
                uint8_t r = static_cast<uint8_t>((rgba >> 16) & 0xFF);
                uint8_t g = static_cast<uint8_t>((rgba >> 8)  & 0xFF);
                uint8_t b = static_cast<uint8_t>((rgba)       & 0xFF);
                frame.properties.withColorTint(Color::fromRGBA(r, g, b, a));
                break;
            }

            case AniNodeType::GraphicEffect: {
                uint16_t effectType = reader.read<uint16_t>();
                frame.properties.set(FramePropertyKey::GraphicEffect,
                    static_cast<int>(effectType));
                // MONOCHROME 额外读取 rgb
                if (effectType == 5) {
                    reader.read<uint8_t>(); // r
                    reader.read<uint8_t>(); // g
                    reader.read<uint8_t>(); // b
                }
                // SPACEDISTORT 额外读取 pos
                else if (effectType == 6) {
                    reader.read<uint16_t>(); // x
                    reader.read<uint16_t>(); // y
                }
                break;
            }

            case AniNodeType::Delay:
                frame.delay = static_cast<float>(reader.read<int32_t>());
                break;

            case AniNodeType::DamageType:
                frame.properties.set(FramePropertyKey::DamageType,
                    static_cast<int>(reader.read<uint16_t>()));
                break;

            case AniNodeType::PlaySound: {
                int32_t len = reader.read<int32_t>();
                std::string soundPath = reader.readAsciiString(len);
                frame.properties.withPlaySound(resolvePath(soundPath));
                break;
            }

            case AniNodeType::SetFlag:
                frame.properties.withSetFlag(reader.read<int32_t>());
                break;

            case AniNodeType::FlipType:
                frame.properties.set(FramePropertyKey::FlipType,
                    static_cast<int>(reader.read<uint16_t>()));
                break;

            case AniNodeType::LoopStart:
                frame.properties.set(FramePropertyKey::LoopStart, true);
                break;

            case AniNodeType::LoopEnd:
                frame.properties.set(FramePropertyKey::LoopEnd,
                    reader.read<int32_t>());
                break;

            case AniNodeType::Clip: {
                std::vector<int> clipRegion = {
                    static_cast<int>(reader.read<int16_t>()),
                    static_cast<int>(reader.read<int16_t>()),
                    static_cast<int>(reader.read<int16_t>()),
                    static_cast<int>(reader.read<int16_t>())
                };
                frame.properties.set(FramePropertyKey::ClipRegion, clipRegion);
                break;
            }

            default:
                // 未知类型 - 跳过
                break;
            }
        }

        result.clip->addFrame(std::move(frame));
    }

    result.success = true;
    return result;
}

AniParseResult AniBinaryParser::parseFromFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        AniParseResult result;
        result.success = false;
        result.errorMessage = "Cannot open binary ANI file: " + filePath;
        return result;
    }

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    file.close();

    // 提取基础路径
    auto lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos && basePath_.empty()) {
        basePath_ = filePath.substr(0, lastSlash);
    }

    auto result = parse(buffer.data(), buffer.size());

    if (result.clip) {
        result.clip->setSourcePath(filePath);
        std::string name = (lastSlash != std::string::npos)
            ? filePath.substr(lastSlash + 1)
            : filePath;
        result.clip->setName(name);
    }

    return result;
}

std::string AniBinaryParser::resolvePath(const std::string& relativePath) const {
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
