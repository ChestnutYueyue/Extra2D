#pragma once

#include <extra2d/core/types.h>
#include <extra2d/platform/iwindow.h>
#include <extra2d/platform/iinput.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

namespace extra2d {

/**
 * @brief 平台模块配置
 */
struct PlatformModuleConfig {
    std::string backend = "sdl2";
    bool gamepad = true;
    bool touch = true;
    float deadzone = 0.15f;
};

/**
 * @brief 平台后端工厂
 * 用于注册和创建平台后端
 */
class BackendFactory {
public:
    using WindowFn = std::function<UniquePtr<IWindow>()>;
    using InputFn = std::function<UniquePtr<IInput>()>;

    /**
     * @brief 注册平台后端
     * @param name 后端名称
     * @param win 窗口创建函数
     * @param in 输入创建函数
     */
    static void reg(const std::string& name, WindowFn win, InputFn in);

    /**
     * @brief 创建窗口实例
     * @param name 后端名称
     * @return 窗口实例，如果后端不存在返回 nullptr
     */
    static UniquePtr<IWindow> createWindow(const std::string& name);

    /**
     * @brief 创建输入实例
     * @param name 后端名称
     * @return 输入实例，如果后端不存在返回 nullptr
     */
    static UniquePtr<IInput> createInput(const std::string& name);

    /**
     * @brief 获取所有已注册的后端名称
     */
    static std::vector<std::string> backends();

    /**
     * @brief 检查后端是否存在
     */
    static bool has(const std::string& name);

private:
    struct BackendEntry {
        WindowFn windowFn;
        InputFn inputFn;
    };

    static std::unordered_map<std::string, BackendEntry>& registry();
};

/**
 * @brief 平台后端注册宏
 * 在全局作用域使用此宏注册平台后端
 * 
 * @example
 * E2D_REG_BACKEND(sdl2, SDL2Window, SDL2Input)
 */
#define E2D_REG_BACKEND(name, WinClass, InClass) \
    namespace { \
        static struct E2D_BACKEND_REG_##name { \
            E2D_BACKEND_REG_##name() { \
                ::extra2d::BackendFactory::reg( \
                    #name, \
                    []() -> ::extra2d::UniquePtr<::extra2d::IWindow> { \
                        return ::extra2d::makeUnique<WinClass>(); \
                    }, \
                    []() -> ::extra2d::UniquePtr<::extra2d::IInput> { \
                        return ::extra2d::makeUnique<InClass>(); \
                    } \
                ); \
            } \
        } e2d_backend_reg_##name; \
    }

} // namespace extra2d
