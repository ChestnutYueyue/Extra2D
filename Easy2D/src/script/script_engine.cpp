#include <easy2d/script/script_engine.h>
#include <easy2d/script/sq_binding_types.h>
#include <easy2d/script/sq_binding_node.h>
#include <easy2d/script/sq_binding_input.h>
#include <easy2d/script/sq_binding_action.h>
#include <easy2d/script/sq_binding_audio.h>
#include <easy2d/script/sq_binding_animation.h>
#include <easy2d/utils/logger.h>
#include <sqstdaux.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdblob.h>
#include <cstdarg>
#include <fstream>
#include <sstream>

namespace easy2d {

ScriptEngine& ScriptEngine::getInstance() {
    static ScriptEngine instance;
    return instance;
}

ScriptEngine::~ScriptEngine() {
    shutdown();
}

bool ScriptEngine::initialize() {
    if (vm_) return true;

    vm_ = sq_open(1024);
    if (!vm_) {
        E2D_ERROR("ScriptEngine: failed to create Squirrel VM");
        return false;
    }

    sq_setprintfunc(vm_, printFunc, errorFunc);
    sq_setcompilererrorhandler(vm_, compilerError);

    sq_pushroottable(vm_);

    // Register standard libraries
    sqstd_register_mathlib(vm_);
    sqstd_register_stringlib(vm_);
    sqstd_register_bloblib(vm_);
    sqstd_register_iolib(vm_);

    // Set error handler
    sq_newclosure(vm_, errorHandler, 0);
    sq_seterrorhandler(vm_);

    sq_pop(vm_, 1); // pop root table

    // Register Easy2D bindings
    sq::registerValueTypes(vm_);
    sq::registerNodeBindings(vm_);
    sq::registerInputBindings(vm_);
    sq::registerActionBindings(vm_);
    sq::registerAudioBindings(vm_);
    sq::registerAnimationBindings(vm_);

    // Register global log function
    sq_pushroottable(vm_);
    sq_pushstring(vm_, "log", -1);
    sq_newclosure(vm_, [](HSQUIRRELVM v) -> SQInteger {
        const SQChar* msg = nullptr;
        sq_getstring(v, 2, &msg);
        if (msg) E2D_INFO("[Script] {}", msg);
        return 0;
    }, 0);
    sq_newslot(vm_, -3, SQFalse);
    sq_pop(vm_, 1);

    E2D_INFO("ScriptEngine: Squirrel VM initialized (v{})", SQUIRREL_VERSION);
    return true;
}

void ScriptEngine::shutdown() {
    if (vm_) {
        sq_close(vm_);
        vm_ = nullptr;
        E2D_INFO("ScriptEngine: Squirrel VM shut down");
    }
}

bool ScriptEngine::executeString(const std::string& code) {
    if (!vm_) {
        E2D_ERROR("ScriptEngine: VM not initialized");
        return false;
    }
    return compileAndRun(code, "<string>");
}

bool ScriptEngine::executeFile(const std::string& filepath) {
    if (!vm_) {
        E2D_ERROR("ScriptEngine: VM not initialized");
        return false;
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        E2D_ERROR("ScriptEngine: cannot open file '{}'", filepath);
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return compileAndRun(ss.str(), filepath);
}

bool ScriptEngine::compileAndRun(const std::string& source, const std::string& sourceName) {
    SQInteger top = sq_gettop(vm_);

    sq_pushroottable(vm_);

    if (SQ_FAILED(sq_compilebuffer(vm_, source.c_str(),
                                    static_cast<SQInteger>(source.size()),
                                    sourceName.c_str(), SQTrue))) {
        sq_settop(vm_, top);
        return false;
    }

    sq_push(vm_, -2); // push root table as 'this'

    if (SQ_FAILED(sq_call(vm_, 1, SQFalse, SQTrue))) {
        sq_settop(vm_, top);
        return false;
    }

    sq_settop(vm_, top);
    return true;
}

void ScriptEngine::printFunc(HSQUIRRELVM, const SQChar* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[2048];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    E2D_INFO("[Squirrel] {}", buf);
}

void ScriptEngine::errorFunc(HSQUIRRELVM, const SQChar* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[2048];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    E2D_ERROR("[Squirrel] {}", buf);
}

SQInteger ScriptEngine::errorHandler(HSQUIRRELVM vm) {
    const SQChar* errMsg = nullptr;
    if (sq_gettop(vm) >= 1) {
        if (SQ_FAILED(sq_getstring(vm, 2, &errMsg))) {
            errMsg = "unknown error";
        }
    }

    // Print call stack
    SQStackInfos si;
    SQInteger level = 1;
    E2D_ERROR("[Squirrel] Runtime error: {}", errMsg ? errMsg : "unknown");

    while (SQ_SUCCEEDED(sq_stackinfos(vm, level, &si))) {
        const SQChar* fn = si.funcname ? si.funcname : "unknown";
        const SQChar* src = si.source ? si.source : "unknown";
        E2D_ERROR("  [{}] {}:{} in {}", level, src, si.line, fn);
        ++level;
    }

    return 0;
}

void ScriptEngine::compilerError(HSQUIRRELVM, const SQChar* desc,
                                  const SQChar* source, SQInteger line, SQInteger column) {
    E2D_ERROR("[Squirrel] Compile error: {}:{}:{}: {}", source, line, column, desc);
}

} // namespace easy2d
