#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace easy2d {

// 前向声明
class Node;

// ============================================================================
// 动画事件类型
// ============================================================================
enum class AnimationEventType : uint32_t {
    FrameChanged   = 0x2001,   // 帧切换
    KeyframeHit    = 0x2002,   // 关键帧触发
    SoundTrigger   = 0x2003,   // 音效触发
    AnimationStart = 0x2004,   // 动画开始播放
    AnimationEnd   = 0x2005,   // 动画播放结束
    AnimationLoop  = 0x2006,   // 动画循环一轮
};

// ============================================================================
// 动画事件数据
// ============================================================================
struct AnimationEvent {
    AnimationEventType type;
    size_t frameIndex = 0;
    size_t previousFrameIndex = 0;
    int keyframeFlag = -1;
    std::string soundPath;
    Node* source = nullptr;
};

// ============================================================================
// 动画事件回调类型
// ============================================================================
using AnimationEventCallback    = std::function<void(const AnimationEvent&)>;
using KeyframeHitCallback       = std::function<void(int flagIndex)>;
using AnimationCompleteCallback = std::function<void()>;

} // namespace easy2d
