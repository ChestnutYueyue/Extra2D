#pragma once

#include <easy2d/graphics/texture.h>
#include <easy2d/graphics/alpha_mask.h>

// 使用标准 GLES3.2
#include <GLES3/gl32.h>

#include <memory>

namespace easy2d {

// ============================================================================
// OpenGL 纹理实现
// ============================================================================
class GLTexture : public Texture {
public:
    GLTexture(int width, int height, const uint8_t* pixels, int channels);
    GLTexture(const std::string& filepath);
    ~GLTexture();

    // Texture 接口实现
    int getWidth() const override { return width_; }
    int getHeight() const override { return height_; }
    Size getSize() const override { return Size(static_cast<float>(width_), static_cast<float>(height_)); }
    int getChannels() const override { return channels_; }
    PixelFormat getFormat() const override;
    void* getNativeHandle() const override { return reinterpret_cast<void*>(static_cast<uintptr_t>(textureID_)); }
    bool isValid() const override { return textureID_ != 0; }
    void setFilter(bool linear) override;
    void setWrap(bool repeat) override;

    // 从参数创建纹理的工厂方法
    static Ptr<Texture> create(int width, int height, PixelFormat format);

    // 加载压缩纹理（KTX/DDS 格式）
    bool loadCompressed(const std::string& filepath);

    // OpenGL 特定
    GLuint getTextureID() const { return textureID_; }
    void bind(unsigned int slot = 0) const;
    void unbind() const;

    // 获取纹理数据大小（字节），用于 VRAM 跟踪
    size_t getDataSize() const { return dataSize_; }

    // Alpha 遮罩
    bool hasAlphaMask() const { return alphaMask_ != nullptr && alphaMask_->isValid(); }
    const AlphaMask* getAlphaMask() const { return alphaMask_.get(); }
    void generateAlphaMask();  // 从当前纹理数据生成遮罩

private:
    GLuint textureID_;
    int width_;
    int height_;
    int channels_;
    PixelFormat format_;
    size_t dataSize_;

    // 原始像素数据（用于生成遮罩）
    std::vector<uint8_t> pixelData_;
    std::unique_ptr<AlphaMask> alphaMask_;

    void createTexture(const uint8_t* pixels);

    // KTX 文件加载
    bool loadKTX(const std::string& filepath);
    // DDS 文件加载
    bool loadDDS(const std::string& filepath);
};

} // namespace easy2d
