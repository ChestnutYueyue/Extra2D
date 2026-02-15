#pragma once

#include <extra2d/core/color.h>
#include <extra2d/graphics/shader_interface.h>
#include <glad/glad.h>
#include <unordered_map>
#include <vector>

namespace extra2d {

// ============================================================================
// OpenGL Shader实现
// ============================================================================
class GLShaderNew : public IShader {
public:
    /**
     * @brief 构造函数
     */
    GLShaderNew();

    /**
     * @brief 析构函数
     */
    ~GLShaderNew() override;

    /**
     * @brief 绑定Shader程序
     */
    void bind() const override;

    /**
     * @brief 解绑Shader程序
     */
    void unbind() const override;

    /**
     * @brief 设置布尔类型uniform变量
     * @param name uniform变量名
     * @param value 布尔值
     */
    void setBool(const std::string& name, bool value) override;

    /**
     * @brief 设置整数类型uniform变量
     * @param name uniform变量名
     * @param value 整数值
     */
    void setInt(const std::string& name, int value) override;

    /**
     * @brief 设置浮点类型uniform变量
     * @param name uniform变量名
     * @param value 浮点值
     */
    void setFloat(const std::string& name, float value) override;

    /**
     * @brief 设置二维向量类型uniform变量
     * @param name uniform变量名
     * @param value 二维向量值
     */
    void setVec2(const std::string& name, const glm::vec2& value) override;

    /**
     * @brief 设置三维向量类型uniform变量
     * @param name uniform变量名
     * @param value 三维向量值
     */
    void setVec3(const std::string& name, const glm::vec3& value) override;

    /**
     * @brief 设置四维向量类型uniform变量
     * @param name uniform变量名
     * @param value 四维向量值
     */
    void setVec4(const std::string& name, const glm::vec4& value) override;

    /**
     * @brief 设置4x4矩阵类型uniform变量
     * @param name uniform变量名
     * @param value 4x4矩阵值
     */
    void setMat4(const std::string& name, const glm::mat4& value) override;

    /**
     * @brief 设置颜色类型uniform变量
     * @param name uniform变量名
     * @param color 颜色值
     */
    void setColor(const std::string& name, const Color& color) override;

    /**
     * @brief 检查Shader是否有效
     * @return 有效返回true，否则返回false
     */
    bool isValid() const override { return programID_ != 0; }

    /**
     * @brief 获取原生句柄（OpenGL程序ID）
     * @return OpenGL程序ID
     */
    uint32_t getNativeHandle() const override { return programID_; }

    /**
     * @brief 获取Shader名称
     * @return Shader名称
     */
    const std::string& getName() const override { return name_; }

    /**
     * @brief 设置Shader名称
     * @param name Shader名称
     */
    void setName(const std::string& name) override { name_ = name; }

    /**
     * @brief 从源码编译Shader
     * @param vertexSource 顶点着色器源码
     * @param fragmentSource 片段着色器源码
     * @return 编译成功返回true，失败返回false
     */
    bool compileFromSource(const char* vertexSource, const char* fragmentSource);

    /**
     * @brief 从二进制数据创建Shader
     * @param binary 二进制数据
     * @return 创建成功返回true，失败返回false
     */
    bool compileFromBinary(const std::vector<uint8_t>& binary);

    /**
     * @brief 获取Shader二进制数据
     * @param outBinary 输出的二进制数据
     * @return 成功返回true，失败返回false
     */
    bool getBinary(std::vector<uint8_t>& outBinary);

    /**
     * @brief 获取OpenGL程序ID
     * @return OpenGL程序ID
     */
    GLuint getProgramID() const { return programID_; }

private:
    GLuint programID_ = 0;
    std::string name_;
    std::unordered_map<std::string, GLint> uniformCache_;

    /**
     * @brief 编译单个着色器
     * @param type 着色器类型
     * @param source 着色器源码
     * @return 着色器ID，失败返回0
     */
    GLuint compileShader(GLenum type, const char* source);

    /**
     * @brief 获取uniform位置
     * @param name uniform变量名
     * @return uniform位置
     */
    GLint getUniformLocation(const std::string& name);
};

// ============================================================================
// OpenGL Shader工厂
// ============================================================================
class GLShaderFactory : public IShaderFactory {
public:
    /**
     * @brief 从源码创建Shader
     * @param name Shader名称
     * @param vertSource 顶点着色器源码
     * @param fragSource 片段着色器源码
     * @return 创建的Shader实例
     */
    Ptr<IShader> createFromSource(
        const std::string& name,
        const std::string& vertSource,
        const std::string& fragSource) override;

    /**
     * @brief 从缓存二进制创建Shader
     * @param name Shader名称
     * @param binary 编译后的二进制数据
     * @return 创建的Shader实例
     */
    Ptr<IShader> createFromBinary(
        const std::string& name,
        const std::vector<uint8_t>& binary) override;

    /**
     * @brief 获取Shader的二进制数据
     * @param shader Shader实例
     * @param outBinary 输出的二进制数据
     * @return 成功返回true，失败返回false
     */
    bool getShaderBinary(const IShader& shader,
                         std::vector<uint8_t>& outBinary) override;
};

} // namespace extra2d
