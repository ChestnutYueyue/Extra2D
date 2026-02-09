#pragma once

#include <easy2d/core/types.h>
#include <squirrel.h>
#include <string>

namespace easy2d {
namespace sq {

// ============================================================================
// Type tag helpers — unique address per type
// ============================================================================
template<typename T>
inline SQUserPointer typeTag() {
    static int tag;
    return &tag;
}

// ============================================================================
// SqClassName trait — maps C++ types to Squirrel class names
// ============================================================================
template<typename T>
struct SqClassName {
    static const char* name();
};

// ============================================================================
// push / get primitives
// ============================================================================
inline void push(HSQUIRRELVM vm, int v) { sq_pushinteger(vm, static_cast<SQInteger>(v)); }
inline void push(HSQUIRRELVM vm, float v) { sq_pushfloat(vm, static_cast<SQFloat>(v)); }
inline void push(HSQUIRRELVM vm, double v) { sq_pushfloat(vm, static_cast<SQFloat>(v)); }
inline void push(HSQUIRRELVM vm, bool v) { sq_pushbool(vm, v ? SQTrue : SQFalse); }
inline void push(HSQUIRRELVM vm, const char* v) { sq_pushstring(vm, v, -1); }
inline void push(HSQUIRRELVM vm, const std::string& v) { sq_pushstring(vm, v.c_str(), static_cast<SQInteger>(v.size())); }
inline void pushNull(HSQUIRRELVM vm) { sq_pushnull(vm); }

inline SQInteger getInt(HSQUIRRELVM vm, SQInteger idx) {
    SQInteger val = 0;
    sq_getinteger(vm, idx, &val);
    return val;
}

inline SQFloat getFloat(HSQUIRRELVM vm, SQInteger idx) {
    SQFloat val = 0;
    if (SQ_FAILED(sq_getfloat(vm, idx, &val))) {
        SQInteger ival = 0;
        if (SQ_SUCCEEDED(sq_getinteger(vm, idx, &ival)))
            val = static_cast<SQFloat>(ival);
    }
    return val;
}

inline bool getBool(HSQUIRRELVM vm, SQInteger idx) {
    SQBool val = SQFalse;
    sq_getbool(vm, idx, &val);
    return val != SQFalse;
}

inline std::string getString(HSQUIRRELVM vm, SQInteger idx) {
    const SQChar* str = nullptr;
    sq_getstring(vm, idx, &str);
    return str ? std::string(str) : std::string();
}

// ============================================================================
// Value type userdata helpers
// ============================================================================
template<typename T>
T* pushValueInstance(HSQUIRRELVM vm, const T& val) {
    sq_pushroottable(vm);
    sq_pushstring(vm, SqClassName<T>::name(), -1);
    if (SQ_FAILED(sq_get(vm, -2))) {
        sq_pop(vm, 1);
        return nullptr;
    }
    if (SQ_FAILED(sq_createinstance(vm, -1))) {
        sq_pop(vm, 2);
        return nullptr;
    }

    T* ud = nullptr;
    sq_getinstanceup(vm, -1, reinterpret_cast<SQUserPointer*>(&ud), nullptr, SQFalse);
    if (ud) *ud = val;

    sq_remove(vm, -2); // class
    sq_remove(vm, -2); // roottable
    return ud;
}

template<typename T>
T* getValueInstance(HSQUIRRELVM vm, SQInteger idx) {
    T* ud = nullptr;
    sq_getinstanceup(vm, idx, reinterpret_cast<SQUserPointer*>(&ud), typeTag<T>(), SQFalse);
    return ud;
}

// ============================================================================
// Shared pointer bridge for reference types
// ============================================================================
template<typename T>
void pushPtr(HSQUIRRELVM vm, Ptr<T> ptr) {
    if (!ptr) { sq_pushnull(vm); return; }

    sq_pushroottable(vm);
    sq_pushstring(vm, SqClassName<T>::name(), -1);
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

    auto* storage = new Ptr<T>(std::move(ptr));
    sq_setinstanceup(vm, -1, storage);
    sq_setreleasehook(vm, -1, [](SQUserPointer p, SQInteger) -> SQInteger {
        delete static_cast<Ptr<T>*>(p);
        return 0;
    });

    sq_remove(vm, -2); // class
    sq_remove(vm, -2); // roottable
}

template<typename T>
Ptr<T> getPtr(HSQUIRRELVM vm, SQInteger idx) {
    SQUserPointer up = nullptr;
    sq_getinstanceup(vm, idx, &up, typeTag<T>(), SQFalse);
    if (!up) return nullptr;
    return *static_cast<Ptr<T>*>(up);
}

template<typename T>
T* getRawPtr(HSQUIRRELVM vm, SQInteger idx) {
    auto p = getPtr<T>(vm, idx);
    return p ? p.get() : nullptr;
}

// ============================================================================
// Singleton pointer (no release hook)
// ============================================================================
template<typename T>
void pushSingleton(HSQUIRRELVM vm, T* ptr, const char* className) {
    sq_pushroottable(vm);
    sq_pushstring(vm, className, -1);
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
    sq_setinstanceup(vm, -1, ptr);
    sq_remove(vm, -2); // class
    sq_remove(vm, -2); // roottable
}

template<typename T>
T* getSingleton(HSQUIRRELVM vm, SQInteger idx) {
    SQUserPointer up = nullptr;
    sq_getinstanceup(vm, idx, &up, nullptr, SQFalse);
    return static_cast<T*>(up);
}

// ============================================================================
// ClassDef — fluent API for registering a class
// ============================================================================
struct ClassDef {
    HSQUIRRELVM vm;

    ClassDef(HSQUIRRELVM v, const char* name, const char* base = nullptr)
        : vm(v) {
        sq_pushroottable(vm);
        sq_pushstring(vm, name, -1);

        if (base) {
            sq_pushstring(vm, base, -1);
            if (SQ_FAILED(sq_get(vm, -3))) {
                sq_newclass(vm, SQFalse);
            } else {
                sq_newclass(vm, SQTrue);
            }
        } else {
            sq_newclass(vm, SQFalse);
        }
    }

    ClassDef& setTypeTag(SQUserPointer tag) {
        sq_settypetag(vm, -1, tag);
        return *this;
    }

    ClassDef& method(const char* name, SQFUNCTION fn, SQInteger nparams = 0, const char* typemask = nullptr) {
        sq_pushstring(vm, name, -1);
        sq_newclosure(vm, fn, 0);
        if (nparams > 0 && typemask)
            sq_setparamscheck(vm, nparams, typemask);
        sq_newslot(vm, -3, SQFalse);
        return *this;
    }

    ClassDef& staticMethod(const char* name, SQFUNCTION fn, SQInteger nparams = 0, const char* typemask = nullptr) {
        sq_pushstring(vm, name, -1);
        sq_newclosure(vm, fn, 0);
        if (nparams > 0 && typemask)
            sq_setparamscheck(vm, nparams, typemask);
        sq_newslot(vm, -3, SQTrue);
        return *this;
    }

    template<typename T>
    ClassDef& setValueType(SQFUNCTION constructor) {
        sq_settypetag(vm, -1, typeTag<T>());
        sq_setclassudsize(vm, -1, sizeof(T));
        sq_pushstring(vm, "constructor", -1);
        sq_newclosure(vm, constructor, 0);
        sq_newslot(vm, -3, SQFalse);
        return *this;
    }

    void commit() {
        sq_newslot(vm, -3, SQFalse);
        sq_pop(vm, 1);
    }
};

// ============================================================================
// Register a table of integer constants
// ============================================================================
inline void registerConstTable(HSQUIRRELVM vm, const char* tableName,
                               const char* const* names, const SQInteger* values, int count) {
    sq_pushroottable(vm);
    sq_pushstring(vm, tableName, -1);
    sq_newtable(vm);
    for (int i = 0; i < count; ++i) {
        sq_pushstring(vm, names[i], -1);
        sq_pushinteger(vm, values[i]);
        sq_newslot(vm, -3, SQFalse);
    }
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 1);
}

} // namespace sq
} // namespace easy2d
