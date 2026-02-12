// ============================================================================
// Number.h - 数字显示类
// 描述: 将整数数字转换为精灵图片显示
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>

namespace flappybird {

/**
 * @brief 数字显示类
 * 用于显示得分，将整数转换为对应的数字图片
 */
class Number : public extra2d::Node {
public:
    /**
     * @brief 构造函数
     */
    Number();

    /**
     * @brief 设置显示的数字（大号）
     * @param number 要显示的数字
     */
    void setNumber(int number);

    /**
     * @brief 设置显示的数字（小号）
     * @param number 要显示的数字
     */
    void setLittleNumber(int number);

    /**
     * @brief 获取当前数字
     * @return 当前数字
     */
    int getNumber() const { return number_; }

private:
    /**
     * @brief 创建数字精灵
     * @param number 数字值
     * @param prefix 数字图片前缀（"number_big_" 或 "number_medium_"）
     */
    void createNumberSprites(int number, const std::string& prefix);

    int number_ = 0;  // 当前数字
};

} // namespace flappybird
