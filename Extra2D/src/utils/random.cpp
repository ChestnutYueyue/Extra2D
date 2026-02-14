#include <chrono>
#include <extra2d/utils/random.h>

namespace extra2d {

/**
 * @brief 构造函数，初始化随机数生成器
 *
 * 使用当前时间作为默认种子初始化随机数生成器
 */
Random::Random() : floatDist_(0.0f, 1.0f) {
  // 使用当前时间作为默认种子
  randomize();
}

/**
 * @brief 获取Random单例实例
 * @return Random单例的引用
 */
Random &Random::get() {
  static Random instance;
  return instance;
}

/**
 * @brief 设置随机数种子
 * @param seed 随机数种子值
 */
void Random::setSeed(uint32 seed) { generator_.seed(seed); }

/**
 * @brief 使用当前时间随机化种子
 *
 * 使用高精度时钟的当前时间作为随机数生成器的种子
 */
void Random::randomize() {
  auto now = std::chrono::high_resolution_clock::now();
  auto time = now.time_since_epoch().count();
  generator_.seed(static_cast<uint32>(time));
}

/**
 * @brief 获取[0.0, 1.0]范围内的随机浮点数
 * @return 随机浮点数，范围[0.0, 1.0]
 */
float Random::getFloat() { return floatDist_(generator_); }

/**
 * @brief 获取指定范围内的随机浮点数
 * @param min 最小值
 * @param max 最大值
 * @return 随机浮点数，范围[min, max]
 */
float Random::getFloat(float min, float max) {
  if (min >= max) {
    return min;
  }
  return min + floatDist_(generator_) * (max - min);
}

/**
 * @brief 获取[0, max]范围内的随机整数
 * @param max 最大值（包含）
 * @return 随机整数，范围[0, max]
 */
int Random::getInt(int max) {
  if (max <= 0) {
    return 0;
  }
  std::uniform_int_distribution<int> dist(0, max);
  return dist(generator_);
}

/**
 * @brief 获取指定范围内的随机整数
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 * @return 随机整数，范围[min, max]
 */
int Random::getInt(int min, int max) {
  if (min >= max) {
    return min;
  }
  std::uniform_int_distribution<int> dist(min, max);
  return dist(generator_);
}

/**
 * @brief 获取随机布尔值（50%概率）
 * @return 随机布尔值
 */
bool Random::getBool() { return floatDist_(generator_) >= 0.5f; }

/**
 * @brief 以指定概率获取随机布尔值
 * @param probability 返回true的概率，范围[0.0, 1.0]
 * @return 随机布尔值
 */
bool Random::getBool(float probability) {
  if (probability <= 0.0f) {
    return false;
  }
  if (probability >= 1.0f) {
    return true;
  }
  return floatDist_(generator_) < probability;
}

/**
 * @brief 获取随机角度值
 * @return 随机角度，范围[0, 2π]
 */
float Random::getAngle() {
  static const float TWO_PI = 6.28318530718f;
  return floatDist_(generator_) * TWO_PI;
}

/**
 * @brief 获取有符号随机数
 * @return 随机浮点数，范围[-1.0, 1.0]
 */
float Random::getSigned() { return floatDist_(generator_) * 2.0f - 1.0f; }

} // namespace extra2d
