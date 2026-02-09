#include <easy2d/script/sq_binding_animation.h>
#include <easy2d/script/sq_binding_node.h>
#include <easy2d/script/sq_binding_types.h>
#include <easy2d/animation/animated_sprite.h>
#include <easy2d/animation/animation_cache.h>
#include <easy2d/animation/animation_clip.h>
#include <easy2d/app/application.h>
#include <easy2d/resource/resource_manager.h>
#include <easy2d/graphics/texture.h>
#include <easy2d/utils/logger.h>

namespace easy2d {
namespace sq {

// SqClassName specialization for AnimatedSprite
template<> struct SqClassName<AnimatedSprite> { static const char* name() { return "AnimatedSprite"; } };

// AnimatedSprite inherits Sprite -> Node, all stored as Ptr<Node>

// 从精灵图创建 AnimatedSprite（网格布局）
// AnimatedSprite.createFromGrid(texturePath, frameWidth, frameHeight, frameDurationMs, frameCount)
static SQInteger animSpriteCreateFromGrid(HSQUIRRELVM vm) {
    SQInteger argc = sq_gettop(vm);
    if (argc < 4) {
        return sq_throwerror(vm, "createFromGrid requires at least 3 arguments: texturePath, frameWidth, frameHeight");
    }
    
    std::string path = getString(vm, 2);
    int frameWidth = static_cast<int>(getInt(vm, 3));
    int frameHeight = static_cast<int>(getInt(vm, 4));
    
    float frameDurationMs = 100.0f;
    if (argc >= 5) {
        frameDurationMs = static_cast<float>(getFloat(vm, 5));
    }
    
    int frameCount = -1;
    if (argc >= 6) {
        frameCount = static_cast<int>(getInt(vm, 6));
    }
    
    // 加载纹理
    auto& resources = Application::instance().resources();
    auto texture = resources.loadTexture(path);
    if (!texture) {
        E2D_ERROR("Failed to load texture: {}", path);
        pushNull(vm);
        return 1;
    }
    
    // 创建动画片段
    auto clip = AnimationClip::createFromGrid(texture, frameWidth, frameHeight, frameDurationMs, frameCount);
    if (!clip || clip->empty()) {
        E2D_ERROR("Failed to create animation clip from grid");
        pushNull(vm);
        return 1;
    }
    clip->setLooping(true);
    
    // 创建 AnimatedSprite
    auto sprite = AnimatedSprite::create(clip);
    sprite->setApplyFrameTransform(false);
    
    pushPtr<AnimatedSprite>(vm, sprite);
    return 1;
}

static SQInteger animSpriteCreate(HSQUIRRELVM vm) {
    SQInteger argc = sq_gettop(vm);
    Ptr<AnimatedSprite> sprite;
    if (argc >= 2 && sq_gettype(vm, 2) == OT_STRING) {
        std::string path = getString(vm, 2);
        sprite = AnimatedSprite::create(path);
    } else {
        sprite = AnimatedSprite::create();
    }
    pushPtr<AnimatedSprite>(vm, sprite);
    return 1;
}

static SQInteger animSpritePlay(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");

    SQInteger argc = sq_gettop(vm);
    if (argc >= 2 && sq_gettype(vm, 2) == OT_STRING) {
        std::string name = getString(vm, 2);
        bool loop = true;
        if (argc >= 3) loop = getBool(vm, 3);
        anim->play(name, loop);
    } else {
        anim->play();
    }
    return 0;
}

static SQInteger animSpritePause(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    anim->pause();
    return 0;
}

static SQInteger animSpriteResume(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    anim->resume();
    return 0;
}

static SQInteger animSpriteStop(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    anim->stop();
    return 0;
}

static SQInteger animSpriteReset(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    anim->reset();
    return 0;
}

static SQInteger animSpriteIsPlaying(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    push(vm, anim->isPlaying());
    return 1;
}

static SQInteger animSpriteSetLooping(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    anim->setLooping(getBool(vm, 2));
    return 0;
}

static SQInteger animSpriteSetPlaybackSpeed(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    anim->setPlaybackSpeed(static_cast<float>(getFloat(vm, 2)));
    return 0;
}

static SQInteger animSpriteAddAnimation(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    std::string name = getString(vm, 2);
    std::string path = getString(vm, 3);
    auto clip = AnimationCache::getInstance().loadClip(path);
    if (clip) anim->addAnimation(name, clip);
    return 0;
}

static SQInteger animSpriteLoadAnimation(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    std::string path = getString(vm, 2);
    anim->loadAnimation(path);
    return 0;
}

static SQInteger animSpriteSetAutoPlay(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    anim->setAutoPlay(getBool(vm, 2));
    return 0;
}

static SQInteger animSpriteGetCurrentFrameIndex(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    push(vm, static_cast<int>(anim->getCurrentFrameIndex()));
    return 1;
}

static SQInteger animSpriteGetTotalFrames(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    push(vm, static_cast<int>(anim->getTotalFrames()));
    return 1;
}

static SQInteger animSpriteSetFrameRange(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    int start = static_cast<int>(getInt(vm, 2));
    int end = -1;
    if (sq_gettop(vm) >= 3) {
        end = static_cast<int>(getInt(vm, 3));
    }
    anim->setFrameRange(start, end);
    return 0;
}

static SQInteger animSpriteSetFrameIndex(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    int index = static_cast<int>(getInt(vm, 2));
    anim->setFrameIndex(static_cast<size_t>(index));
    return 0;
}

static SQInteger animSpriteSetApplyFrameTransform(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* anim = dynamic_cast<AnimatedSprite*>(node.get());
    if (!anim) return sq_throwerror(vm, "not an AnimatedSprite");
    anim->setApplyFrameTransform(getBool(vm, 2));
    return 0;
}

void registerAnimationBindings(HSQUIRRELVM vm) {
    // Register SqClassName for AnimatedSprite
    ClassDef(vm, "AnimatedSprite", "Sprite")
        .setTypeTag(typeTag<AnimatedSprite>())
        .staticMethod("create", animSpriteCreate)
        .staticMethod("createFromGrid", animSpriteCreateFromGrid)
        .method("play", animSpritePlay)
        .method("pause", animSpritePause)
        .method("resume", animSpriteResume)
        .method("stop", animSpriteStop)
        .method("reset", animSpriteReset)
        .method("isPlaying", animSpriteIsPlaying)
        .method("setLooping", animSpriteSetLooping)
        .method("setPlaybackSpeed", animSpriteSetPlaybackSpeed)
        .method("addAnimation", animSpriteAddAnimation)
        .method("loadAnimation", animSpriteLoadAnimation)
        .method("setAutoPlay", animSpriteSetAutoPlay)
        .method("getCurrentFrameIndex", animSpriteGetCurrentFrameIndex)
        .method("getTotalFrames", animSpriteGetTotalFrames)
        .method("setFrameRange", animSpriteSetFrameRange)
        .method("setFrameIndex", animSpriteSetFrameIndex)
        .method("setApplyFrameTransform", animSpriteSetApplyFrameTransform)
        .commit();
}

} // namespace sq
} // namespace easy2d
