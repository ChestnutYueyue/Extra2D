#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/color.h>
#include <easy2d/graphics/texture.h>
#include <easy2d/graphics/opengl/gl_shader.h>
#include <string>
#include <functional>
#include <vector>

namespace easy2d {

// ============================================================================
// 前向声明
// ============================================================================
class RenderTarget;
class RenderBackend;

// ============================================================================
// 后处理效果基类
// ============================================================================
class PostProcessEffect {
public:
    PostProcessEffect(const std::string& name);
    virtual ~PostProcessEffect() = default;

    // ------------------------------------------------------------------------
    // 生命周期
    // ------------------------------------------------------------------------
    
    /**
     * @brief 初始化效果
     */
    virtual bool init();
    
    /**
     * @brief 关闭效果
     */
    virtual void shutdown();

    // ------------------------------------------------------------------------
    // 渲染
    // ------------------------------------------------------------------------
    
    /**
     * @brief 应用效果
     * @param source 输入纹理
     * @param target 输出渲染目标
     * @param renderer 渲染后端
     */
    virtual void apply(const Texture& source, RenderTarget& target, RenderBackend& renderer);
    
    /**
     * @brief 设置Shader参数（子类重写）
     */
    virtual void onShaderBind(GLShader& shader) {}

    // ------------------------------------------------------------------------
    // 状态
    // ------------------------------------------------------------------------
    const std::string& getName() const { return name_; }
    bool isEnabled() const { return enabled_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isValid() const { return valid_; }

    // ------------------------------------------------------------------------
    // 链式API
    // ------------------------------------------------------------------------
    PostProcessEffect& withEnabled(bool enabled) {
        enabled_ = enabled;
        return *this;
    }

protected:
    std::string name_;
    bool enabled_ = true;
    bool valid_ = false;
    Ptr<GLShader> shader_;

    /**
     * @brief 加载自定义Shader
     */
    bool loadShader(const std::string& vertSource, const std::string& fragSource);
    bool loadShaderFromFile(const std::string& vertPath, const std::string& fragPath);
    
    /**
     * @brief 渲染全屏四边形
     */
    void renderFullscreenQuad();

private:
    static GLuint quadVao_;
    static GLuint quadVbo_;
    static bool quadInitialized_;
    
    void initQuad();
    void destroyQuad();
};

// ============================================================================
// 后处理栈 - 管理多个后处理效果
// ============================================================================
class PostProcessStack {
public:
    PostProcessStack();
    ~PostProcessStack();

    // ------------------------------------------------------------------------
    // 初始化和关闭
    // ------------------------------------------------------------------------
    bool init(int width, int height);
    void shutdown();

    // ------------------------------------------------------------------------
    // 效果管理
    // ------------------------------------------------------------------------
    
    /**
     * @brief 添加效果到栈
     */
    void addEffect(Ptr<PostProcessEffect> effect);
    
    /**
     * @brief 插入效果到指定位置
     */
    void insertEffect(size_t index, Ptr<PostProcessEffect> effect);
    
    /**
     * @brief 移除效果
     */
    void removeEffect(const std::string& name);
    void removeEffect(size_t index);
    
    /**
     * @brief 获取效果
     */
    Ptr<PostProcessEffect> getEffect(const std::string& name);
    Ptr<PostProcessEffect> getEffect(size_t index);
    
    /**
     * @brief 清空所有效果
     */
    void clearEffects();
    
    /**
     * @brief 获取效果数量
     */
    size_t getEffectCount() const { return effects_.size(); }

    // ------------------------------------------------------------------------
    // 渲染
    // ------------------------------------------------------------------------
    
    /**
     * @brief 开始捕获场景
     */
    void beginCapture();
    
    /**
     * @brief 结束捕获并应用所有效果
     */
    void endCapture(RenderBackend& renderer);
    
    /**
     * @brief 直接应用效果到纹理
     */
    void process(const Texture& source, RenderTarget& target, RenderBackend& renderer);

    // ------------------------------------------------------------------------
    // 配置
    // ------------------------------------------------------------------------
    void resize(int width, int height);
    bool isValid() const { return valid_; }

    // ------------------------------------------------------------------------
    // 便捷方法 - 添加内置效果
    // ------------------------------------------------------------------------
    PostProcessStack& addBloom(float intensity = 1.0f, float threshold = 0.8f);
    PostProcessStack& addBlur(float radius = 2.0f);
    PostProcessStack& addColorGrading(const Color& tint);
    PostProcessStack& addVignette(float intensity = 0.5f);
    PostProcessStack& addChromaticAberration(float amount = 1.0f);

private:
    std::vector<Ptr<PostProcessEffect>> effects_;
    Ptr<RenderTarget> renderTargetA_;
    Ptr<RenderTarget> renderTargetB_;
    int width_ = 0;
    int height_ = 0;
    bool valid_ = false;
    bool capturing_ = false;
};

// ============================================================================
// 全局后处理管理
// ============================================================================
class PostProcessManager {
public:
    static PostProcessManager& getInstance();
    
    void init(int width, int height);
    void shutdown();
    
    PostProcessStack& getMainStack() { return mainStack_; }
    
    void resize(int width, int height);
    void beginFrame();
    void endFrame(RenderBackend& renderer);

private:
    PostProcessManager() = default;
    ~PostProcessManager() = default;
    PostProcessManager(const PostProcessManager&) = delete;
    PostProcessManager& operator=(const PostProcessManager&) = delete;

    PostProcessStack mainStack_;
    bool initialized_ = false;
};

// ============================================================================
// 便捷宏
// ============================================================================
#define E2D_POST_PROCESS() ::easy2d::PostProcessManager::getInstance()

} // namespace easy2d
