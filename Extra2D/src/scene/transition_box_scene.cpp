#include <extra2d/scene/transition_box_scene.h>
#include <extra2d/app/application.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/color.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace extra2d {

TransitionBoxScene::TransitionBoxScene(float duration, Ptr<Scene> inScene,
                                       int divisions)
    : TransitionScene(duration, inScene), divisions_(divisions) {}

Ptr<TransitionBoxScene> TransitionBoxScene::create(float duration,
                                                   Ptr<Scene> inScene,
                                                   int divisions) {
  return makePtr<TransitionBoxScene>(duration, inScene, divisions);
}

void TransitionBoxScene::onTransitionStart() {
  // 方块过渡不需要特殊的初始化
}

void TransitionBoxScene::renderContent(RenderBackend &renderer) {
  // 获取窗口大小
  auto &app = Application::instance();
  float windowWidth = static_cast<float>(app.window().getWidth());
  float windowHeight = static_cast<float>(app.window().getHeight());

  // 更新进度
  elapsed_ += 1.0f / 60.0f;
  progress_ = duration_ > 0.0f ? std::min(1.0f, elapsed_ / duration_) : 1.0f;

  // 先渲染新场景
  if (inScene_) {
    inScene_->renderContent(renderer);
  } else if (outScene_) {
    outScene_->renderContent(renderer);
  }

  // 设置视口为整个窗口
  renderer.setViewport(0, 0, static_cast<int>(windowWidth),
                       static_cast<int>(windowHeight));

  // 计算要显示的方块数量
  int div = std::max(1, divisions_);
  int total = div * div;
  int visible = std::clamp(static_cast<int>(total * progress_), 0, total);

  float cellW = windowWidth / static_cast<float>(div);
  float cellH = windowHeight / static_cast<float>(div);

  // 设置正交投影
  glm::mat4 overlayVP =
      glm::ortho(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f);
  renderer.setViewProjection(overlayVP);

  // 绘制剩余的方块（作为遮罩）
  for (int idx = visible; idx < total; ++idx) {
    int x = idx % div;
    int y = idx / div;
    renderer.fillRect(Rect(x * cellW, y * cellH, cellW + 1.0f, cellH + 1.0f),
                      Colors::Black);
  }

  // 检查是否完成
  if (progress_ >= 1.0f && !isFinished_) {
    finish();
  }
}

} // namespace extra2d
