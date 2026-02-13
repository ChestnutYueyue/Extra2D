// ============================================================================
// Pipes.cpp - 水管管理器实现
// ============================================================================

#include "Pipes.h"
#include "BaseScene.h"

namespace flappybird {

Pipes::Pipes() {
    pipeCount_ = 0;
    moving_ = false;

    // 初始化水管数组
    for (int i = 0; i < maxPipes; ++i) {
        pipes_[i] = nullptr;
    }

    // 注意：不要在构造函数中添加水管
    // 因为此时 weak_from_this() 还不能使用
}

void Pipes::onEnter() {
    Node::onEnter();
    // 在 onEnter 中初始化水管，此时 weak_from_this() 可用
    if (pipeCount_ == 0) {
        addPipe();
        addPipe();
        addPipe();
    }
}

Pipes::~Pipes() = default;

void Pipes::onUpdate(float dt) {
    extra2d::Node::onUpdate(dt);

    if (!moving_) return;

    // 移动所有水管
    for (int i = 0; i < pipeCount_; ++i) {
        if (pipes_[i]) {
            extra2d::Vec2 pos = pipes_[i]->getPosition();
            pos.x -= pipeSpeed * dt;
            pipes_[i]->setPosition(pos);
        }
    }

    // 检查最前面的水管是否移出屏幕
    if (pipes_[0] && pipes_[0]->getPosition().x <= -30.0f) {
        // 移除第一个水管（通过名称查找并移除）
        // 由于 removeChild 需要 Ptr<Node>，我们使用 removeChildByName 或直接操作
        // 这里我们直接移除第一个子节点（假设它是水管）
        auto children = getChildren();
        if (!children.empty()) {
            removeChild(children[0]);
        }
        
        // 将后面的水管前移
        for (int i = 0; i < pipeCount_ - 1; ++i) {
            pipes_[i] = pipes_[i + 1];
        }
        pipes_[pipeCount_ - 1] = nullptr;
        pipeCount_--;

        // 添加新水管
        addPipe();
    }
}

void Pipes::addPipe() {
    if (pipeCount_ >= maxPipes) return;

    // 创建新水管
    auto pipe = extra2d::makePtr<Pipe>();
    
    // 设置水管位置
    if (pipeCount_ == 0) {
        // 第一个水管在屏幕外 130 像素处
        pipe->setPosition(extra2d::Vec2(
            GAME_WIDTH + 130.0f,
            0.0f
        ));
    } else {
        // 其他水管在前一个水管后方
        float prevX = pipes_[pipeCount_ - 1]->getPosition().x;
        pipe->setPosition(extra2d::Vec2(prevX + pipeSpacing, 0.0f));
    }

    // 保存水管指针
    pipes_[pipeCount_] = pipe.get();
    pipeCount_++;

    // 添加到场景
    addChild(pipe);
}

void Pipes::start() {
    moving_ = true;
}

void Pipes::stop() {
    moving_ = false;
}

} // namespace flappybird
