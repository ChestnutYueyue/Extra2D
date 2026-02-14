#include <extra2d/graphics/opengl/gl_shader.h>
#include <extra2d/utils/logger.h>
#include <fstream>
#include <sstream>

namespace extra2d {

/**
 * @brief 构造函数，初始化着色器程序ID为0
 */
GLShader::GLShader() : programID_(0) {}

/**
 * @brief 析构函数，删除OpenGL着色器程序
 */
GLShader::~GLShader() {
  if (programID_ != 0) {
    glDeleteProgram(programID_);
  }
}

/**
 * @brief 从源代码编译着色器程序
 * @param vertexSource 顶点着色器源代码
 * @param fragmentSource 片段着色器源代码
 * @return 编译成功返回true，失败返回false
 */
bool GLShader::compileFromSource(const char *vertexSource,
                                 const char *fragmentSource) {
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
  if (vertexShader == 0)
    return false;

  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
  if (fragmentShader == 0) {
    glDeleteShader(vertexShader);
    return false;
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
 * @brief 从文件编译着色器程序
 * @param vertexPath 顶点着色器文件路径
 * @param fragmentPath 片段着色器文件路径
 * @return 编译成功返回true，失败返回false
 */
bool GLShader::compileFromFile(const std::string &vertexPath,
                               const std::string &fragmentPath) {
  std::ifstream vShaderFile(vertexPath);
  std::ifstream fShaderFile(fragmentPath);

  if (!vShaderFile.is_open() || !fShaderFile.is_open()) {
    E2D_LOG_ERROR("Failed to open shader files: {}, {}", vertexPath,
                  fragmentPath);
    return false;
  }

  std::stringstream vShaderStream, fShaderStream;
  vShaderStream << vShaderFile.rdbuf();
  fShaderStream << fShaderFile.rdbuf();

  return compileFromSource(vShaderStream.str().c_str(),
                           fShaderStream.str().c_str());
}

/**
 * @brief 绑定（使用）当前着色器程序
 */
void GLShader::bind() const { glUseProgram(programID_); }

/**
 * @brief 解绑当前着色器程序
 */
void GLShader::unbind() const { glUseProgram(0); }

/**
 * @brief 设置布尔类型uniform变量
 * @param name uniform变量名
 * @param value 布尔值
 */
void GLShader::setBool(const std::string &name, bool value) {
  glUniform1i(getUniformLocation(name), value ? 1 : 0);
}

/**
 * @brief 设置整数类型uniform变量
 * @param name uniform变量名
 * @param value 整数值
 */
void GLShader::setInt(const std::string &name, int value) {
  glUniform1i(getUniformLocation(name), value);
}

/**
 * @brief 设置浮点类型uniform变量
 * @param name uniform变量名
 * @param value 浮点值
 */
void GLShader::setFloat(const std::string &name, float value) {
  glUniform1f(getUniformLocation(name), value);
}

/**
 * @brief 设置二维向量类型uniform变量
 * @param name uniform变量名
 * @param value 二维向量值
 */
void GLShader::setVec2(const std::string &name, const glm::vec2 &value) {
  glUniform2fv(getUniformLocation(name), 1, &value[0]);
}

/**
 * @brief 设置三维向量类型uniform变量
 * @param name uniform变量名
 * @param value 三维向量值
 */
void GLShader::setVec3(const std::string &name, const glm::vec3 &value) {
  glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

/**
 * @brief 设置四维向量类型uniform变量
 * @param name uniform变量名
 * @param value 四维向量值
 */
void GLShader::setVec4(const std::string &name, const glm::vec4 &value) {
  glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

/**
 * @brief 设置4x4矩阵类型uniform变量
 * @param name uniform变量名
 * @param value 4x4矩阵值
 */
void GLShader::setMat4(const std::string &name, const glm::mat4 &value) {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

/**
 * @brief 编译单个着色器
 * @param type 着色器类型（GL_VERTEX_SHADER或GL_FRAGMENT_SHADER）
 * @param source 着色器源代码
 * @return 编译成功返回着色器ID，失败返回0
 */
GLuint GLShader::compileShader(GLenum type, const char *source) {
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
 * @brief 获取uniform变量位置，使用缓存避免重复查询
 * @param name uniform变量名
 * @return uniform位置
 */
GLint GLShader::getUniformLocation(const std::string &name) {
  auto it = uniformCache_.find(name);
  if (it != uniformCache_.end()) {
    return it->second;
  }

  GLint location = glGetUniformLocation(programID_, name.c_str());
  uniformCache_[name] = location;
  return location;
}

} // namespace extra2d
