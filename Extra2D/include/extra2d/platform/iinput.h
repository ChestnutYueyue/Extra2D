#pragma once

#include <extra2d/platform/keys.h>
#include <extra2d/core/math_types.h>

namespace extra2d {

/**
 * @brief 触摸点信息
 */
struct TouchPoint {
    int id = 0;
    Vec2 position;
    Vec2 delta;
    bool pressed = false;
    bool released = false;
};

/**
 * @brief 输入抽象接口
 * 所有平台输入后端必须实现此接口
 */
class IInput {
public:
    virtual ~IInput() = default;

    /**
     * @brief 初始化输入系统
     */
    virtual void init() = 0;

    /**
     * @brief 关闭输入系统
     */
    virtual void shutdown() = 0;

    /**
     * @brief 每帧更新输入状态
     */
    virtual void update() = 0;

    // ========== 键盘 ==========

    /**
     * @brief 检测按键是否按下（持续状态）
     */
    virtual bool down(Key key) const = 0;

    /**
     * @brief 检测按键是否刚按下（仅当前帧）
     */
    virtual bool pressed(Key key) const = 0;

    /**
     * @brief 检测按键是否刚释放（仅当前帧）
     */
    virtual bool released(Key key) const = 0;

    // ========== 鼠标 ==========

    /**
     * @brief 检测鼠标按钮是否按下
     */
    virtual bool down(Mouse btn) const = 0;

    /**
     * @brief 检测鼠标按钮是否刚按下
     */
    virtual bool pressed(Mouse btn) const = 0;

    /**
     * @brief 检测鼠标按钮是否刚释放
     */
    virtual bool released(Mouse btn) const = 0;

    /**
     * @brief 获取鼠标位置
     */
    virtual Vec2 mouse() const = 0;

    /**
     * @brief 获取鼠标移动增量
     */
    virtual Vec2 mouseDelta() const = 0;

    /**
     * @brief 获取滚轮值
     */
    virtual float scroll() const = 0;

    /**
     * @brief 获取滚轮增量
     */
    virtual float scrollDelta() const = 0;

    /**
     * @brief 设置鼠标位置
     */
    virtual void setMouse(const Vec2& pos) = 0;

    // ========== 手柄 ==========

    /**
     * @brief 检测手柄是否连接
     */
    virtual bool gamepad() const = 0;

    /**
     * @brief 检测手柄按钮是否按下
     */
    virtual bool down(Gamepad btn) const = 0;

    /**
     * @brief 检测手柄按钮是否刚按下
     */
    virtual bool pressed(Gamepad btn) const = 0;

    /**
     * @brief 检测手柄按钮是否刚释放
     */
    virtual bool released(Gamepad btn) const = 0;

    /**
     * @brief 获取左摇杆值
     */
    virtual Vec2 leftStick() const = 0;

    /**
     * @brief 获取右摇杆值
     */
    virtual Vec2 rightStick() const = 0;

    /**
     * @brief 获取左扳机值
     */
    virtual float leftTrigger() const = 0;

    /**
     * @brief 获取右扳机值
     */
    virtual float rightTrigger() const = 0;

    /**
     * @brief 设置手柄振动
     * @param left 左马达强度 [0, 1]
     * @param right 右马达强度 [0, 1]
     */
    virtual void vibrate(float left, float right) = 0;

    // ========== 触摸 ==========

    /**
     * @brief 检测是否有触摸
     */
    virtual bool touching() const = 0;

    /**
     * @brief 获取触摸点数量
     */
    virtual int touchCount() const = 0;

    /**
     * @brief 获取触摸点位置
     * @param index 触摸点索引
     */
    virtual Vec2 touch(int index) const = 0;

    /**
     * @brief 获取触摸点信息
     * @param index 触摸点索引
     */
    virtual TouchPoint touchPoint(int index) const = 0;
};

} // namespace extra2d
