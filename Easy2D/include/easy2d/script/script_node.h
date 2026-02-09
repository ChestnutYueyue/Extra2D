#pragma once

#include <easy2d/scene/node.h>
#include <easy2d/script/script_engine.h>
#include <string>

namespace easy2d {

class ScriptNode : public Node {
public:
    ScriptNode();
    ~ScriptNode() override;

    static Ptr<ScriptNode> create(const std::string& scriptPath);

    bool loadScript(const std::string& scriptPath);
    const std::string& getScriptPath() const { return scriptPath_; }

    void onEnter() override;
    void onExit() override;
    void onUpdate(float dt) override;

private:
    bool callMethod(const char* name);
    bool callMethodWithFloat(const char* name, float arg);
    void pushSelf();

    std::string scriptPath_;
    HSQOBJECT scriptTable_;
    bool tableValid_ = false;
};

} // namespace easy2d
