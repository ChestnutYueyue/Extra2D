#include <extra2d/graphics/opengl/gl_renderer.h>
#include <extra2d/graphics/render_backend.h>

namespace extra2d {

UniquePtr<RenderBackend> RenderBackend::create(BackendType type) {
  switch (type) {
  case BackendType::OpenGL:
    return makeUnique<GLRenderer>();
  default:
    return nullptr;
  }
}

} // namespace extra2d
