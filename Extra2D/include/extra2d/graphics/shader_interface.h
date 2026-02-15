#pragma once

#include <extra2d/core/types.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <vector>

namespace extra2d {

class Color;

// ============================================================================
// Shader抽象接口 - 渲染后端无关
// ============================================================================
class IShader {
public:
    virtual ~IShader() = default;

    /**
     * @brief 绑定Shader程序
     */
    virtual void bind() const = 0;

    /**
     * @brief 解绑Shader程序
     */
    virtual void unbind() const = 0;

    /**
     * @brief 设置布尔类型uniform变量
     * @param name uniform变量名
     * @param value 布尔值
     */
    virtual void setBool(const std::string& name, bool value) = 0;

    /**
     * @brief 设置整数类型uniform变量
     * @param name uniform变量名
     * @param value 整数值
     */
    virtual void setInt(const std::string& name, int value) = 0;

    /**
     * @brief 设置浮点类型uniform变量
     * @param name uniform变量名
     * @param value 浮点值
     */
    virtual void setFloat(const std::string& name, float value) = 0;

    /**
     * @brief 设置二维向量类型uniform变量
     * @param name uniform变量名
     * @param value 二维向量值
     */
    virtual void setVec2(const std::string& name, const glm::vec2& value) = 0;

    /**
     * @brief 设置三维向量类型uniform变量
     * @param name uniform变量名
     * @param value 三维向量值
     */
    virtual void setVec3(const std::string& name, const glm::vec3& value) = 0;

    /**
     * @brief 设置四维向量类型uniform变量
     * @param name uniform变量名
     * @param value 四维向量值
     */
    virtual void setVec4(const std::string& name, const glm::vec4& value) = 0;

    /**
     * @brief 设置4x4矩阵类型uniform变量
     * @param name uniform变量名
     * @param value 4x4矩阵值
     */
    virtual void setMat4(const std::string& name, const glm::mat4& value) = 0;

    /**
     * @brief 设置颜色类型uniform变量
     * @param name uniform变量名
     * @param color 颜色值
     */
    virtual void setColor(const std::string& name, const Color& color) = 0;

    /**
     * @brief 检查Shader是否有效
     * @return 有效返回true，否则返回false
     */
    virtual bool isValid() const = 0;

    /**
     * @brief 获取原生句柄（如OpenGL程序ID）
     * @return 原生句柄值
     */
    virtual uint32_t getNativeHandle() const = 0;

    /**
     * @brief 获取Shader名称
     * @return Shader名称
     */
    virtual const std::string& getName() const = 0;

    /**
     * @brief 设置Shader名称
     * @param name Shader名称
     */
    virtual void setName(const std::string& name) = 0;
};

// ============================================================================
// Shader工厂接口 - 用于创建渲染后端特定的Shader实例
// ============================================================================
class IShaderFactory {
public:
    virtual ~IShaderFactory() = default;

    /**
     * @brief 从源码创建Shader
     * @param name Shader名称
     * @param vertSource 顶点着色器源码
     * @param fragSource 片段着色器源码
     * @return 创建的Shader实例
     */
    virtual Ptr<IShader> createFromSource(
        const std::string& name,
        const std::string& vertSource,
        const std::string& fragSource) = 0;

    /**
     * @brief 从缓存二进制创建Shader
     * @param name Shader名称
     * @param binary 编译后的二进制数据
     * @return 创建的Shader实例
     */
    virtual Ptr<IShader> createFromBinary(
        const std::string& name,
        const std::vector<uint8_t>& binary) = 0;

    /**
     * @brief 获取Shader的二进制数据（用于缓存）
     * @param shader Shader实例
     * @param outBinary 输出的二进制数据
     * @return 成功返回true，失败返回false
     */
    virtual bool getShaderBinary(const IShader& shader,
                                 std::vector<uint8_t>& outBinary) = 0;
};

} // namespace extra2d
