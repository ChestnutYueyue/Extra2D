#include <extra2d/graphics/opengl/gl_texture.h>
#include <extra2d/graphics/gpu_context.h>
#include <extra2d/graphics/vram_manager.h>
#define STB_IMAGE_IMPLEMENTATION
#include <cstring>
#include <extra2d/utils/logger.h>
#include <fstream>
#include <stb/stb_image.h>

namespace extra2d {

// ============================================================================
// KTX 文件头结构
// ============================================================================
#pragma pack(push, 1)
struct KTXHeader {
  uint8_t identifier[12];
  uint32_t endianness;
  uint32_t glType;
  uint32_t glTypeSize;
  uint32_t glFormat;
  uint32_t glInternalFormat;
  uint32_t glBaseInternalFormat;
  uint32_t pixelWidth;
  uint32_t pixelHeight;
  uint32_t pixelDepth;
  uint32_t numberOfArrayElements;
  uint32_t numberOfFaces;
  uint32_t numberOfMipmapLevels;
  uint32_t bytesOfKeyValueData;
};
#pragma pack(pop)

// KTX 文件标识符
static const uint8_t KTX_IDENTIFIER[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31,
                                           0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};

// ============================================================================
// DDS 文件头结构
// ============================================================================
#pragma pack(push, 1)
struct DDSPixelFormat {
  uint32_t size;
  uint32_t flags;
  uint32_t fourCC;
  uint32_t rgbBitCount;
  uint32_t rBitMask;
  uint32_t gBitMask;
  uint32_t bBitMask;
  uint32_t aBitMask;
};

struct DDSHeader {
  uint32_t magic;
  uint32_t size;
  uint32_t flags;
  uint32_t height;
  uint32_t width;
  uint32_t pitchOrLinearSize;
  uint32_t depth;
  uint32_t mipMapCount;
  uint32_t reserved1[11];
  DDSPixelFormat pixelFormat;
  uint32_t caps;
  uint32_t caps2;
  uint32_t caps3;
  uint32_t caps4;
  uint32_t reserved2;
};

struct DDSHeaderDXT10 {
  uint32_t dxgiFormat;
  uint32_t resourceDimension;
  uint32_t miscFlag;
  uint32_t arraySize;
  uint32_t miscFlags2;
};
#pragma pack(pop)

static constexpr uint32_t DDS_MAGIC = 0x20534444; // "DDS "
static constexpr uint32_t DDPF_FOURCC = 0x04;

static uint32_t makeFourCC(char a, char b, char c, char d) {
  return static_cast<uint32_t>(a) | (static_cast<uint32_t>(b) << 8) |
         (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24);
}

// ============================================================================
// GLTexture 实现
// ============================================================================

GLTexture::GLTexture(int width, int height, const uint8_t *pixels, int channels)
    : textureID_(0), width_(width), height_(height), channels_(channels),
      format_(PixelFormat::RGBA8), dataSize_(0) {
  // 保存像素数据用于生成遮罩
  if (pixels) {
    pixelData_.resize(width * height * channels);
    std::memcpy(pixelData_.data(), pixels, pixelData_.size());
  }
  createTexture(pixels);
}

GLTexture::GLTexture(const std::string &filepath)
    : textureID_(0), width_(0), height_(0), channels_(0),
      format_(PixelFormat::RGBA8), dataSize_(0) {
  // 检查是否为压缩纹理格式
  std::string ext = filepath.substr(filepath.find_last_of('.') + 1);
  if (ext == "ktx" || ext == "KTX") {
    loadCompressed(filepath);
    return;
  }
  if (ext == "dds" || ext == "DDS") {
    loadCompressed(filepath);
    return;
  }

  // 不翻转图片，保持原始方向
  stbi_set_flip_vertically_on_load(false);
  uint8_t *data = stbi_load(filepath.c_str(), &width_, &height_, &channels_, 0);
  if (data) {
    // 保存像素数据用于生成遮罩
    pixelData_.resize(width_ * height_ * channels_);
    std::memcpy(pixelData_.data(), data, pixelData_.size());

    createTexture(data);
    stbi_image_free(data);
  } else {
    E2D_LOG_ERROR("Failed to load texture: {}", filepath);
  }
}

GLTexture::~GLTexture() {
  if (textureID_ != 0) {
    // 检查 GPU 上下文是否仍然有效
    // 如果 OpenGL 上下文已销毁，则跳过 glDeleteTextures 调用
    if (GPUContext::getInstance().isValid()) {
      glDeleteTextures(1, &textureID_);
    }
    // VRAM 跟踪: 释放纹理显存（无论上下文是否有效都需要更新统计）
    if (dataSize_ > 0) {
      VRAMManager::getInstance().freeTexture(dataSize_);
    }
  }
}

void GLTexture::setFilter(bool linear) {
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  linear ? GL_LINEAR : GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  linear ? GL_LINEAR : GL_NEAREST);
}

void GLTexture::setWrap(bool repeat) {
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
}

void GLTexture::bind(unsigned int slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, textureID_);
}

void GLTexture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

void GLTexture::createTexture(const uint8_t *pixels) {
  GLenum format = GL_RGBA;
  GLenum internalFormat = GL_RGBA8;
  int unpackAlignment = 4;
  if (channels_ == 1) {
    format = GL_RED;
    internalFormat = GL_R8;
    unpackAlignment = 1;
    format_ = PixelFormat::R8;
  } else if (channels_ == 3) {
    format = GL_RGB;
    internalFormat = GL_RGB8;
    unpackAlignment = 1;
    format_ = PixelFormat::RGB8;
  } else if (channels_ == 4) {
    format = GL_RGBA;
    internalFormat = GL_RGBA8;
    unpackAlignment = 4;
    format_ = PixelFormat::RGBA8;
  }

  glGenTextures(1, &textureID_);
  bind();

  GLint prevUnpackAlignment = 4;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevUnpackAlignment);
  glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width_, height_, 0, format,
               GL_UNSIGNED_BYTE, pixels);
  glPixelStorei(GL_UNPACK_ALIGNMENT, prevUnpackAlignment);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // 使用 NEAREST 过滤器，更适合像素艺术风格的精灵
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenerateMipmap(GL_TEXTURE_2D);

  // VRAM 跟踪
  dataSize_ = static_cast<size_t>(width_ * height_ * channels_);
  VRAMManager::getInstance().allocTexture(dataSize_);
}

// ============================================================================
// 压缩纹理加载
// ============================================================================

bool GLTexture::loadCompressed(const std::string &filepath) {
  std::string ext = filepath.substr(filepath.find_last_of('.') + 1);
  if (ext == "ktx" || ext == "KTX") {
    return loadKTX(filepath);
  }
  if (ext == "dds" || ext == "DDS") {
    return loadDDS(filepath);
  }
  E2D_LOG_ERROR("Unsupported compressed texture format: {}", filepath);
  return false;
}

bool GLTexture::loadKTX(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    E2D_LOG_ERROR("Failed to open KTX file: {}", filepath);
    return false;
  }

  KTXHeader header;
  file.read(reinterpret_cast<char *>(&header), sizeof(header));
  if (!file) {
    E2D_LOG_ERROR("Failed to read KTX header: {}", filepath);
    return false;
  }

  // 验证标识符
  if (std::memcmp(header.identifier, KTX_IDENTIFIER, 12) != 0) {
    E2D_LOG_ERROR("Invalid KTX identifier: {}", filepath);
    return false;
  }

  width_ = static_cast<int>(header.pixelWidth);
  height_ = static_cast<int>(header.pixelHeight);
  channels_ = 4; // 压缩纹理通常解压为 RGBA

  // 确定压缩格式
  GLenum glInternalFormat = header.glInternalFormat;
  switch (glInternalFormat) {
  case GL_COMPRESSED_RGB8_ETC2:
    format_ = PixelFormat::ETC2_RGB8;
    channels_ = 3;
    break;
  case GL_COMPRESSED_RGBA8_ETC2_EAC:
    format_ = PixelFormat::ETC2_RGBA8;
    break;
  case GL_COMPRESSED_RGBA_ASTC_4x4:
    format_ = PixelFormat::ASTC_4x4;
    break;
  case GL_COMPRESSED_RGBA_ASTC_6x6:
    format_ = PixelFormat::ASTC_6x6;
    break;
  case GL_COMPRESSED_RGBA_ASTC_8x8:
    format_ = PixelFormat::ASTC_8x8;
    break;
  default:
    E2D_LOG_ERROR("Unsupported KTX internal format: {:#06x}", glInternalFormat);
    return false;
  }

  // 跳过 key-value 数据
  file.seekg(header.bytesOfKeyValueData, std::ios::cur);

  // 读取第一个 mipmap level
  uint32_t imageSize = 0;
  file.read(reinterpret_cast<char *>(&imageSize), sizeof(imageSize));
  if (!file || imageSize == 0) {
    E2D_LOG_ERROR("Failed to read KTX image size: {}", filepath);
    return false;
  }

  std::vector<uint8_t> compressedData(imageSize);
  file.read(reinterpret_cast<char *>(compressedData.data()), imageSize);
  if (!file) {
    E2D_LOG_ERROR("Failed to read KTX image data: {}", filepath);
    return false;
  }

  // 创建 GL 纹理
  glGenTextures(1, &textureID_);
  bind();

  glCompressedTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width_, height_, 0,
                         static_cast<GLsizei>(imageSize),
                         compressedData.data());

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    E2D_LOG_ERROR("glCompressedTexImage2D failed for KTX: {:#06x}", err);
    glDeleteTextures(1, &textureID_);
    textureID_ = 0;
    return false;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // VRAM 跟踪
  dataSize_ = imageSize;
  VRAMManager::getInstance().allocTexture(dataSize_);

  E2D_LOG_INFO("Loaded compressed KTX texture: {} ({}x{}, format={:#06x})",
               filepath, width_, height_, glInternalFormat);
  return true;
}

bool GLTexture::loadDDS(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    E2D_LOG_ERROR("Failed to open DDS file: {}", filepath);
    return false;
  }

  DDSHeader header;
  file.read(reinterpret_cast<char *>(&header), sizeof(header));
  if (!file) {
    E2D_LOG_ERROR("Failed to read DDS header: {}", filepath);
    return false;
  }

  if (header.magic != DDS_MAGIC) {
    E2D_LOG_ERROR("Invalid DDS magic: {}", filepath);
    return false;
  }

  width_ = static_cast<int>(header.width);
  height_ = static_cast<int>(header.height);
  channels_ = 4;

  GLenum glInternalFormat = 0;

  // 检查 DX10 扩展头
  if ((header.pixelFormat.flags & DDPF_FOURCC) &&
      header.pixelFormat.fourCC == makeFourCC('D', 'X', '1', '0')) {
    DDSHeaderDXT10 dx10Header;
    file.read(reinterpret_cast<char *>(&dx10Header), sizeof(dx10Header));
    if (!file) {
      E2D_LOG_ERROR("Failed to read DDS DX10 header: {}", filepath);
      return false;
    }

    // DXGI_FORMAT 映射到 GL 格式
    switch (dx10Header.dxgiFormat) {
    case 147: // DXGI_FORMAT_ETC2_RGB8
      glInternalFormat = GL_COMPRESSED_RGB8_ETC2;
      format_ = PixelFormat::ETC2_RGB8;
      channels_ = 3;
      break;
    case 148: // DXGI_FORMAT_ETC2_RGBA8
      glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
      format_ = PixelFormat::ETC2_RGBA8;
      break;
    default:
      E2D_LOG_ERROR("Unsupported DDS DX10 format: {}", dx10Header.dxgiFormat);
      return false;
    }
  } else {
    E2D_LOG_ERROR("DDS file does not use DX10 extension, unsupported: {}",
                  filepath);
    return false;
  }

  // 计算压缩数据大小
  size_t blockSize = (glInternalFormat == GL_COMPRESSED_RGB8_ETC2) ? 8 : 16;
  size_t blocksWide = (width_ + 3) / 4;
  size_t blocksHigh = (height_ + 3) / 4;
  size_t imageSize = blocksWide * blocksHigh * blockSize;

  std::vector<uint8_t> compressedData(imageSize);
  file.read(reinterpret_cast<char *>(compressedData.data()), imageSize);
  if (!file) {
    E2D_LOG_ERROR("Failed to read DDS image data: {}", filepath);
    return false;
  }

  // 创建 GL 纹理
  glGenTextures(1, &textureID_);
  bind();

  glCompressedTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width_, height_, 0,
                         static_cast<GLsizei>(imageSize),
                         compressedData.data());

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    E2D_LOG_ERROR("glCompressedTexImage2D failed for DDS: {:#06x}", err);
    glDeleteTextures(1, &textureID_);
    textureID_ = 0;
    return false;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // VRAM 跟踪
  dataSize_ = imageSize;
  VRAMManager::getInstance().allocTexture(dataSize_);

  E2D_LOG_INFO("Loaded compressed DDS texture: {} ({}x{})", filepath, width_,
               height_);
  return true;
}

void GLTexture::generateAlphaMask() {
  if (pixelData_.empty() || width_ <= 0 || height_ <= 0) {
    E2D_LOG_WARN("Cannot generate alpha mask: no pixel data available");
    return;
  }

  alphaMask_ = std::make_unique<AlphaMask>(AlphaMask::createFromPixels(
      pixelData_.data(), width_, height_, channels_));

  E2D_LOG_DEBUG("Generated alpha mask for texture: {}x{}", width_, height_);
}

PixelFormat GLTexture::getFormat() const { return format_; }

Ptr<Texture> GLTexture::create(int width, int height, PixelFormat format) {
  int channels = 4;
  switch (format) {
  case PixelFormat::R8:
    channels = 1;
    break;
  case PixelFormat::RG8:
    channels = 2;
    break;
  case PixelFormat::RGB8:
    channels = 3;
    break;
  case PixelFormat::RGBA8:
    channels = 4;
    break;
  default:
    channels = 4;
    break;
  }
  return makePtr<GLTexture>(width, height, nullptr, channels);
}

} // namespace extra2d
