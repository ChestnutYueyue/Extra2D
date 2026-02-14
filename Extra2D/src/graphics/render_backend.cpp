#include <extra2d/graphics/opengl/gl_renderer.h>
#include <extra2d/graphics/render_backend.h>

namespace extra2d {

/**
 * @brief 创建渲染后端实例
 *
 * 根据指定的后端类型创建对应的渲染后端实例，
 * 目前支持OpenGL后端
 *
 * @param type 渲染后端类型（如OpenGL）
 * @return 成功返回渲染后端的唯一指针，不支持的类型返回nullptr
 */
UniquePtr<RenderBackend> RenderBackend::create(BackendType type) {
  switch (type) {
  case BackendType::OpenGL:
    return makeUnique<GLRenderer>();
  default:
    return nullptr;
  }
}

} // namespace extra2d
