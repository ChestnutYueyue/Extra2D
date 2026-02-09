#include <easy2d/script/sq_binding_types.h>

namespace easy2d {
namespace sq {

// ============================================================================
// Vec2
// ============================================================================
static SQInteger vec2Constructor(HSQUIRRELVM vm) {
    Vec2* self = nullptr;
    sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&self), nullptr, SQFalse);
    SQInteger argc = sq_gettop(vm);
    if (argc >= 3) {
        self->x = static_cast<float>(getFloat(vm, 2));
        self->y = static_cast<float>(getFloat(vm, 3));
    } else {
        self->x = 0.0f;
        self->y = 0.0f;
    }
    return 0;
}

static SQInteger vec2GetX(HSQUIRRELVM vm) {
    Vec2* v = getValueInstance<Vec2>(vm, 1);
    sq_pushfloat(vm, v->x);
    return 1;
}

static SQInteger vec2SetX(HSQUIRRELVM vm) {
    Vec2* v = getValueInstance<Vec2>(vm, 1);
    v->x = static_cast<float>(getFloat(vm, 2));
    return 0;
}

static SQInteger vec2GetY(HSQUIRRELVM vm) {
    Vec2* v = getValueInstance<Vec2>(vm, 1);
    sq_pushfloat(vm, v->y);
    return 1;
}

static SQInteger vec2SetY(HSQUIRRELVM vm) {
    Vec2* v = getValueInstance<Vec2>(vm, 1);
    v->y = static_cast<float>(getFloat(vm, 2));
    return 0;
}

static SQInteger vec2Length(HSQUIRRELVM vm) {
    Vec2* v = getValueInstance<Vec2>(vm, 1);
    sq_pushfloat(vm, v->length());
    return 1;
}

static SQInteger vec2Normalized(HSQUIRRELVM vm) {
    Vec2* v = getValueInstance<Vec2>(vm, 1);
    pushValueInstance(vm, v->normalized());
    return 1;
}

static SQInteger vec2Dot(HSQUIRRELVM vm) {
    Vec2* a = getValueInstance<Vec2>(vm, 1);
    Vec2* b = getValueInstance<Vec2>(vm, 2);
    sq_pushfloat(vm, a->dot(*b));
    return 1;
}

static SQInteger vec2Distance(HSQUIRRELVM vm) {
    Vec2* a = getValueInstance<Vec2>(vm, 1);
    Vec2* b = getValueInstance<Vec2>(vm, 2);
    sq_pushfloat(vm, a->distance(*b));
    return 1;
}

static SQInteger vec2Add(HSQUIRRELVM vm) {
    Vec2* a = getValueInstance<Vec2>(vm, 1);
    Vec2* b = getValueInstance<Vec2>(vm, 2);
    pushValueInstance(vm, *a + *b);
    return 1;
}

static SQInteger vec2Sub(HSQUIRRELVM vm) {
    Vec2* a = getValueInstance<Vec2>(vm, 1);
    Vec2* b = getValueInstance<Vec2>(vm, 2);
    pushValueInstance(vm, *a - *b);
    return 1;
}

static SQInteger vec2Mul(HSQUIRRELVM vm) {
    Vec2* a = getValueInstance<Vec2>(vm, 1);
    SQFloat s = getFloat(vm, 2);
    pushValueInstance(vm, *a * static_cast<float>(s));
    return 1;
}

static SQInteger vec2Div(HSQUIRRELVM vm) {
    Vec2* a = getValueInstance<Vec2>(vm, 1);
    SQFloat s = getFloat(vm, 2);
    if (s == 0.0f) return sq_throwerror(vm, "division by zero");
    pushValueInstance(vm, *a / static_cast<float>(s));
    return 1;
}

static SQInteger vec2Neg(HSQUIRRELVM vm) {
    Vec2* a = getValueInstance<Vec2>(vm, 1);
    pushValueInstance(vm, -*a);
    return 1;
}

static SQInteger vec2ToString(HSQUIRRELVM vm) {
    Vec2* v = getValueInstance<Vec2>(vm, 1);
    char buf[64];
    snprintf(buf, sizeof(buf), "Vec2(%.3f, %.3f)", v->x, v->y);
    sq_pushstring(vm, buf, -1);
    return 1;
}

void registerVec2(HSQUIRRELVM vm) {
    ClassDef(vm, "Vec2")
        .setValueType<Vec2>(vec2Constructor)
        .method("getX", vec2GetX)
        .method("setX", vec2SetX)
        .method("getY", vec2GetY)
        .method("setY", vec2SetY)
        .method("length", vec2Length)
        .method("normalized", vec2Normalized)
        .method("dot", vec2Dot, 2, "xx")
        .method("distance", vec2Distance, 2, "xx")
        .method("_add", vec2Add, 2, "xx")
        .method("_sub", vec2Sub, 2, "xx")
        .method("_mul", vec2Mul)
        .method("_div", vec2Div)
        .method("_unm", vec2Neg)
        .method("_tostring", vec2ToString)
        .commit();
}

// ============================================================================
// Size
// ============================================================================
static SQInteger sizeConstructor(HSQUIRRELVM vm) {
    Size* self = nullptr;
    sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&self), nullptr, SQFalse);
    SQInteger argc = sq_gettop(vm);
    if (argc >= 3) {
        self->width = static_cast<float>(getFloat(vm, 2));
        self->height = static_cast<float>(getFloat(vm, 3));
    } else {
        self->width = 0.0f;
        self->height = 0.0f;
    }
    return 0;
}

static SQInteger sizeGetWidth(HSQUIRRELVM vm) {
    Size* s = getValueInstance<Size>(vm, 1);
    sq_pushfloat(vm, s->width);
    return 1;
}

static SQInteger sizeSetWidth(HSQUIRRELVM vm) {
    Size* s = getValueInstance<Size>(vm, 1);
    s->width = static_cast<float>(getFloat(vm, 2));
    return 0;
}

static SQInteger sizeGetHeight(HSQUIRRELVM vm) {
    Size* s = getValueInstance<Size>(vm, 1);
    sq_pushfloat(vm, s->height);
    return 1;
}

static SQInteger sizeSetHeight(HSQUIRRELVM vm) {
    Size* s = getValueInstance<Size>(vm, 1);
    s->height = static_cast<float>(getFloat(vm, 2));
    return 0;
}

static SQInteger sizeArea(HSQUIRRELVM vm) {
    Size* s = getValueInstance<Size>(vm, 1);
    sq_pushfloat(vm, s->area());
    return 1;
}

static SQInteger sizeToString(HSQUIRRELVM vm) {
    Size* s = getValueInstance<Size>(vm, 1);
    char buf[64];
    snprintf(buf, sizeof(buf), "Size(%.3f, %.3f)", s->width, s->height);
    sq_pushstring(vm, buf, -1);
    return 1;
}

void registerSize(HSQUIRRELVM vm) {
    ClassDef(vm, "Size")
        .setValueType<Size>(sizeConstructor)
        .method("getWidth", sizeGetWidth)
        .method("setWidth", sizeSetWidth)
        .method("getHeight", sizeGetHeight)
        .method("setHeight", sizeSetHeight)
        .method("area", sizeArea)
        .method("_tostring", sizeToString)
        .commit();
}

// ============================================================================
// Rect
// ============================================================================
static SQInteger rectConstructor(HSQUIRRELVM vm) {
    Rect* self = nullptr;
    sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&self), nullptr, SQFalse);
    SQInteger argc = sq_gettop(vm);
    if (argc >= 5) {
        self->origin.x = static_cast<float>(getFloat(vm, 2));
        self->origin.y = static_cast<float>(getFloat(vm, 3));
        self->size.width = static_cast<float>(getFloat(vm, 4));
        self->size.height = static_cast<float>(getFloat(vm, 5));
    } else {
        *self = Rect();
    }
    return 0;
}

static SQInteger rectGetX(HSQUIRRELVM vm) {
    Rect* r = getValueInstance<Rect>(vm, 1);
    sq_pushfloat(vm, r->origin.x);
    return 1;
}

static SQInteger rectGetY(HSQUIRRELVM vm) {
    Rect* r = getValueInstance<Rect>(vm, 1);
    sq_pushfloat(vm, r->origin.y);
    return 1;
}

static SQInteger rectGetWidth(HSQUIRRELVM vm) {
    Rect* r = getValueInstance<Rect>(vm, 1);
    sq_pushfloat(vm, r->size.width);
    return 1;
}

static SQInteger rectGetHeight(HSQUIRRELVM vm) {
    Rect* r = getValueInstance<Rect>(vm, 1);
    sq_pushfloat(vm, r->size.height);
    return 1;
}

static SQInteger rectContainsPoint(HSQUIRRELVM vm) {
    Rect* r = getValueInstance<Rect>(vm, 1);
    Vec2* p = getValueInstance<Vec2>(vm, 2);
    sq_pushbool(vm, r->containsPoint(*p) ? SQTrue : SQFalse);
    return 1;
}

static SQInteger rectIntersects(HSQUIRRELVM vm) {
    Rect* a = getValueInstance<Rect>(vm, 1);
    Rect* b = getValueInstance<Rect>(vm, 2);
    sq_pushbool(vm, a->intersects(*b) ? SQTrue : SQFalse);
    return 1;
}

static SQInteger rectToString(HSQUIRRELVM vm) {
    Rect* r = getValueInstance<Rect>(vm, 1);
    char buf[96];
    snprintf(buf, sizeof(buf), "Rect(%.1f, %.1f, %.1f, %.1f)",
             r->origin.x, r->origin.y, r->size.width, r->size.height);
    sq_pushstring(vm, buf, -1);
    return 1;
}

void registerRect(HSQUIRRELVM vm) {
    ClassDef(vm, "Rect")
        .setValueType<Rect>(rectConstructor)
        .method("getX", rectGetX)
        .method("getY", rectGetY)
        .method("getWidth", rectGetWidth)
        .method("getHeight", rectGetHeight)
        .method("containsPoint", rectContainsPoint, 2, "xx")
        .method("intersects", rectIntersects, 2, "xx")
        .method("_tostring", rectToString)
        .commit();
}

// ============================================================================
// Color
// ============================================================================
static SQInteger colorConstructor(HSQUIRRELVM vm) {
    Color* self = nullptr;
    sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&self), nullptr, SQFalse);
    SQInteger argc = sq_gettop(vm);
    if (argc >= 5) {
        self->r = static_cast<float>(getFloat(vm, 2));
        self->g = static_cast<float>(getFloat(vm, 3));
        self->b = static_cast<float>(getFloat(vm, 4));
        self->a = static_cast<float>(getFloat(vm, 5));
    } else if (argc >= 4) {
        self->r = static_cast<float>(getFloat(vm, 2));
        self->g = static_cast<float>(getFloat(vm, 3));
        self->b = static_cast<float>(getFloat(vm, 4));
        self->a = 1.0f;
    } else {
        *self = Color(1.0f, 1.0f, 1.0f, 1.0f);
    }
    return 0;
}

static SQInteger colorGetR(HSQUIRRELVM vm) { Color* c = getValueInstance<Color>(vm, 1); sq_pushfloat(vm, c->r); return 1; }
static SQInteger colorGetG(HSQUIRRELVM vm) { Color* c = getValueInstance<Color>(vm, 1); sq_pushfloat(vm, c->g); return 1; }
static SQInteger colorGetB(HSQUIRRELVM vm) { Color* c = getValueInstance<Color>(vm, 1); sq_pushfloat(vm, c->b); return 1; }
static SQInteger colorGetA(HSQUIRRELVM vm) { Color* c = getValueInstance<Color>(vm, 1); sq_pushfloat(vm, c->a); return 1; }

static SQInteger colorSetR(HSQUIRRELVM vm) { Color* c = getValueInstance<Color>(vm, 1); c->r = static_cast<float>(getFloat(vm, 2)); return 0; }
static SQInteger colorSetG(HSQUIRRELVM vm) { Color* c = getValueInstance<Color>(vm, 1); c->g = static_cast<float>(getFloat(vm, 2)); return 0; }
static SQInteger colorSetB(HSQUIRRELVM vm) { Color* c = getValueInstance<Color>(vm, 1); c->b = static_cast<float>(getFloat(vm, 2)); return 0; }
static SQInteger colorSetA(HSQUIRRELVM vm) { Color* c = getValueInstance<Color>(vm, 1); c->a = static_cast<float>(getFloat(vm, 2)); return 0; }

static SQInteger colorFromRGBA(HSQUIRRELVM vm) {
    auto r = static_cast<uint8_t>(getInt(vm, 2));
    auto g = static_cast<uint8_t>(getInt(vm, 3));
    auto b = static_cast<uint8_t>(getInt(vm, 4));
    uint8_t a = 255;
    if (sq_gettop(vm) >= 5)
        a = static_cast<uint8_t>(getInt(vm, 5));
    pushValueInstance(vm, Color::fromRGBA(r, g, b, a));
    return 1;
}

static SQInteger colorToString(HSQUIRRELVM vm) {
    Color* c = getValueInstance<Color>(vm, 1);
    char buf[80];
    snprintf(buf, sizeof(buf), "Color(%.3f, %.3f, %.3f, %.3f)", c->r, c->g, c->b, c->a);
    sq_pushstring(vm, buf, -1);
    return 1;
}

void registerColor(HSQUIRRELVM vm) {
    ClassDef(vm, "Color")
        .setValueType<Color>(colorConstructor)
        .method("getR", colorGetR)
        .method("setR", colorSetR)
        .method("getG", colorGetG)
        .method("setG", colorSetG)
        .method("getB", colorGetB)
        .method("setB", colorSetB)
        .method("getA", colorGetA)
        .method("setA", colorSetA)
        .staticMethod("fromRGBA", colorFromRGBA)
        .method("_tostring", colorToString)
        .commit();
}

// ============================================================================
// Register all value types
// ============================================================================
void registerValueTypes(HSQUIRRELVM vm) {
    registerVec2(vm);
    registerSize(vm);
    registerRect(vm);
    registerColor(vm);
}

} // namespace sq
} // namespace easy2d
