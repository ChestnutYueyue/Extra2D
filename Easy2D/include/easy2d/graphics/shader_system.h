#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/color.h>
#include <easy2d/graphics/opengl/gl_shader.h>
#include <string>
#include <unordered_map>
#include <functional>

namespace easy2d {

// ============================================================================
// Shader参数绑定回调
// ============================================================================
using ShaderBindCallback = std::function<void(GLShader&)>;

// ============================================================================
// Shader系统 - 管理所有Shader的加载、缓存和热重载
// ============================================================================
class ShaderSystem {
public:
    // ------------------------------------------------------------------------
    // 单例访问
    // ------------------------------------------------------------------------
    static ShaderSystem& getInstance();

    // ------------------------------------------------------------------------
    // 初始化和关闭
    // ------------------------------------------------------------------------
    bool init();
    void shutdown();

    // ------------------------------------------------------------------------
    // Shader加载
    // ------------------------------------------------------------------------
    
    /**
     * @brief 从文件加载Shader
     * @param name Shader名称（用于缓存）
     * @param vertPath 顶点着色器文件路径
     * @param fragPath 片段着色器文件路径
     * @return 加载的Shader，失败返回nullptr
     */
    Ptr<GLShader> loadFromFile(const std::string& name, 
                               const std::string& vertPath, 
                               const std::string& fragPath);
    
    /**
     * @brief 从源码字符串加载Shader
     * @param name Shader名称（用于缓存）
     * @param vertSource 顶点着色器源码
     * @param fragSource 片段着色器源码
     * @return 加载的Shader，失败返回nullptr
     */
    Ptr<GLShader> loadFromSource(const std::string& name,
                                 const std::string& vertSource,
                                 const std::string& fragSource);
    
    /**
     * @brief 从已编译的程序获取Shader
     * @param name Shader名称
     * @return 缓存的Shader，不存在返回nullptr
     */
    Ptr<GLShader> get(const std::string& name);
    
    /**
     * @brief 检查Shader是否存在
     */
    bool has(const std::string& name) const;

    // ------------------------------------------------------------------------
    // Shader移除
    // ------------------------------------------------------------------------
    void remove(const std::string& name);
    void clear();

    // ------------------------------------------------------------------------
    // 热重载支持
    // ------------------------------------------------------------------------
    
    /**
     * @brief 启用/禁用文件监视
     */
    void setFileWatching(bool enable);
    bool isFileWatching() const { return fileWatching_; }
    
    /**
     * @brief 更新文件监视（在主循环中调用）
     */
    void updateFileWatching();
    
    /**
     * @brief 重载指定Shader
     */
    bool reload(const std::string& name);
    
    /**
     * @brief 重载所有Shader
     */
    void reloadAll();

    // ------------------------------------------------------------------------
    // 内置Shader获取
    // ------------------------------------------------------------------------
    Ptr<GLShader> getBuiltinSpriteShader();
    Ptr<GLShader> getBuiltinParticleShader();
    Ptr<GLShader> getBuiltinPostProcessShader();
    Ptr<GLShader> getBuiltinShapeShader();

    // ------------------------------------------------------------------------
    // 工具方法
    // ------------------------------------------------------------------------
    
    /**
     * @brief 从文件读取文本内容
     */
    static std::string readFile(const std::string& filepath);
    
    /**
     * @brief 获取Shader文件的最后修改时间
     */
    static uint64_t getFileModifiedTime(const std::string& filepath);

private:
    ShaderSystem() = default;
    ~ShaderSystem() = default;
    ShaderSystem(const ShaderSystem&) = delete;
    ShaderSystem& operator=(const ShaderSystem&) = delete;

    struct ShaderInfo {
        Ptr<GLShader> shader;
        std::string vertPath;
        std::string fragPath;
        uint64_t vertModifiedTime;
        uint64_t fragModifiedTime;
        bool isBuiltin;
    };

    std::unordered_map<std::string, ShaderInfo> shaders_;
    bool fileWatching_ = false;
    float watchTimer_ = 0.0f;
    static constexpr float WATCH_INTERVAL = 1.0f; // 检查间隔（秒）

    // 内置Shader缓存
    Ptr<GLShader> builtinSpriteShader_;
    Ptr<GLShader> builtinParticleShader_;
    Ptr<GLShader> builtinPostProcessShader_;
    Ptr<GLShader> builtinShapeShader_;

    bool loadBuiltinShaders();
    void checkAndReload();
};

// ============================================================================
// Shader参数包装器 - 简化Uniform设置
// ============================================================================
class ShaderParams {
public:
    explicit ShaderParams(GLShader& shader);

    ShaderParams& setBool(const std::string& name, bool value);
    ShaderParams& setInt(const std::string& name, int value);
    ShaderParams& setFloat(const std::string& name, float value);
    ShaderParams& setVec2(const std::string& name, const glm::vec2& value);
    ShaderParams& setVec3(const std::string& name, const glm::vec3& value);
    ShaderParams& setVec4(const std::string& name, const glm::vec4& value);
    ShaderParams& setMat4(const std::string& name, const glm::mat4& value);
    ShaderParams& setColor(const std::string& name, const Color& color);

private:
    GLShader& shader_;
};

// ============================================================================
// 便捷宏
// ============================================================================
#define E2D_SHADER_SYSTEM() ::easy2d::ShaderSystem::getInstance()

} // namespace easy2d
