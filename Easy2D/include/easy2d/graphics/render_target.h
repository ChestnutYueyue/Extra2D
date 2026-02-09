#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/color.h>
#include <easy2d/graphics/texture.h>
#include <easy2d/graphics/opengl/gl_texture.h>
#include <mutex>

namespace easy2d {

// ============================================================================
// 渲染目标配置
// ============================================================================
struct RenderTargetConfig {
    int width = 800;                    // 宽度
    int height = 600;                   // 高度
    PixelFormat colorFormat = PixelFormat::RGBA8;  // 颜色格式
    bool hasDepth = true;               // 是否包含深度缓冲
    bool hasDepthBuffer = true;         // 兼容旧API的别名 (同hasDepth)
    bool hasStencil = false;            // 是否包含模板缓冲
    int samples = 1;                    // 多重采样数 (1 = 无MSAA)
    bool autoResize = true;             // 是否自动调整大小
};

// ============================================================================
// 渲染目标 - 基于FBO的离屏渲染
// ============================================================================
class RenderTarget {
public:
    RenderTarget();
    ~RenderTarget();

    // 禁止拷贝
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    // 允许移动
    RenderTarget(RenderTarget&& other) noexcept;
    RenderTarget& operator=(RenderTarget&& other) noexcept;

    // ------------------------------------------------------------------------
    // 创建和销毁
    // ------------------------------------------------------------------------
    
    /**
     * @brief 创建渲染目标
     */
    bool create(const RenderTargetConfig& config);
    
    /**
     * @brief 初始化渲染目标（create的别名，兼容旧API）
     */
    bool init(const RenderTargetConfig& config) { return create(config); }
    
    /**
     * @brief 从现有纹理创建渲染目标
     */
    bool createFromTexture(Ptr<Texture> texture, bool hasDepth = false);
    
    /**
     * @brief 销毁渲染目标
     */
    void destroy();
    
    /**
     * @brief 关闭渲染目标（destroy的别名，兼容旧API）
     */
    void shutdown() { destroy(); }
    
    /**
     * @brief 检查是否有效
     */
    bool isValid() const { return fbo_ != 0; }

    // ------------------------------------------------------------------------
    // 尺寸和格式
    // ------------------------------------------------------------------------
    
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    Vec2 getSize() const { return Vec2(static_cast<float>(width_), static_cast<float>(height_)); }
    PixelFormat getColorFormat() const { return colorFormat_; }

    // ------------------------------------------------------------------------
    // 绑定和解绑
    // ------------------------------------------------------------------------
    
    /**
     * @brief 绑定为当前渲染目标
     */
    void bind();
    
    /**
     * @brief 解绑（恢复默认渲染目标）
     */
    void unbind();
    
    /**
     * @brief 清除渲染目标
     */
    void clear(const Color& color = Colors::Transparent);

    // ------------------------------------------------------------------------
    // 纹理访问
    // ------------------------------------------------------------------------
    
    /**
     * @brief 获取颜色纹理
     */
    Ptr<Texture> getColorTexture() const { return colorTexture_; }
    
    /**
     * @brief 获取深度纹理（如果有）
     */
    Ptr<Texture> getDepthTexture() const { return depthTexture_; }

    // ------------------------------------------------------------------------
    // 视口和裁剪
    // ------------------------------------------------------------------------
    
    /**
     * @brief 设置视口（相对于渲染目标）
     */
    void setViewport(int x, int y, int width, int height);
    
    /**
     * @brief 获取完整视口
     */
    void getFullViewport(int& x, int& y, int& width, int& height) const;

    // ------------------------------------------------------------------------
    // 工具方法
    // ------------------------------------------------------------------------
    
    /**
     * @brief 调整大小（会销毁并重新创建）
     */
    bool resize(int width, int height);
    
    /**
     * @brief 复制到另一个渲染目标
     */
    void copyTo(RenderTarget& target);
    
    /**
     * @brief 复制到另一个渲染目标（blitTo的别名，兼容旧API）
     * @param target 目标渲染目标
     * @param color 是否复制颜色缓冲
     * @param depth 是否复制深度缓冲
     */
    void blitTo(RenderTarget& target, bool color = true, bool depth = false);
    
    /**
     * @brief 复制到屏幕
     */
    void copyToScreen(int screenWidth, int screenHeight);
    
    /**
     * @brief 保存为图像文件
     */
    bool saveToFile(const std::string& filepath);

    // ------------------------------------------------------------------------
    // 静态方法
    // ------------------------------------------------------------------------
    
    /**
     * @brief 创建渲染目标的静态工厂方法
     */
    static Ptr<RenderTarget> createFromConfig(const RenderTargetConfig& config);
    
    /**
     * @brief 获取当前绑定的渲染目标ID
     */
    static GLuint getCurrentFBO();
    
    /**
     * @brief 绑定默认渲染目标（屏幕）
     */
    static void bindDefault();

    /**
     * @brief 获取FBO ID（供内部使用）
     */
    GLuint getFBO() const { return fbo_; }

protected:
    GLuint fbo_ = 0;                    // 帧缓冲对象
    GLuint rbo_ = 0;                    // 渲染缓冲对象（深度/模板）
    
    Ptr<Texture> colorTexture_;         // 颜色纹理
    Ptr<Texture> depthTexture_;         // 深度纹理（可选）
    
    int width_ = 0;
    int height_ = 0;
    PixelFormat colorFormat_ = PixelFormat::RGBA8;
    bool hasDepth_ = false;
    bool hasStencil_ = false;
    int samples_ = 1;
    
    bool createFBO();
    void deleteFBO();
};

// ============================================================================
// 多重采样渲染目标（用于MSAA）
// ============================================================================
class MultisampleRenderTarget : public RenderTarget {
public:
    /**
     * @brief 创建多重采样渲染目标
     */
    bool create(int width, int height, int samples = 4);
    
    /**
     * @brief 解析到普通渲染目标（用于显示）
     */
    void resolveTo(RenderTarget& target);
    
    /**
     * @brief 销毁渲染目标
     */
    void destroy();

private:
    GLuint colorRBO_ = 0;  // 多重采样颜色渲染缓冲
};

// ============================================================================
// 渲染目标栈（用于嵌套渲染）
// ============================================================================
class RenderTargetStack {
public:
    static RenderTargetStack& getInstance();
    
    /**
     * @brief 压入渲染目标
     */
    void push(RenderTarget* target);
    
    /**
     * @brief 弹出渲染目标
     */
    void pop();
    
    /**
     * @brief 获取当前渲染目标
     */
    RenderTarget* getCurrent() const;
    
    /**
     * @brief 获取栈大小
     */
    size_t size() const;
    
    /**
     * @brief 清空栈
     */
    void clear();

private:
    RenderTargetStack() = default;
    ~RenderTargetStack() = default;
    
    std::vector<RenderTarget*> stack_;
    mutable std::mutex mutex_;
};

// ============================================================================
// 渲染目标管理器 - 全局渲染目标管理
// ============================================================================
class RenderTargetManager {
public:
    /**
     * @brief 获取单例实例
     */
    static RenderTargetManager& getInstance();

    /**
     * @brief 初始化渲染目标管理器
     * @param width 默认宽度
     * @param height 默认高度
     */
    bool init(int width, int height);

    /**
     * @brief 关闭渲染目标管理器
     */
    void shutdown();

    /**
     * @brief 创建新的渲染目标
     */
    Ptr<RenderTarget> createRenderTarget(const RenderTargetConfig& config);

    /**
     * @brief 获取默认渲染目标
     */
    RenderTarget* getDefaultRenderTarget() const { return defaultRenderTarget_.get(); }

    /**
     * @brief 调整所有受管渲染目标的大小
     */
    void resize(int width, int height);

    /**
     * @brief 检查是否已初始化
     */
    bool isInitialized() const { return initialized_; }

private:
    RenderTargetManager() = default;
    ~RenderTargetManager() = default;
    RenderTargetManager(const RenderTargetManager&) = delete;
    RenderTargetManager& operator=(const RenderTargetManager&) = delete;

    Ptr<RenderTarget> defaultRenderTarget_;
    std::vector<Ptr<RenderTarget>> renderTargets_;
    bool initialized_ = false;
};

// ============================================================================
// 便捷宏
// ============================================================================
#define E2D_RENDER_TARGET_STACK() ::easy2d::RenderTargetStack::getInstance()
#define E2D_RENDER_TARGET_MANAGER() ::easy2d::RenderTargetManager::getInstance()

} // namespace easy2d
