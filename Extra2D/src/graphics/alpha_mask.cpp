#include <extra2d/graphics/alpha_mask.h>

namespace extra2d {

/**
 * @brief 构造函数
 *
 * 创建指定尺寸的Alpha遮罩，初始时所有像素均为不透明（值为255）
 *
 * @param width 遮罩宽度（像素）
 * @param height 遮罩高度（像素）
 */
AlphaMask::AlphaMask(int width, int height)
    : width_(width), height_(height), data_(width * height, 255) {}

/**
 * @brief 从像素数据创建Alpha遮罩
 *
 * 根据输入的像素数据提取Alpha通道创建遮罩，
 * 支持RGBA（4通道）、RGB（3通道）和灰度（1通道）格式
 *
 * @param pixels 像素数据指针
 * @param width 图像宽度
 * @param height 图像高度
 * @param channels 通道数量（1、3或4）
 * @return 创建的Alpha遮罩对象
 */
AlphaMask AlphaMask::createFromPixels(const uint8_t *pixels, int width,
                                      int height, int channels) {
  AlphaMask mask(width, height);

  if (!pixels || width <= 0 || height <= 0) {
    return mask;
  }

  // 根据通道数提取Alpha值
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixelIndex = (y * width + x) * channels;
      uint8_t alpha = 255;

      if (channels == 4) {
        // RGBA格式，Alpha在第四个通道
        alpha = pixels[pixelIndex + 3];
      } else if (channels == 1) {
        // 灰度图，直接作为Alpha
        alpha = pixels[pixelIndex];
      } else if (channels == 3) {
        // RGB格式，没有Alpha通道，视为不透明
        alpha = 255;
      }

      mask.data_[y * width + x] = alpha;
    }
  }

  return mask;
}

/**
 * @brief 获取指定位置的Alpha值
 *
 * 返回指定坐标处的Alpha值，如果坐标无效则返回0
 *
 * @param x X坐标
 * @param y Y坐标
 * @return Alpha值（0-255），坐标无效时返回0
 */
uint8_t AlphaMask::getAlpha(int x, int y) const {
  if (!isValid(x, y)) {
    return 0;
  }
  return data_[y * width_ + x];
}

/**
 * @brief 检查指定位置是否不透明
 *
 * 判断指定坐标处的Alpha值是否大于等于给定的阈值
 *
 * @param x X坐标
 * @param y Y坐标
 * @param threshold 不透明度阈值（默认为255，即完全不透明）
 * @return 如果Alpha值大于等于阈值返回true，否则返回false
 */
bool AlphaMask::isOpaque(int x, int y, uint8_t threshold) const {
  return getAlpha(x, y) >= threshold;
}

/**
 * @brief 检查坐标是否有效
 *
 * 判断给定的坐标是否在遮罩的有效范围内
 *
 * @param x X坐标
 * @param y Y坐标
 * @return 如果坐标在有效范围内返回true，否则返回false
 */
bool AlphaMask::isValid(int x, int y) const {
  return x >= 0 && x < width_ && y >= 0 && y < height_;
}

} // namespace extra2d
