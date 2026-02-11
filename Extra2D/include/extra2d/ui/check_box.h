#pragma once

#include <extra2d/core/types.h>
#include <extra2d/graphics/font.h>
#include <extra2d/ui/widget.h>

namespace extra2d {

// ============================================================================
// 复选框组件
// ============================================================================
class CheckBox : public Widget {
public:
    CheckBox();
    ~CheckBox() override = default;

    static Ptr<CheckBox> create();
    static Ptr<CheckBox> create(const std::string &label);

    // ------------------------------------------------------------------------
    // 链式调用构建器方法
    // ------------------------------------------------------------------------
    CheckBox *withPosition(float x, float y);
    CheckBox *withPosition(const Vec2 &pos);
    CheckBox *withAnchor(float x, float y);
    CheckBox *withAnchor(const Vec2 &anchor);
    CheckBox *withText(const std::string &text);
    CheckBox *withFont(Ptr<FontAtlas> font);
    CheckBox *withTextColor(const Color &color);
    CheckBox *withSize(float width, float height);

    // ------------------------------------------------------------------------
    // 链式调用 - 坐标空间设置
    // ------------------------------------------------------------------------
    CheckBox *withCoordinateSpace(CoordinateSpace space);
    CheckBox *withScreenPosition(float x, float y);
    CheckBox *withScreenPosition(const Vec2 &pos);
    CheckBox *withCameraOffset(float x, float y);
    CheckBox *withCameraOffset(const Vec2 &offset);

    void setChecked(bool checked);
    bool isChecked() const { return checked_; }
    void toggle();

    void setLabel(const std::string &label);
    const std::string &getLabel() const { return label_; }

    void setFont(Ptr<FontAtlas> font);
    Ptr<FontAtlas> getFont() const { return font_; }

    void setTextColor(const Color &color);
    Color getTextColor() const { return textColor_; }

    void setBoxSize(float size);
    float getBoxSize() const { return boxSize_; }

    void setSpacing(float spacing);
    float getSpacing() const { return spacing_; }

    void setCheckedColor(const Color &color);
    Color getCheckedColor() const { return checkedColor_; }

    void setUncheckedColor(const Color &color);
    Color getUncheckedColor() const { return uncheckedColor_; }

    void setCheckMarkColor(const Color &color);
    Color getCheckMarkColor() const { return checkMarkColor_; }

    void setOnStateChange(Function<void(bool)> callback);

    Rect getBoundingBox() const override;

protected:
    void onDrawWidget(RenderBackend &renderer) override;
    bool onMousePress(const MouseEvent &event) override;
    bool onMouseRelease(const MouseEvent &event) override;

private:
    bool checked_ = false;
    std::string label_;
    Ptr<FontAtlas> font_;
    Color textColor_ = Colors::White;
    
    float boxSize_ = 20.0f;
    float spacing_ = 8.0f;
    
    Color checkedColor_ = Color(0.2f, 0.6f, 1.0f, 1.0f);
    Color uncheckedColor_ = Color(0.3f, 0.3f, 0.3f, 1.0f);
    Color checkMarkColor_ = Colors::White;
    
    bool pressed_ = false;
    Function<void(bool)> onStateChange_;
};

} // namespace extra2d
