#pragma once

#include <extra2d/core/types.h>
#include <extra2d/core/math_types.h>
#include <extra2d/platform/window_config.h>
#include <functional>
#include <string>

namespace extra2d {

class IInput;

/**
 * @brief 光标形状
 */
enum class Cursor {
    Arrow,
    IBeam,
    Crosshair,
    Hand,
    HResize,
    VResize,
    Hidden
};

/**
 * @brief 窗口抽象接口
 * 所有平台窗口后端必须实现此接口
 */
class IWindow {
public:
    virtual ~IWindow() = default;

    /**
     * @brief 创建窗口
     * @param cfg 窗口配置
     * @return 创建是否成功
     */
    virtual bool create(const WindowConfigData& cfg) = 0;

    /**
     * @brief 销毁窗口
     */
    virtual void destroy() = 0;

    /**
     * @brief 轮询事件
     */
    virtual void poll() = 0;

    /**
     * @brief 交换缓冲区
     */
    virtual void swap() = 0;

    /**
     * @brief 窗口是否应该关闭
     */
    virtual bool shouldClose() const = 0;

    /**
     * @brief 设置窗口关闭标志
     */
    virtual void close() = 0;

    /**
     * @brief 设置窗口标题
     */
    virtual void setTitle(const std::string& title) = 0;

    /**
     * @brief 设置窗口大小
     */
    virtual void setSize(int w, int h) = 0;

    /**
     * @brief 设置窗口位置
     */
    virtual void setPos(int x, int y) = 0;

    /**
     * @brief 设置全屏模式
     */
    virtual void setFullscreen(bool fs) = 0;

    /**
     * @brief 设置垂直同步
     */
    virtual void setVSync(bool vsync) = 0;

    /**
     * @brief 设置窗口可见性
     */
    virtual void setVisible(bool visible) = 0;

    /**
     * @brief 获取窗口宽度
     */
    virtual int width() const = 0;

    /**
     * @brief 获取窗口高度
     */
    virtual int height() const = 0;

    /**
     * @brief 获取窗口大小
     */
    virtual Size size() const = 0;

    /**
     * @brief 获取窗口位置
     */
    virtual Vec2 pos() const = 0;

    /**
     * @brief 是否全屏
     */
    virtual bool fullscreen() const = 0;

    /**
     * @brief 是否启用垂直同步
     */
    virtual bool vsync() const = 0;

    /**
     * @brief 窗口是否获得焦点
     */
    virtual bool focused() const = 0;

    /**
     * @brief 窗口是否最小化
     */
    virtual bool minimized() const = 0;

    /**
     * @brief 获取内容缩放X
     */
    virtual float scaleX() const = 0;

    /**
     * @brief 获取内容缩放Y
     */
    virtual float scaleY() const = 0;

    /**
     * @brief 设置光标形状
     */
    virtual void setCursor(Cursor cursor) = 0;

    /**
     * @brief 显示/隐藏光标
     */
    virtual void showCursor(bool show) = 0;

    /**
     * @brief 锁定/解锁光标
     */
    virtual void lockCursor(bool lock) = 0;

    /**
     * @brief 获取输入接口
     */
    virtual IInput* input() const = 0;

    /**
     * @brief 窗口大小改变回调
     */
    using ResizeCb = std::function<void(int, int)>;

    /**
     * @brief 窗口关闭回调
     */
    using CloseCb = std::function<void()>;

    /**
     * @brief 窗口焦点改变回调
     */
    using FocusCb = std::function<void(bool)>;

    /**
     * @brief 设置大小改变回调
     */
    virtual void onResize(ResizeCb cb) = 0;

    /**
     * @brief 设置关闭回调
     */
    virtual void onClose(CloseCb cb) = 0;

    /**
     * @brief 设置焦点改变回调
     */
    virtual void onFocus(FocusCb cb) = 0;

    /**
     * @brief 获取原生窗口句柄
     */
    virtual void* native() const = 0;
};

} // namespace extra2d
