// ============================================================================
// Bird.cpp - 小鸟类实现
// ============================================================================

#include "Bird.h"
#include "ResLoader.h"

namespace flappybird {

Bird::Bird() {
    // 注意：不要在构造函数中调用 initAnimations()
    // 因为此时 weak_from_this() 还不能使用
    setStatus(Status::Idle);
}

void Bird::onEnter() {
    Node::onEnter();
    // 在 onEnter 中初始化动画，此时 weak_from_this() 可用
    if (!animSprite_) {
        initAnimations();
    }
}

Bird::~Bird() = default;

void Bird::initAnimations() {
    // 随机选择小鸟颜色（0-2）
    int colorMode = extra2d::randomInt(0, 2);
    std::string prefix = "bird" + std::to_string(colorMode) + "_";

    // 创建动画片段
    auto clip = extra2d::AnimationClip::create("bird_fly");

    // 添加动画帧序列: 0 -> 1 -> 2 -> 1
    // 注意：每个颜色只有 0, 1, 2 三个帧，没有 3
    int frameSequence[] = {0, 1, 2, 1};
    for (int frameIndex : frameSequence) {
        auto frameSprite = ResLoader::getKeyFrame(prefix + std::to_string(frameIndex));
        if (frameSprite) {
            extra2d::AnimationFrame frame;
            frame.spriteFrame = frameSprite;
            frame.delay = 100.0f;  // 100毫秒 = 0.1秒
            clip->addFrame(std::move(frame));
        } else {
            E2D_LOG_WARN("无法加载动画帧: {}{}", prefix, frameIndex);
        }
    }

    // 创建动画精灵
    if (clip->getFrameCount() > 0) {
        clip->setLooping(true);
        animSprite_ = extra2d::AnimatedSprite::create(clip);
        // 精灵图动画不应应用帧变换（避免覆盖节点位置）
        animSprite_->setApplyFrameTransform(false);
        animSprite_->play();
        addChild(animSprite_);
        E2D_LOG_INFO("小鸟动画创建成功: 颜色={}, 帧数={}, running={}, animSprite父节点={}", 
                     colorMode, clip->getFrameCount(), isRunning(),
                     animSprite_->getParent() ? "有" : "无");
    } else {
        E2D_LOG_ERROR("小鸟动画创建失败: 没有找到任何动画帧");
    }
}

void Bird::onUpdate(float dt) {
    extra2d::Node::onUpdate(dt);

    // 处理闲置动画（上下浮动）
    if (status_ == Status::Idle) {
        idleTimer_ += dt;
        idleOffset_ = std::sin(idleTimer_ * 5.0f) * 4.0f;
    }
}

void Bird::onRender(extra2d::RenderBackend& renderer) {
    // 动画精灵会自动渲染，这里只需要处理旋转和偏移
    if (animSprite_) {
        animSprite_->setRotation(rotation_);

        // 应用闲置偏移
        if (status_ == Status::Idle) {
            animSprite_->setPosition(extra2d::Vec2(0.0f, idleOffset_));
        } else {
            animSprite_->setPosition(extra2d::Vec2(0.0f, 0.0f));
        }
    }

    // 调用父类的 onRender 来渲染子节点
    Node::onRender(renderer);
}

void Bird::fall(float dt) {
    if (!living_) return;

    // 更新垂直位置
    extra2d::Vec2 pos = getPosition();
    pos.y += speed_ * dt;
    setPosition(pos);

    // 应用重力
    speed_ += gravity * dt;

    // 限制顶部边界
    if (pos.y < 0) {
        pos.y = 0;
        setPosition(pos);
        speed_ = 0;
    }

    // 根据速度计算旋转角度
    // 上升时抬头(-15度)，下降时低头(最大90度)
    if (speed_ < 0) {
        rotation_ = -15.0f;
    } else {
        rotation_ = std::min(90.0f, speed_ * 0.15f);
    }
}

void Bird::jump() {
    if (!living_) return;

    // 给小鸟向上的速度
    speed_ = -jumpSpeed;
    
    // 设置状态为飞行
    setStatus(Status::Fly);
    
    // 播放音效
    ResLoader::playMusic(MusicType::Fly);
}

void Bird::die() {
    living_ = false;
    
    // 播放死亡音效
    ResLoader::playMusic(MusicType::Hit);
}

void Bird::setStatus(Status status) {
    status_ = status;

    switch (status) {
        case Status::Still:
            // 停止所有动画
            if (animSprite_) {
                animSprite_->pause();
            }
            break;

        case Status::Idle:
            // 开始闲置动画
            if (animSprite_) {
                animSprite_->setPlaybackSpeed(1.0f);  // 正常速度
                animSprite_->play();
            }
            idleTimer_ = 0.0f;
            break;

        case Status::StartToFly:
            // 停止闲置动画，加速翅膀扇动
            idleOffset_ = 0.0f;
            if (animSprite_) {
                animSprite_->setPlaybackSpeed(2.0f);  // 2倍速度 = 0.05秒每帧
            }
            break;

        case Status::Fly:
            // 飞行状态
            break;

        default:
            break;
    }
}

extra2d::Rect Bird::getBoundingBox() const {
    extra2d::Vec2 pos = getPosition();
    // 小鸟碰撞框大小约为 24x24
    float halfSize = 12.0f;
    return extra2d::Rect(
        pos.x - halfSize,
        pos.y - halfSize,
        halfSize * 2.0f,
        halfSize * 2.0f
    );
}

} // namespace flappybird
