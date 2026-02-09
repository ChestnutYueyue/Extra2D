#include <easy2d/script/script_node.h>
#include <easy2d/script/sq_binding.h>
#include <easy2d/script/sq_binding_node.h>
#include <easy2d/utils/logger.h>
#include <fstream>
#include <sstream>

namespace easy2d {

// SqClassName needed for pushPtr<Node> which ScriptNode inherits
namespace sq {
    // Already defined in sq_binding_node.cpp, but we need it here for pushPtr
    template<> struct SqClassName<Node> { static const char* name() { return "Node"; } };
}

ScriptNode::ScriptNode() {
    sq_resetobject(&scriptTable_);
}

ScriptNode::~ScriptNode() {
    if (tableValid_) {
        auto& engine = ScriptEngine::getInstance();
        if (engine.isInitialized()) {
            sq_release(engine.getVM(), &scriptTable_);
        }
        tableValid_ = false;
    }
}

Ptr<ScriptNode> ScriptNode::create(const std::string& scriptPath) {
    auto node = makePtr<ScriptNode>();
    if (!node->loadScript(scriptPath)) {
        E2D_ERROR("ScriptNode: failed to load '{}'", scriptPath);
    }
    return node;
}

bool ScriptNode::loadScript(const std::string& scriptPath) {
    scriptPath_ = scriptPath;

    auto& engine = ScriptEngine::getInstance();
    if (!engine.isInitialized()) {
        E2D_ERROR("ScriptNode: ScriptEngine not initialized");
        return false;
    }

    HSQUIRRELVM vm = engine.getVM();

    // Read file
    std::ifstream file(scriptPath);
    if (!file.is_open()) {
        E2D_ERROR("ScriptNode: cannot open '{}'", scriptPath);
        return false;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    SQInteger top = sq_gettop(vm);

    // Compile
    sq_pushroottable(vm);
    if (SQ_FAILED(sq_compilebuffer(vm, source.c_str(),
                                    static_cast<SQInteger>(source.size()),
                                    scriptPath.c_str(), SQTrue))) {
        sq_settop(vm, top);
        return false;
    }

    sq_push(vm, -2); // root table as 'this'

    // Execute — expect a return value (the table)
    if (SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue))) {
        sq_settop(vm, top);
        return false;
    }

    // Check if return value is a table
    if (sq_gettype(vm, -1) == OT_TABLE) {
        sq_getstackobj(vm, -1, &scriptTable_);
        sq_addref(vm, &scriptTable_);
        tableValid_ = true;
    } else {
        E2D_WARN("ScriptNode: '{}' did not return a table", scriptPath);
    }

    sq_settop(vm, top);
    return tableValid_;
}

void ScriptNode::onEnter() {
    Node::onEnter();
    if (tableValid_) {
        auto& engine = ScriptEngine::getInstance();
        HSQUIRRELVM vm = engine.getVM();
        SQInteger top = sq_gettop(vm);

        sq_pushobject(vm, scriptTable_);
        sq_pushstring(vm, "onEnter", -1);
        if (SQ_SUCCEEDED(sq_get(vm, -2))) {
            sq_pushobject(vm, scriptTable_); // 'this' = script table
            pushSelf();                       // arg: node
            sq_call(vm, 2, SQFalse, SQTrue);
            sq_pop(vm, 1); // pop closure
        }

        sq_settop(vm, top);
    }
}

void ScriptNode::onExit() {
    if (tableValid_) {
        auto& engine = ScriptEngine::getInstance();
        HSQUIRRELVM vm = engine.getVM();
        SQInteger top = sq_gettop(vm);

        sq_pushobject(vm, scriptTable_);
        sq_pushstring(vm, "onExit", -1);
        if (SQ_SUCCEEDED(sq_get(vm, -2))) {
            sq_pushobject(vm, scriptTable_);
            pushSelf();
            sq_call(vm, 2, SQFalse, SQTrue);
            sq_pop(vm, 1);
        }

        sq_settop(vm, top);
    }
    Node::onExit();
}

void ScriptNode::onUpdate(float dt) {
    Node::onUpdate(dt);
    if (tableValid_) {
        auto& engine = ScriptEngine::getInstance();
        HSQUIRRELVM vm = engine.getVM();
        SQInteger top = sq_gettop(vm);

        sq_pushobject(vm, scriptTable_);
        sq_pushstring(vm, "onUpdate", -1);
        if (SQ_SUCCEEDED(sq_get(vm, -2))) {
            sq_pushobject(vm, scriptTable_);
            pushSelf();
            sq_pushfloat(vm, dt);
            sq_call(vm, 3, SQFalse, SQTrue);
            sq_pop(vm, 1);
        }

        sq_settop(vm, top);
    }
}

void ScriptNode::pushSelf() {
    auto& engine = ScriptEngine::getInstance();
    HSQUIRRELVM vm = engine.getVM();

    // Push this node as a Node instance with shared_ptr
    auto self = std::dynamic_pointer_cast<Node>(shared_from_this());
    sq::pushPtr(vm, self);
}

bool ScriptNode::callMethod(const char* name) {
    if (!tableValid_) return false;
    auto& engine = ScriptEngine::getInstance();
    HSQUIRRELVM vm = engine.getVM();
    SQInteger top = sq_gettop(vm);

    sq_pushobject(vm, scriptTable_);
    sq_pushstring(vm, name, -1);
    if (SQ_FAILED(sq_get(vm, -2))) {
        sq_settop(vm, top);
        return false;
    }
    sq_pushobject(vm, scriptTable_);
    pushSelf();
    bool ok = SQ_SUCCEEDED(sq_call(vm, 2, SQFalse, SQTrue));
    sq_settop(vm, top);
    return ok;
}

bool ScriptNode::callMethodWithFloat(const char* name, float arg) {
    if (!tableValid_) return false;
    auto& engine = ScriptEngine::getInstance();
    HSQUIRRELVM vm = engine.getVM();
    SQInteger top = sq_gettop(vm);

    sq_pushobject(vm, scriptTable_);
    sq_pushstring(vm, name, -1);
    if (SQ_FAILED(sq_get(vm, -2))) {
        sq_settop(vm, top);
        return false;
    }
    sq_pushobject(vm, scriptTable_);
    pushSelf();
    sq_pushfloat(vm, arg);
    bool ok = SQ_SUCCEEDED(sq_call(vm, 3, SQFalse, SQTrue));
    sq_settop(vm, top);
    return ok;
}

} // namespace easy2d
