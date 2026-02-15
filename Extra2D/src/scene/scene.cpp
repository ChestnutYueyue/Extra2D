#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_command.h>
#include <extra2d/scene/scene.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

/**
 * @brief 构造函数，初始化场景对象
 *
 * 创建默认相机实例
 */
Scene::Scene() { defaultCamera_ = makePtr<Camera>(); }

/**
 * @brief 设置场景相机
 * @param camera 要设置的相机智能指针
 */
void Scene::setCamera(Ptr<Camera> camera) { camera_ = camera; }

/**
 * @brief 设置视口大小
 * @param width 视口宽度
 * @param height 视口高度
 *
 * 同时更新活动相机的视口参数
 */
void Scene::setViewportSize(float width, float height) {
  viewportSize_ = Size(width, height);
  if (defaultCamera_) {
    defaultCamera_->setViewport(0, width, height, 0);
  } else if (camera_) {
    camera_->setViewport(0, width, height, 0);
  }
}

/**
 * @brief 设置视口大小
 * @param size 视口尺寸结构体
 */
void Scene::setViewportSize(const Size &size) {
  setViewportSize(size.width, size.height);
}

/**
 * @brief 渲染场景
 * @param renderer 渲染后端引用
 *
 * 如果场景不可见则直接返回，否则开始帧渲染、渲染内容并结束帧
 */
void Scene::renderScene(RenderBackend &renderer) {
  if (!isVisible())
    return;

  // Begin frame with background color
  renderer.beginFrame(backgroundColor_);
  renderContent(renderer);
  renderer.endFrame();
}

/**
 * @brief 渲染场景内容
 * @param renderer 渲染后端引用
 *
 * 批量更新节点变换，开始精灵批处理并渲染
 * 注意：视图投影矩阵由 Application 通过 CameraService 设置
 */
void Scene::renderContent(RenderBackend &renderer) {
  if (!isVisible())
    return;

  batchTransforms();

  renderer.beginSpriteBatch();
  render(renderer);
  renderer.endSpriteBatch();
}

/**
 * @brief 更新场景
 * @param dt 帧间隔时间（秒）
 *
 * 如果场景未暂停则调用update方法
 */
void Scene::updateScene(float dt) {
  if (!paused_) {
    update(dt);
  }
}

/**
 * @brief 场景进入时的回调函数
 *
 * 调用父类Node的onEnter方法
 */
void Scene::onEnter() { Node::onEnter(); }

/**
 * @brief 场景退出时的回调函数
 *
 * 调用父类Node的onExit方法
 */
void Scene::onExit() { Node::onExit(); }

/**
 * @brief 收集渲染命令
 * @param commands 渲染命令输出向量
 * @param parentZOrder 父节点的Z序
 *
 * 如果场景不可见则直接返回，否则从场景的子节点开始收集渲染命令
 */
void Scene::collectRenderCommands(std::vector<RenderCommand> &commands,
                                  int parentZOrder) {
  if (!isVisible())
    return;

  // 从场景的子节点开始收集渲染命令
  Node::collectRenderCommands(commands, parentZOrder);
}

/**
 * @brief 创建场景对象
 * @return 新创建的场景智能指针
 */
Ptr<Scene> Scene::create() { return makePtr<Scene>(); }

} // namespace extra2d
