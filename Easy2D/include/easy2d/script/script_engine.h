#pragma once

#include <easy2d/core/types.h>
#include <string>
#include <squirrel.h>

namespace easy2d {

class ScriptEngine {
public:
    static ScriptEngine& getInstance();

    ScriptEngine(const ScriptEngine&) = delete;
    ScriptEngine& operator=(const ScriptEngine&) = delete;

    bool initialize();
    void shutdown();

    bool executeString(const std::string& code);
    bool executeFile(const std::string& filepath);

    HSQUIRRELVM getVM() const { return vm_; }
    bool isInitialized() const { return vm_ != nullptr; }

private:
    ScriptEngine() = default;
    ~ScriptEngine();

    bool compileAndRun(const std::string& source, const std::string& sourceName);

    static void printFunc(HSQUIRRELVM vm, const SQChar* fmt, ...);
    static void errorFunc(HSQUIRRELVM vm, const SQChar* fmt, ...);
    static SQInteger errorHandler(HSQUIRRELVM vm);
    static void compilerError(HSQUIRRELVM vm, const SQChar* desc,
                              const SQChar* source, SQInteger line, SQInteger column);

    HSQUIRRELVM vm_ = nullptr;
};

} // namespace easy2d
