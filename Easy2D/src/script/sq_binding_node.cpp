#include <easy2d/script/sq_binding_node.h>
#include <easy2d/script/sq_binding_types.h>
#include <easy2d/scene/node.h>
#include <easy2d/scene/sprite.h>
#include <easy2d/scene/scene.h>
#include <easy2d/scene/scene_manager.h>
#include <easy2d/app/application.h>
#include <easy2d/graphics/texture.h>
#include <easy2d/resource/resource_manager.h>

namespace easy2d {
namespace sq {

// SqClassName specializations
template<> struct SqClassName<Node>   { static const char* name() { return "Node"; } };
template<> struct SqClassName<Sprite> { static const char* name() { return "Sprite"; } };
template<> struct SqClassName<Scene>  { static const char* name() { return "Scene"; } };

// ============================================================================
// Node
// ============================================================================
static SQInteger nodeCreate(HSQUIRRELVM vm) {
    auto node = makePtr<Node>();
    pushPtr(vm, node);
    return 1;
}

static SQInteger nodeSetPosition(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");

    SQInteger argc = sq_gettop(vm);
    if (argc >= 3) {
        float x = static_cast<float>(getFloat(vm, 2));
        float y = static_cast<float>(getFloat(vm, 3));
        node->setPosition(x, y);
    } else {
        Vec2* v = getValueInstance<Vec2>(vm, 2);
        if (v) node->setPosition(*v);
    }
    return 0;
}

static SQInteger nodeGetPosition(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    pushValueInstance(vm, node->getPosition());
    return 1;
}

static SQInteger nodeSetRotation(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->setRotation(static_cast<float>(getFloat(vm, 2)));
    return 0;
}

static SQInteger nodeGetRotation(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    push(vm, node->getRotation());
    return 1;
}

static SQInteger nodeSetScale(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");

    SQInteger argc = sq_gettop(vm);
    if (argc >= 3) {
        node->setScale(static_cast<float>(getFloat(vm, 2)), static_cast<float>(getFloat(vm, 3)));
    } else {
        // Could be float or Vec2
        SQObjectType t = sq_gettype(vm, 2);
        if (t == OT_INSTANCE) {
            Vec2* v = getValueInstance<Vec2>(vm, 2);
            if (v) node->setScale(*v);
        } else {
            node->setScale(static_cast<float>(getFloat(vm, 2)));
        }
    }
    return 0;
}

static SQInteger nodeGetScale(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    pushValueInstance(vm, node->getScale());
    return 1;
}

static SQInteger nodeSetAnchor(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    SQInteger argc = sq_gettop(vm);
    if (argc >= 3) {
        node->setAnchor(static_cast<float>(getFloat(vm, 2)), static_cast<float>(getFloat(vm, 3)));
    } else {
        Vec2* v = getValueInstance<Vec2>(vm, 2);
        if (v) node->setAnchor(*v);
    }
    return 0;
}

static SQInteger nodeGetAnchor(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    pushValueInstance(vm, node->getAnchor());
    return 1;
}

static SQInteger nodeSetOpacity(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->setOpacity(static_cast<float>(getFloat(vm, 2)));
    return 0;
}

static SQInteger nodeGetOpacity(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    push(vm, node->getOpacity());
    return 1;
}

static SQInteger nodeSetVisible(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->setVisible(getBool(vm, 2));
    return 0;
}

static SQInteger nodeIsVisible(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    push(vm, node->isVisible());
    return 1;
}

static SQInteger nodeSetZOrder(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->setZOrder(static_cast<int>(getInt(vm, 2)));
    return 0;
}

static SQInteger nodeGetZOrder(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    push(vm, node->getZOrder());
    return 1;
}

static SQInteger nodeSetName(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->setName(getString(vm, 2));
    return 0;
}

static SQInteger nodeGetName(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    push(vm, node->getName());
    return 1;
}

static SQInteger nodeSetTag(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->setTag(static_cast<int>(getInt(vm, 2)));
    return 0;
}

static SQInteger nodeGetTag(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    push(vm, node->getTag());
    return 1;
}

static SQInteger nodeAddChild(HSQUIRRELVM vm) {
    auto parent = getPtr<Node>(vm, 1);
    auto child = getPtr<Node>(vm, 2);
    if (!parent || !child) return sq_throwerror(vm, "null node");
    parent->addChild(child);
    return 0;
}

static SQInteger nodeRemoveFromParent(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->removeFromParent();
    return 0;
}

static SQInteger nodeRemoveAllChildren(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->removeAllChildren();
    return 0;
}

static SQInteger nodeGetChildByName(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    auto child = node->getChildByName(getString(vm, 2));
    if (child) pushPtr(vm, child);
    else pushNull(vm);
    return 1;
}

static SQInteger nodeRunAction(HSQUIRRELVM vm) {
    // Will be implemented in action bindings
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    // Get action pointer from userdata
    SQUserPointer up = nullptr;
    sq_getinstanceup(vm, 2, &up, nullptr, SQFalse);
    if (!up) return sq_throwerror(vm, "null action");
    auto actionPtr = *static_cast<Ptr<Action>*>(up);
    node->runAction(actionPtr);
    return 0;
}

static SQInteger nodeStopAllActions(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    node->stopAllActions();
    return 0;
}

static SQInteger nodeGetBoundingBox(HSQUIRRELVM vm) {
    auto* node = getRawPtr<Node>(vm, 1);
    if (!node) return sq_throwerror(vm, "null node");
    pushValueInstance(vm, node->getBoundingBox());
    return 1;
}

void registerNode(HSQUIRRELVM vm) {
    // Node class — uses shared_ptr bridge
    ClassDef(vm, "Node")
        .setTypeTag(typeTag<Node>())
        .staticMethod("create", nodeCreate)
        .method("setPosition", nodeSetPosition)
        .method("getPosition", nodeGetPosition)
        .method("setRotation", nodeSetRotation)
        .method("getRotation", nodeGetRotation)
        .method("setScale", nodeSetScale)
        .method("getScale", nodeGetScale)
        .method("setAnchor", nodeSetAnchor)
        .method("getAnchor", nodeGetAnchor)
        .method("setOpacity", nodeSetOpacity)
        .method("getOpacity", nodeGetOpacity)
        .method("setVisible", nodeSetVisible)
        .method("isVisible", nodeIsVisible)
        .method("setZOrder", nodeSetZOrder)
        .method("getZOrder", nodeGetZOrder)
        .method("setName", nodeSetName)
        .method("getName", nodeGetName)
        .method("setTag", nodeSetTag)
        .method("getTag", nodeGetTag)
        .method("addChild", nodeAddChild)
        .method("removeFromParent", nodeRemoveFromParent)
        .method("removeAllChildren", nodeRemoveAllChildren)
        .method("getChildByName", nodeGetChildByName)
        .method("runAction", nodeRunAction)
        .method("stopAllActions", nodeStopAllActions)
        .method("getBoundingBox", nodeGetBoundingBox)
        .commit();
}

// ============================================================================
// Sprite
// ============================================================================
static SQInteger spriteCreate(HSQUIRRELVM vm) {
    SQInteger argc = sq_gettop(vm);
    Ptr<Sprite> sprite;

    if (argc >= 2 && sq_gettype(vm, 2) == OT_STRING) {
        std::string path = getString(vm, 2);
        auto tex = Application::instance().resources().loadTexture(path);
        if (tex) sprite = Sprite::create(tex);
        else sprite = Sprite::create();
    } else {
        sprite = Sprite::create();
    }

    pushPtr<Node>(vm, sprite);
    return 1;
}

static SQInteger spriteSetColor(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* sprite = dynamic_cast<Sprite*>(node.get());
    if (!sprite) return sq_throwerror(vm, "not a sprite");
    Color* c = getValueInstance<Color>(vm, 2);
    if (c) sprite->setColor(*c);
    return 0;
}

static SQInteger spriteGetColor(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* sprite = dynamic_cast<Sprite*>(node.get());
    if (!sprite) return sq_throwerror(vm, "not a sprite");
    pushValueInstance(vm, sprite->getColor());
    return 1;
}

static SQInteger spriteSetFlipX(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* sprite = dynamic_cast<Sprite*>(node.get());
    if (!sprite) return sq_throwerror(vm, "not a sprite");
    sprite->setFlipX(getBool(vm, 2));
    return 0;
}

static SQInteger spriteSetFlipY(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* sprite = dynamic_cast<Sprite*>(node.get());
    if (!sprite) return sq_throwerror(vm, "not a sprite");
    sprite->setFlipY(getBool(vm, 2));
    return 0;
}

static SQInteger spriteIsFlipX(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* sprite = dynamic_cast<Sprite*>(node.get());
    if (!sprite) return sq_throwerror(vm, "not a sprite");
    push(vm, sprite->isFlipX());
    return 1;
}

static SQInteger spriteIsFlipY(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* sprite = dynamic_cast<Sprite*>(node.get());
    if (!sprite) return sq_throwerror(vm, "not a sprite");
    push(vm, sprite->isFlipY());
    return 1;
}

void registerSprite(HSQUIRRELVM vm) {
    ClassDef(vm, "Sprite", "Node")
        .setTypeTag(typeTag<Sprite>())
        .staticMethod("create", spriteCreate)
        .method("setColor", spriteSetColor)
        .method("getColor", spriteGetColor)
        .method("setFlipX", spriteSetFlipX)
        .method("setFlipY", spriteSetFlipY)
        .method("isFlipX", spriteIsFlipX)
        .method("isFlipY", spriteIsFlipY)
        .commit();
}

// ============================================================================
// Scene
// ============================================================================
static SQInteger sceneCreate(HSQUIRRELVM vm) {
    auto scene = Scene::create();
    pushPtr<Node>(vm, scene);
    return 1;
}

static SQInteger sceneSetBackgroundColor(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* scene = dynamic_cast<Scene*>(node.get());
    if (!scene) return sq_throwerror(vm, "not a scene");
    Color* c = getValueInstance<Color>(vm, 2);
    if (c) scene->setBackgroundColor(*c);
    return 0;
}

static SQInteger sceneGetBackgroundColor(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 1);
    auto* scene = dynamic_cast<Scene*>(node.get());
    if (!scene) return sq_throwerror(vm, "not a scene");
    pushValueInstance(vm, scene->getBackgroundColor());
    return 1;
}

void registerScene(HSQUIRRELVM vm) {
    ClassDef(vm, "Scene", "Node")
        .setTypeTag(typeTag<Scene>())
        .staticMethod("create", sceneCreate)
        .method("setBackgroundColor", sceneSetBackgroundColor)
        .method("getBackgroundColor", sceneGetBackgroundColor)
        .commit();
}

// ============================================================================
// SceneManager (global "Scenes" variable)
// ============================================================================
static SQInteger smRunWithScene(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 2);
    auto scene = std::dynamic_pointer_cast<Scene>(node);
    if (!scene) return sq_throwerror(vm, "expected Scene");
    SceneManager::getInstance().runWithScene(scene);
    return 0;
}

static SQInteger smReplaceScene(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 2);
    auto scene = std::dynamic_pointer_cast<Scene>(node);
    if (!scene) return sq_throwerror(vm, "expected Scene");
    SceneManager::getInstance().replaceScene(scene);
    return 0;
}

static SQInteger smPushScene(HSQUIRRELVM vm) {
    auto node = getPtr<Node>(vm, 2);
    auto scene = std::dynamic_pointer_cast<Scene>(node);
    if (!scene) return sq_throwerror(vm, "expected Scene");
    SceneManager::getInstance().pushScene(scene);
    return 0;
}

static SQInteger smPopScene(HSQUIRRELVM vm) {
    SceneManager::getInstance().popScene();
    return 0;
}

static SQInteger smGetCurrentScene(HSQUIRRELVM vm) {
    auto scene = SceneManager::getInstance().getCurrentScene();
    if (scene) pushPtr<Node>(vm, scene);
    else pushNull(vm);
    return 1;
}

void registerSceneManager(HSQUIRRELVM vm) {
    ClassDef(vm, "SceneManagerClass")
        .method("runWithScene", smRunWithScene)
        .method("replaceScene", smReplaceScene)
        .method("pushScene", smPushScene)
        .method("popScene", smPopScene)
        .method("getCurrentScene", smGetCurrentScene)
        .commit();

    // Create global "Scenes" instance
    pushSingleton(vm, &SceneManager::getInstance(), "SceneManagerClass");
    sq_pushroottable(vm);
    sq_pushstring(vm, "Scenes", -1);
    sq_push(vm, -3);       // push the instance
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 2);         // pop root table and instance
}

// ============================================================================
// Application (global "App" variable)
// ============================================================================
static SQInteger appQuit(HSQUIRRELVM vm) {
    Application::instance().quit();
    return 0;
}

static SQInteger appGetDeltaTime(HSQUIRRELVM vm) {
    push(vm, Application::instance().deltaTime());
    return 1;
}

static SQInteger appGetTotalTime(HSQUIRRELVM vm) {
    push(vm, Application::instance().totalTime());
    return 1;
}

static SQInteger appGetFps(HSQUIRRELVM vm) {
    push(vm, Application::instance().fps());
    return 1;
}

static SQInteger appIsPaused(HSQUIRRELVM vm) {
    push(vm, Application::instance().isPaused());
    return 1;
}

void registerApplication(HSQUIRRELVM vm) {
    ClassDef(vm, "ApplicationClass")
        .method("quit", appQuit)
        .method("getDeltaTime", appGetDeltaTime)
        .method("getTotalTime", appGetTotalTime)
        .method("getFps", appGetFps)
        .method("isPaused", appIsPaused)
        .commit();

    // Global "App" instance
    pushSingleton(vm, &Application::instance(), "ApplicationClass");
    sq_pushroottable(vm);
    sq_pushstring(vm, "App", -1);
    sq_push(vm, -3);
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 2);
}

// ============================================================================
// Register all
// ============================================================================
void registerNodeBindings(HSQUIRRELVM vm) {
    registerNode(vm);
    registerSprite(vm);
    registerScene(vm);
    registerSceneManager(vm);
    registerApplication(vm);
}

} // namespace sq
} // namespace easy2d
