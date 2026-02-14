#include <extra2d/core/color.h>
#include <extra2d/graphics/shader_system.h>
#include <extra2d/utils/logger.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace extra2d {

// ============================================================================
// 内置Shader源码
// ============================================================================

// 标准精灵着色器 (GLES 3.2)
static const char *BUILTIN_SPRITE_VERT = R"(
#version 300 es
precision highp float;
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec4 a_color;

uniform mat4 u_viewProjection;
uniform mat4 u_model;

out vec2 v_texCoord;
out vec4 v_color;

void main() {
    gl_Position = u_viewProjection * u_model * vec4(a_position, 0.0, 1.0);
    v_texCoord = a_texCoord;
    v_color = a_color;
}
)";

static const char *BUILTIN_SPRITE_FRAG = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;
in vec4 v_color;

uniform sampler2D u_texture;
uniform float u_opacity;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(u_texture, v_texCoord);
    fragColor = texColor * v_color;
    fragColor.a *= u_opacity;
    
    if (fragColor.a < 0.01) {
        discard;
    }
}
)";

// 粒子着色器 (GLES 3.2)
static const char *BUILTIN_PARTICLE_VERT = R"(
#version 300 es
precision highp float;
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec4 a_color;
layout(location = 3) in float a_size;
layout(location = 4) in float a_rotation;

uniform mat4 u_viewProjection;

out vec2 v_texCoord;
out vec4 v_color;

void main() {
    float c = cos(a_rotation);
    float s = sin(a_rotation);
    mat2 rot = mat2(c, -s, s, c);
    
    vec2 pos = rot * a_position * a_size;
    gl_Position = u_viewProjection * vec4(pos, 0.0, 1.0);
    
    v_texCoord = a_texCoord;
    v_color = a_color;
}
)";

static const char *BUILTIN_PARTICLE_FRAG = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;
in vec4 v_color;

uniform sampler2D u_texture;
uniform int u_textureEnabled;

out vec4 fragColor;

void main() {
    vec4 color = v_color;
    
    if (u_textureEnabled > 0) {
        color *= texture(u_texture, v_texCoord);
    }
    
    // 圆形粒子
    vec2 center = v_texCoord - vec2(0.5);
    float dist = length(center);
    float alpha = 1.0 - smoothstep(0.4, 0.5, dist);
    color.a *= alpha;
    
    if (color.a < 0.01) {
        discard;
    }
    
    fragColor = color;
}
)";

// 后处理着色器 (GLES 3.2)
static const char *BUILTIN_POSTPROCESS_VERT = R"(
#version 300 es
precision highp float;
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;

out vec2 v_texCoord;

void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_texCoord = a_texCoord;
}
)";

static const char *BUILTIN_POSTPROCESS_FRAG = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform float u_time;

out vec4 fragColor;

void main() {
    fragColor = texture(u_texture, v_texCoord);
}
)";

// 形状渲染着色器 (GLES 3.2)
static const char *BUILTIN_SHAPE_VERT = R"(
#version 300 es
precision highp float;
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec4 a_color;

uniform mat4 u_viewProjection;

out vec4 v_color;

void main() {
    gl_Position = u_viewProjection * vec4(a_position, 0.0, 1.0);
    v_color = a_color;
}
)";

static const char *BUILTIN_SHAPE_FRAG = R"(
#version 300 es
precision highp float;
in vec4 v_color;

out vec4 fragColor;

void main() {
    fragColor = v_color;
}
)";

// ============================================================================
// ShaderSystem实现
// ============================================================================

/**
 * @brief 获取ShaderSystem单例实例
 * @return ShaderSystem单例的引用
 *
 * 使用静态局部变量实现线程安全的单例模式
 */
ShaderSystem &ShaderSystem::get() {
  static ShaderSystem instance;
  return instance;
}

/**
 * @brief 初始化Shader系统
 * @return 初始化成功返回true，失败返回false
 *
 * 加载所有内置着色器，包括精灵、粒子、后处理和形状着色器
 */
bool ShaderSystem::init() {
  E2D_INFO("初始化Shader系统...");

  if (!loadBuiltinShaders()) {
    E2D_ERROR("加载内置Shader失败！");
    return false;
  }

  E2D_INFO("Shader系统初始化完成");
  return true;
}

/**
 * @brief 关闭Shader系统
 *
 * 清理所有着色器资源，释放内置着色器
 */
void ShaderSystem::shutdown() {
  E2D_INFO("关闭Shader系统...");
  clear();

  builtinSpriteShader_.reset();
  builtinParticleShader_.reset();
  builtinPostProcessShader_.reset();
  builtinShapeShader_.reset();
}

/**
 * @brief 加载内置着色器
 * @return 加载成功返回true，失败返回false
 *
 * 编译并加载所有内置着色器：精灵、粒子、后处理和形状着色器
 */
bool ShaderSystem::loadBuiltinShaders() {
  // 加载精灵Shader
  builtinSpriteShader_ = std::make_shared<GLShader>();
  if (!builtinSpriteShader_->compileFromSource(BUILTIN_SPRITE_VERT,
                                               BUILTIN_SPRITE_FRAG)) {
    E2D_ERROR("编译内置精灵Shader失败！");
    return false;
  }

  // 加载粒子Shader
  builtinParticleShader_ = std::make_shared<GLShader>();
  if (!builtinParticleShader_->compileFromSource(BUILTIN_PARTICLE_VERT,
                                                 BUILTIN_PARTICLE_FRAG)) {
    E2D_ERROR("编译内置粒子Shader失败！");
    return false;
  }

  // 加载后处理Shader
  builtinPostProcessShader_ = std::make_shared<GLShader>();
  if (!builtinPostProcessShader_->compileFromSource(BUILTIN_POSTPROCESS_VERT,
                                                    BUILTIN_POSTPROCESS_FRAG)) {
    E2D_ERROR("编译内置后处理Shader失败！");
    return false;
  }

  // 加载形状Shader
  builtinShapeShader_ = std::make_shared<GLShader>();
  if (!builtinShapeShader_->compileFromSource(BUILTIN_SHAPE_VERT,
                                              BUILTIN_SHAPE_FRAG)) {
    E2D_ERROR("编译内置形状Shader失败！");
    return false;
  }

  E2D_INFO("内置Shader加载成功");
  return true;
}

/**
 * @brief 从文件加载着色器
 * @param name 着色器名称
 * @param vertPath 顶点着色器文件路径
 * @param fragPath 片段着色器文件路径
 * @return 加载成功返回着色器指针，失败返回nullptr
 *
 * 从指定路径读取顶点和片段着色器源码并编译
 */
Ptr<GLShader> ShaderSystem::loadFromFile(const std::string &name,
                                         const std::string &vertPath,
                                         const std::string &fragPath) {
  // 读取文件内容
  std::string vertSource = readFile(vertPath);
  std::string fragSource = readFile(fragPath);

  if (vertSource.empty()) {
    E2D_ERROR("无法读取顶点着色器文件: {}", vertPath);
    return nullptr;
  }

  if (fragSource.empty()) {
    E2D_ERROR("无法读取片段着色器文件: {}", fragPath);
    return nullptr;
  }

  // 编译Shader
  auto shader = std::make_shared<GLShader>();
  if (!shader->compileFromSource(vertSource.c_str(), fragSource.c_str())) {
    E2D_ERROR("编译Shader '{}' 失败", name);
    return nullptr;
  }

  // 存储Shader信息
  ShaderInfo info;
  info.shader = shader;
  info.vertPath = vertPath;
  info.fragPath = fragPath;
  info.vertModifiedTime = getFileModifiedTime(vertPath);
  info.fragModifiedTime = getFileModifiedTime(fragPath);
  info.isBuiltin = false;

  shaders_[name] = std::move(info);

  E2D_INFO("加载Shader '{}' 成功", name);
  return shader;
}

/**
 * @brief 从源码加载着色器
 * @param name 着色器名称
 * @param vertSource 顶点着色器源码
 * @param fragSource 片段着色器源码
 * @return 加载成功返回着色器指针，失败返回nullptr
 *
 * 直接从源码字符串编译着色器
 */
Ptr<GLShader> ShaderSystem::loadFromSource(const std::string &name,
                                           const std::string &vertSource,
                                           const std::string &fragSource) {
  auto shader = std::make_shared<GLShader>();
  if (!shader->compileFromSource(vertSource.c_str(), fragSource.c_str())) {
    E2D_ERROR("编译Shader '{}' 失败", name);
    return nullptr;
  }

  ShaderInfo info;
  info.shader = shader;
  info.vertModifiedTime = 0;
  info.fragModifiedTime = 0;
  info.isBuiltin = false;

  shaders_[name] = std::move(info);

  E2D_INFO("加载Shader '{}' 成功", name);
  return shader;
}

/**
 * @brief 获取指定名称的着色器
 * @param name 着色器名称
 * @return 找到返回着色器指针，未找到返回nullptr
 */
Ptr<GLShader> ShaderSystem::get(const std::string &name) {
  auto it = shaders_.find(name);
  if (it != shaders_.end()) {
    return it->second.shader;
  }
  return nullptr;
}

/**
 * @brief 检查指定名称的着色器是否存在
 * @param name 着色器名称
 * @return 存在返回true，不存在返回false
 */
bool ShaderSystem::has(const std::string &name) const {
  return shaders_.find(name) != shaders_.end();
}

/**
 * @brief 移除指定名称的着色器
 * @param name 着色器名称
 *
 * 从着色器管理器中移除指定着色器
 */
void ShaderSystem::remove(const std::string &name) { shaders_.erase(name); }

/**
 * @brief 清空所有着色器
 *
 * 移除所有已加载的着色器
 */
void ShaderSystem::clear() { shaders_.clear(); }

/**
 * @brief 设置文件监视功能
 * @param enable 是否启用
 *
 * 启用或禁用着色器文件变化监视功能
 */
void ShaderSystem::setFileWatching(bool enable) {
  fileWatching_ = enable;
  if (enable) {
    E2D_INFO("启用Shader文件监视");
  } else {
    E2D_INFO("禁用Shader文件监视");
  }
}

/**
 * @brief 更新文件监视
 *
 * 定期检查着色器文件是否发生变化，如有变化则自动重载
 */
void ShaderSystem::updateFileWatching() {
  if (!fileWatching_)
    return;

  watchTimer_ += 0.016f; // 假设60fps
  if (watchTimer_ >= WATCH_INTERVAL) {
    watchTimer_ = 0.0f;
    checkAndReload();
  }
}

/**
 * @brief 检查并重载变化的着色器
 *
 * 检查所有着色器文件的修改时间，如有变化则自动重载
 */
void ShaderSystem::checkAndReload() {
  for (auto &[name, info] : shaders_) {
    if (info.isBuiltin)
      continue;
    if (info.vertPath.empty() || info.fragPath.empty())
      continue;

    uint64_t vertTime = getFileModifiedTime(info.vertPath);
    uint64_t fragTime = getFileModifiedTime(info.fragPath);

    if (vertTime > info.vertModifiedTime || fragTime > info.fragModifiedTime) {
      E2D_INFO("检测到Shader '{}' 文件变化，正在重载...", name);
      reload(name);
    }
  }
}

/**
 * @brief 重载指定着色器
 * @param name 着色器名称
 * @return 重载成功返回true，失败返回false
 *
 * 重新从文件加载并编译指定着色器
 */
bool ShaderSystem::reload(const std::string &name) {
  auto it = shaders_.find(name);
  if (it == shaders_.end()) {
    E2D_ERROR("无法重载不存在的Shader '{}'", name);
    return false;
  }

  auto &info = it->second;
  if (info.isBuiltin) {
    E2D_WARN("无法重载内置Shader '{}'", name);
    return false;
  }

  if (info.vertPath.empty() || info.fragPath.empty()) {
    E2D_ERROR("Shader '{}' 没有关联的文件路径", name);
    return false;
  }

  // 重新加载
  auto newShader = loadFromFile(name, info.vertPath, info.fragPath);
  if (newShader) {
    E2D_INFO("重载Shader '{}' 成功", name);
    return true;
  }

  return false;
}

/**
 * @brief 重载所有着色器
 *
 * 重载所有非内置着色器
 */
void ShaderSystem::reloadAll() {
  for (const auto &[name, info] : shaders_) {
    if (!info.isBuiltin) {
      reload(name);
    }
  }
}

/**
 * @brief 获取内置精灵着色器
 * @return 精灵着色器指针
 */
Ptr<GLShader> ShaderSystem::getBuiltinSpriteShader() {
  return builtinSpriteShader_;
}

/**
 * @brief 获取内置粒子着色器
 * @return 粒子着色器指针
 */
Ptr<GLShader> ShaderSystem::getBuiltinParticleShader() {
  return builtinParticleShader_;
}

/**
 * @brief 获取内置后处理着色器
 * @return 后处理着色器指针
 */
Ptr<GLShader> ShaderSystem::getBuiltinPostProcessShader() {
  return builtinPostProcessShader_;
}

/**
 * @brief 获取内置形状着色器
 * @return 形状着色器指针
 */
Ptr<GLShader> ShaderSystem::getBuiltinShapeShader() {
  return builtinShapeShader_;
}

/**
 * @brief 读取文件内容
 * @param filepath 文件路径
 * @return 文件内容字符串，读取失败返回空字符串
 *
 * 以二进制模式读取文件全部内容
 */
std::string ShaderSystem::readFile(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

/**
 * @brief 获取文件修改时间
 * @param filepath 文件路径
 * @return 文件最后修改时间戳，失败返回0
 *
 * 获取文件的最后修改时间，用于文件监视功能
 */
uint64_t ShaderSystem::getFileModifiedTime(const std::string &filepath) {
#ifdef _WIN32
  struct _stat64 statBuf;
  if (_stat64(filepath.c_str(), &statBuf) == 0) {
    return static_cast<uint64_t>(statBuf.st_mtime);
  }
#else
  struct stat statBuf;
  if (stat(filepath.c_str(), &statBuf) == 0) {
    return static_cast<uint64_t>(statBuf.st_mtime);
  }
#endif
  return 0;
}

// ============================================================================
// ShaderParams实现
// ============================================================================

/**
 * @brief 构造函数
 * @param shader 关联的着色器对象
 *
 * 创建一个着色器参数设置器
 */
ShaderParams::ShaderParams(GLShader &shader) : shader_(shader) {}

/**
 * @brief 设置布尔类型uniform变量
 * @param name 变量名
 * @param value 布尔值
 * @return 当前对象引用，支持链式调用
 */
ShaderParams &ShaderParams::setBool(const std::string &name, bool value) {
  shader_.setBool(name, value);
  return *this;
}

/**
 * @brief 设置整数类型uniform变量
 * @param name 变量名
 * @param value 整数值
 * @return 当前对象引用，支持链式调用
 */
ShaderParams &ShaderParams::setInt(const std::string &name, int value) {
  shader_.setInt(name, value);
  return *this;
}

/**
 * @brief 设置浮点类型uniform变量
 * @param name 变量名
 * @param value 浮点值
 * @return 当前对象引用，支持链式调用
 */
ShaderParams &ShaderParams::setFloat(const std::string &name, float value) {
  shader_.setFloat(name, value);
  return *this;
}

/**
 * @brief 设置二维向量类型uniform变量
 * @param name 变量名
 * @param value 二维向量值
 * @return 当前对象引用，支持链式调用
 */
ShaderParams &ShaderParams::setVec2(const std::string &name,
                                    const glm::vec2 &value) {
  shader_.setVec2(name, value);
  return *this;
}

/**
 * @brief 设置三维向量类型uniform变量
 * @param name 变量名
 * @param value 三维向量值
 * @return 当前对象引用，支持链式调用
 */
ShaderParams &ShaderParams::setVec3(const std::string &name,
                                    const glm::vec3 &value) {
  shader_.setVec3(name, value);
  return *this;
}

/**
 * @brief 设置四维向量类型uniform变量
 * @param name 变量名
 * @param value 四维向量值
 * @return 当前对象引用，支持链式调用
 */
ShaderParams &ShaderParams::setVec4(const std::string &name,
                                    const glm::vec4 &value) {
  shader_.setVec4(name, value);
  return *this;
}

/**
 * @brief 设置4x4矩阵类型uniform变量
 * @param name 变量名
 * @param value 4x4矩阵值
 * @return 当前对象引用，支持链式调用
 */
ShaderParams &ShaderParams::setMat4(const std::string &name,
                                    const glm::mat4 &value) {
  shader_.setMat4(name, value);
  return *this;
}

/**
 * @brief 设置颜色类型uniform变量
 * @param name 变量名
 * @param color 颜色值
 * @return 当前对象引用，支持链式调用
 *
 * 将颜色对象转换为四维向量并设置
 */
ShaderParams &ShaderParams::setColor(const std::string &name,
                                     const Color &color) {
  shader_.setVec4(name, glm::vec4(color.r, color.g, color.b, color.a));
  return *this;
}

} // namespace extra2d
