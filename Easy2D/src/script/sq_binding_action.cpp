#include <easy2d/script/sq_binding_action.h>
#include <easy2d/script/sq_binding_types.h>
#include <easy2d/action/action.h>
#include <easy2d/action/actions.h>

namespace easy2d {
namespace sq {

// Actions are stored as Ptr<Action> in userdata (they use raw new in C++ API but
// we wrap them in shared_ptr for the script bridge)

template<> struct SqClassName<Action> { static const char* name() { return "Action"; } };

// Helper: wrap a raw Action* into Ptr<Action> and push it
static void pushAction(HSQUIRRELVM vm, Action* raw) {
    Ptr<Action> ptr(raw);

    sq_pushroottable(vm);
    sq_pushstring(vm, "Action", -1);
    if (SQ_FAILED(sq_get(vm, -2))) {
        sq_pop(vm, 1);
        sq_pushnull(vm);
        return;
    }
    if (SQ_FAILED(sq_createinstance(vm, -1))) {
        sq_pop(vm, 2);
        sq_pushnull(vm);
        return;
    }

    auto* storage = new Ptr<Action>(std::move(ptr));
    sq_setinstanceup(vm, -1, storage);
    sq_setreleasehook(vm, -1, [](SQUserPointer p, SQInteger) -> SQInteger {
        delete static_cast<Ptr<Action>*>(p);
        return 0;
    });

    sq_remove(vm, -2); // class
    sq_remove(vm, -2); // roottable
}

static Ptr<Action> getAction(HSQUIRRELVM vm, SQInteger idx) {
    SQUserPointer up = nullptr;
    sq_getinstanceup(vm, idx, &up, nullptr, SQFalse);
    if (!up) return nullptr;
    return *static_cast<Ptr<Action>*>(up);
}

// ============================================================================
// Factory functions (registered as global functions)
// ============================================================================
static SQInteger sqMoveTo(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    Vec2* pos = getValueInstance<Vec2>(vm, 3);
    if (!pos) return sq_throwerror(vm, "expected Vec2");
    pushAction(vm, new MoveTo(dur, *pos));
    return 1;
}

static SQInteger sqMoveBy(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    Vec2* delta = getValueInstance<Vec2>(vm, 3);
    if (!delta) return sq_throwerror(vm, "expected Vec2");
    pushAction(vm, new MoveBy(dur, *delta));
    return 1;
}

static SQInteger sqScaleTo(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    SQInteger argc = sq_gettop(vm);
    if (argc >= 4) {
        float sx = static_cast<float>(getFloat(vm, 3));
        float sy = static_cast<float>(getFloat(vm, 4));
        pushAction(vm, new ScaleTo(dur, sx, sy));
    } else {
        float s = static_cast<float>(getFloat(vm, 3));
        pushAction(vm, new ScaleTo(dur, s));
    }
    return 1;
}

static SQInteger sqRotateTo(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    float angle = static_cast<float>(getFloat(vm, 3));
    pushAction(vm, new RotateTo(dur, angle));
    return 1;
}

static SQInteger sqRotateBy(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    float angle = static_cast<float>(getFloat(vm, 3));
    pushAction(vm, new RotateBy(dur, angle));
    return 1;
}

static SQInteger sqFadeIn(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    pushAction(vm, new FadeIn(dur));
    return 1;
}

static SQInteger sqFadeOut(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    pushAction(vm, new FadeOut(dur));
    return 1;
}

static SQInteger sqFadeTo(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    float opacity = static_cast<float>(getFloat(vm, 3));
    pushAction(vm, new FadeTo(dur, opacity));
    return 1;
}

static SQInteger sqDelay(HSQUIRRELVM vm) {
    float dur = static_cast<float>(getFloat(vm, 2));
    pushAction(vm, new Delay(dur));
    return 1;
}

static SQInteger sqSequence(HSQUIRRELVM vm) {
    // arg 2 is an array of actions
    SQInteger size = sq_getsize(vm, 2);
    std::vector<Action*> actions;
    actions.reserve(size);
    for (SQInteger i = 0; i < size; ++i) {
        sq_pushinteger(vm, i);
        sq_get(vm, 2);
        auto a = getAction(vm, -1);
        if (a) actions.push_back(a->clone());
        sq_pop(vm, 1);
    }
    if (actions.empty()) return sq_throwerror(vm, "empty sequence");
    pushAction(vm, new Sequence(actions));
    return 1;
}

static SQInteger sqSpawn(HSQUIRRELVM vm) {
    SQInteger size = sq_getsize(vm, 2);
    std::vector<Action*> actions;
    actions.reserve(size);
    for (SQInteger i = 0; i < size; ++i) {
        sq_pushinteger(vm, i);
        sq_get(vm, 2);
        auto a = getAction(vm, -1);
        if (a) actions.push_back(a->clone());
        sq_pop(vm, 1);
    }
    if (actions.empty()) return sq_throwerror(vm, "empty spawn");
    pushAction(vm, new Spawn(actions));
    return 1;
}

static SQInteger sqLoop(HSQUIRRELVM vm) {
    auto a = getAction(vm, 2);
    if (!a) return sq_throwerror(vm, "null action");
    int times = -1;
    if (sq_gettop(vm) >= 3)
        times = static_cast<int>(getInt(vm, 3));
    pushAction(vm, new Loop(a->clone(), times));
    return 1;
}

static SQInteger sqCallFunc(HSQUIRRELVM vm) {
    // arg 2 is a Squirrel closure
    HSQOBJECT closure;
    sq_resetobject(&closure);
    sq_getstackobj(vm, 2, &closure);
    sq_addref(vm, &closure);

    HSQUIRRELVM capturedVM = vm;
    HSQOBJECT capturedClosure = closure;

    pushAction(vm, new CallFunc([capturedVM, capturedClosure]() mutable {
        sq_pushobject(capturedVM, capturedClosure);
        sq_pushroottable(capturedVM);
        sq_call(capturedVM, 1, SQFalse, SQTrue);
        sq_pop(capturedVM, 1); // pop closure
    }));
    return 1;
}

void registerActionBindings(HSQUIRRELVM vm) {
    // Base Action class (used as container)
    ClassDef(vm, "Action")
        .setTypeTag(typeTag<Action>())
        .commit();

    // Register global factory functions
    auto regFunc = [&](const char* name, SQFUNCTION fn) {
        sq_pushroottable(vm);
        sq_pushstring(vm, name, -1);
        sq_newclosure(vm, fn, 0);
        sq_newslot(vm, -3, SQFalse);
        sq_pop(vm, 1);
    };

    regFunc("MoveTo", sqMoveTo);
    regFunc("MoveBy", sqMoveBy);
    regFunc("ScaleTo", sqScaleTo);
    regFunc("RotateTo", sqRotateTo);
    regFunc("RotateBy", sqRotateBy);
    regFunc("FadeIn", sqFadeIn);
    regFunc("FadeOut", sqFadeOut);
    regFunc("FadeTo", sqFadeTo);
    regFunc("Delay", sqDelay);
    regFunc("Sequence", sqSequence);
    regFunc("Spawn", sqSpawn);
    regFunc("Loop", sqLoop);
    regFunc("CallFunc", sqCallFunc);
}

} // namespace sq
} // namespace easy2d
