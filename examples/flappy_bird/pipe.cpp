// ============================================================================
// Pipe.cpp - 水管类实现
// ============================================================================

#include "Pipe.h"
#include "ResLoader.h"

namespace flappybird {

Pipe::Pipe() {
    scored = false;
    // 注意：不要在构造函数中创建子节点
    // 因为此时 weak_from_this() 还不能使用
}

void Pipe::onEnter() {
    Node::onEnter();
    
    // 在 onEnter 中创建子节点，此时 weak_from_this() 可用
    if (!topPipe_ && !bottomPipe_) {
        auto& app = extra2d::Application::instance();
        float screenHeight = static_cast<float>(app.getConfig().height);

        // 获取地面高度
        auto landFrame = ResLoader::getKeyFrame("land");
        float landHeight = landFrame ? landFrame->getRect().size.height : 112.0f;

        // 随机生成水管高度
        // 范围：与屏幕顶部最小距离不小于 100 像素
        // 与屏幕底部最小距离不小于地面上方 100 像素
        float minHeight = 100.0f;
        float maxHeight = screenHeight - landHeight - 100.0f - gapHeight_;
        float height = static_cast<float>(extra2d::randomInt(static_cast<int>(minHeight), static_cast<int>(maxHeight)));

        // 创建上水管
        auto topFrame = ResLoader::getKeyFrame("pipe_above");
        if (topFrame) {
            topPipe_ = extra2d::Sprite::create(topFrame->getTexture(), topFrame->getRect());
            topPipe_->setAnchor(extra2d::Vec2(0.5f, 1.0f));  // 锚点设在底部中心
            topPipe_->setPosition(extra2d::Vec2(0.0f, height - gapHeight_ / 2.0f));
            addChild(topPipe_);
        }

        // 创建下水管
        auto bottomFrame = ResLoader::getKeyFrame("pipe_below");
        if (bottomFrame) {
            bottomPipe_ = extra2d::Sprite::create(bottomFrame->getTexture(), bottomFrame->getRect());
            bottomPipe_->setAnchor(extra2d::Vec2(0.5f, 0.0f));  // 锚点设在顶部中心
            bottomPipe_->setPosition(extra2d::Vec2(0.0f, height + gapHeight_ / 2.0f));
            addChild(bottomPipe_);
        }
    }
}

Pipe::~Pipe() = default;

extra2d::Rect Pipe::getBoundingBox() const {
    // 返回整个水管的边界框（包含上下两根）
    extra2d::Vec2 pos = getPosition();
    
    // 水管宽度约为 52
    float pipeWidth = 52.0f;
    float halfWidth = pipeWidth / 2.0f;
    
    auto& app = extra2d::Application::instance();
    float screenHeight = static_cast<float>(app.getConfig().height);
    
    return extra2d::Rect(
        pos.x - halfWidth,
        0.0f,
        pipeWidth,
        screenHeight
    );
}

extra2d::Rect Pipe::getTopPipeBox() const {
    if (!topPipe_) return extra2d::Rect();
    
    extra2d::Vec2 pos = getPosition();
    extra2d::Vec2 topPos = topPipe_->getPosition();
    
    // 上水管尺寸
    float pipeWidth = 52.0f;
    float pipeHeight = 320.0f;
    
    return extra2d::Rect(
        pos.x - pipeWidth / 2.0f,
        pos.y + topPos.y - pipeHeight,
        pipeWidth,
        pipeHeight
    );
}

extra2d::Rect Pipe::getBottomPipeBox() const {
    if (!bottomPipe_) return extra2d::Rect();
    
    extra2d::Vec2 pos = getPosition();
    extra2d::Vec2 bottomPos = bottomPipe_->getPosition();
    
    // 下水管尺寸
    float pipeWidth = 52.0f;
    float pipeHeight = 320.0f;
    
    return extra2d::Rect(
        pos.x - pipeWidth / 2.0f,
        pos.y + bottomPos.y,
        pipeWidth,
        pipeHeight
    );
}

} // namespace flappybird
