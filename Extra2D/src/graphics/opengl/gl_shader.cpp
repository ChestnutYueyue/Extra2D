#include <extra2d/graphics/opengl/gl_shader.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

/**
 * @brief 构造函数，初始化着色器程序ID为0
 */
GLShader::GLShader() : programID_(0) {
}

/**
 * @brief 析构函数，删除OpenGL着色器程序
 */
GLShader::~GLShader() {
    if (programID_ != 0) {
        glDeleteProgram(programID_);
        programID_ = 0;
    }
}

/**
 * @brief 绑定Shader程序
 */
void GLShader::bind() const {
    glUseProgram(programID_);
}

/**
 * @brief 解绑Shader程序
 */
void GLShader::unbind() const {
    glUseProgram(0);
}

/**
 * @brief 设置布尔类型uniform变量
 * @param name uniform变量名
 * @param value 布尔值
 */
void GLShader::setBool(const std::string& name, bool value) {
    glUniform1i(getUniformLocation(name), value ? 1 : 0);
}

/**
 * @brief 设置整数类型uniform变量
 * @param name uniform变量名
 * @param value 整数值
 */
void GLShader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

/**
 * @brief 设置浮点类型uniform变量
 * @param name uniform变量名
 * @param value 浮点值
 */
void GLShader::setFloat(const std::string& name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

/**
 * @brief 设置二维向量类型uniform变量
 * @param name uniform变量名
 * @param value 二维向量值
 */
void GLShader::setVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}

/**
 * @brief 设置三维向量类型uniform变量
 * @param name uniform变量名
 * @param value 三维向量值
 */
void GLShader::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

/**
 * @brief 设置四维向量类型uniform变量
 * @param name uniform变量名
 * @param value 四维向量值
 */
void GLShader::setVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

/**
 * @brief 设置4x4矩阵类型uniform变量
 * @param name uniform变量名
 * @param value 4x4矩阵值
 */
void GLShader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

/**
 * @brief 设置颜色类型uniform变量
 * @param name uniform变量名
 * @param color 颜色值
 */
void GLShader::setColor(const std::string& name, const Color& color) {
    glUniform4f(getUniformLocation(name), color.r, color.g, color.b, color.a);
}

/**
 * @brief 从源码编译Shader
 * @param vertexSource 顶点着色器源码
 * @param fragmentSource 片段着色器源码
 * @return 编译成功返回true，失败返回false
 */
bool GLShader::compileFromSource(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0) {
        return false;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    if (programID_ != 0) {
        glDeleteProgram(programID_);
        uniformCache_.clear();
    }

    programID_ = glCreateProgram();
    glAttachShader(programID_, vertexShader);
    glAttachShader(programID_, fragmentShader);
    glLinkProgram(programID_);

    GLint success;
    glGetProgramiv(programID_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programID_, 512, nullptr, infoLog);
        E2D_LOG_ERROR("Shader program linking failed: {}", infoLog);
        glDeleteProgram(programID_);
        programID_ = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return success == GL_TRUE;
}

/**
 * @brief 从二进制数据创建Shader
 * @param binary 二进制数据
 * @return 创建成功返回true，失败返回false
 */
bool GLShader::compileFromBinary(const std::vector<uint8_t>& binary) {
    if (binary.empty()) {
        E2D_LOG_ERROR("Binary data is empty");
        return false;
    }

    GLint numFormats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numFormats);
    if (numFormats == 0) {
        E2D_LOG_ERROR("Program binary formats not supported");
        return false;
    }

    if (programID_ != 0) {
        glDeleteProgram(programID_);
        uniformCache_.clear();
    }

    programID_ = glCreateProgram();

    GLenum binaryFormat = 0;
    glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, reinterpret_cast<GLint*>(&binaryFormat));

    glProgramBinary(programID_, binaryFormat, binary.data(), static_cast<GLsizei>(binary.size()));

    GLint success = 0;
    glGetProgramiv(programID_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programID_, 512, nullptr, infoLog);
        E2D_LOG_ERROR("Failed to load shader from binary: {}", infoLog);
        glDeleteProgram(programID_);
        programID_ = 0;
        return false;
    }

    return true;
}

/**
 * @brief 获取Shader二进制数据
 * @param outBinary 输出的二进制数据
 * @return 成功返回true，失败返回false
 */
bool GLShader::getBinary(std::vector<uint8_t>& outBinary) {
    if (programID_ == 0) {
        E2D_LOG_WARN("Cannot get binary: shader program is 0");
        return false;
    }

    GLint binaryLength = 0;
    glGetProgramiv(programID_, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
    
    E2D_LOG_DEBUG("Shader binary length: {}", binaryLength);
    
    if (binaryLength <= 0) {
        E2D_LOG_WARN("Shader binary length is 0 or negative");
        return false;
    }

    outBinary.resize(binaryLength);

    GLenum binaryFormat = 0;
    GLsizei actualLength = 0;
    glGetProgramBinary(programID_, binaryLength, &actualLength, &binaryFormat, outBinary.data());
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        E2D_LOG_ERROR("glGetProgramBinary failed with error: {}", err);
        outBinary.clear();
        return false;
    }
    
    if (actualLength == 0) {
        E2D_LOG_WARN("glGetProgramBinary returned 0 bytes");
        outBinary.clear();
        return false;
    }
    
    if (actualLength != binaryLength) {
        outBinary.resize(actualLength);
    }

    E2D_LOG_DEBUG("Shader binary retrieved: {} bytes, format: {}", actualLength, binaryFormat);
    return true;
}

/**
 * @brief 编译单个着色器
 * @param type 着色器类型
 * @param source 着色器源码
 * @return 着色器ID，失败返回0
 */
GLuint GLShader::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        E2D_LOG_ERROR("Shader compilation failed: {}", infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

/**
 * @brief 获取uniform位置
 * @param name uniform变量名
 * @return uniform位置
 */
GLint GLShader::getUniformLocation(const std::string& name) {
    auto it = uniformCache_.find(name);
    if (it != uniformCache_.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(programID_, name.c_str());
    uniformCache_[name] = location;
    return location;
}

// ============================================================================
// GLShaderFactory 实现
// ============================================================================

/**
 * @brief 从源码创建Shader
 * @param name Shader名称
 * @param vertSource 顶点着色器源码
 * @param fragSource 片段着色器源码
 * @return 创建的Shader实例
 */
Ptr<IShader> GLShaderFactory::createFromSource(
    const std::string& name,
    const std::string& vertSource,
    const std::string& fragSource) {
    
    auto shader = std::make_shared<GLShader>();
    shader->setName(name);
    
    if (!shader->compileFromSource(vertSource.c_str(), fragSource.c_str())) {
        E2D_LOG_ERROR("Failed to compile shader from source: {}", name);
        return nullptr;
    }

    return shader;
}

/**
 * @brief 从缓存二进制创建Shader
 * @param name Shader名称
 * @param binary 编译后的二进制数据
 * @return 创建的Shader实例
 */
Ptr<IShader> GLShaderFactory::createFromBinary(
    const std::string& name,
    const std::vector<uint8_t>& binary) {
    
    auto shader = std::make_shared<GLShader>();
    shader->setName(name);
    
    if (!shader->compileFromBinary(binary)) {
        E2D_LOG_ERROR("Failed to create shader from binary: {}", name);
        return nullptr;
    }

    return shader;
}

/**
 * @brief 获取Shader的二进制数据
 * @param shader Shader实例
 * @param outBinary 输出的二进制数据
 * @return 成功返回true，失败返回false
 */
bool GLShaderFactory::getShaderBinary(const IShader& shader, std::vector<uint8_t>& outBinary) {
    const GLShader* glShader = dynamic_cast<const GLShader*>(&shader);
    if (!glShader) {
        E2D_LOG_ERROR("Shader is not a GLShader instance");
        return false;
    }

    return const_cast<GLShader*>(glShader)->getBinary(outBinary);
}

} // namespace extra2d
