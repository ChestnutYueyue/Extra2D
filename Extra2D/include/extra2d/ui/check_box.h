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
    // 选中状态
    // ------------------------------------------------------------------------
    void setChecked(bool checked);
    bool isChecked() const { return checked_; }
    void toggle();

    // ------------------------------------------------------------------------
    // 标签设置
    // ------------------------------------------------------------------------
    void setLabel(const std::string &label);
    const std::string &getLabel() const { return label_; }

    // ------------------------------------------------------------------------
    // 字体设置
    // ------------------------------------------------------------------------
    void setFont(Ptr<FontAtlas> font);
    Ptr<FontAtlas> getFont() const { return font_; }

    // ------------------------------------------------------------------------
    // 文字颜色
    // ------------------------------------------------------------------------
    void setTextColor(const Color &color);
    Color getTextColor() const { return textColor_; }

    // ------------------------------------------------------------------------
    // 复选框尺寸
    // ------------------------------------------------------------------------
    void setBoxSize(float size);
    float getBoxSize() const { return boxSize_; }

    // ------------------------------------------------------------------------
    // 间距
    // ------------------------------------------------------------------------
    void setSpacing(float spacing);
    float getSpacing() const { return spacing_; }

    // ------------------------------------------------------------------------
    // 颜色设置
    // ------------------------------------------------------------------------
    void setCheckedColor(const Color &color);
    Color getCheckedColor() const { return checkedColor_; }

    void setUncheckedColor(const Color &color);
    Color getUncheckedColor() const { return uncheckedColor_; }

    void setCheckMarkColor(const Color &color);
    Color getCheckMarkColor() const { return checkMarkColor_; }

    // ------------------------------------------------------------------------
    // 回调设置
    // ------------------------------------------------------------------------
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
