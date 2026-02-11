#pragma once

#include <extra2d/core/types.h>
#include <extra2d/graphics/font.h>
#include <extra2d/ui/widget.h>

namespace extra2d {

// ============================================================================
// 单选按钮组件
// ============================================================================
class RadioButton : public Widget {
public:
    RadioButton();
    ~RadioButton() override = default;

    static Ptr<RadioButton> create();
    static Ptr<RadioButton> create(const std::string &label);

    // ------------------------------------------------------------------------
    // 链式调用构建器方法
    // ------------------------------------------------------------------------
    RadioButton *withPosition(float x, float y);
    RadioButton *withPosition(const Vec2 &pos);
    RadioButton *withAnchor(float x, float y);
    RadioButton *withAnchor(const Vec2 &anchor);
    RadioButton *withText(const std::string &text);
    RadioButton *withFont(Ptr<FontAtlas> font);
    RadioButton *withTextColor(const Color &color);
    RadioButton *withSize(float width, float height);
    RadioButton *withCoordinateSpace(CoordinateSpace space);
    RadioButton *withScreenPosition(float x, float y);
    RadioButton *withScreenPosition(const Vec2 &pos);
    RadioButton *withCameraOffset(float x, float y);
    RadioButton *withCameraOffset(const Vec2 &offset);

    void setSelected(bool selected);
    bool isSelected() const { return selected_; }

    void setLabel(const std::string &label);
    const std::string &getLabel() const { return label_; }

    void setFont(Ptr<FontAtlas> font);
    Ptr<FontAtlas> getFont() const { return font_; }

    void setTextColor(const Color &color);
    Color getTextColor() const { return textColor_; }

    void setCircleSize(float size);
    float getCircleSize() const { return circleSize_; }

    void setSpacing(float spacing);
    float getSpacing() const { return spacing_; }

    void setSelectedColor(const Color &color);
    Color getSelectedColor() const { return selectedColor_; }

    void setUnselectedColor(const Color &color);
    Color getUnselectedColor() const { return unselectedColor_; }

    void setDotColor(const Color &color);
    Color getDotColor() const { return dotColor_; }

    void setGroupId(int groupId);
    int getGroupId() const { return groupId_; }

    void setOnStateChange(Function<void(bool)> callback);

    Rect getBoundingBox() const override;

protected:
    void onDrawWidget(RenderBackend &renderer) override;
    bool onMousePress(const MouseEvent &event) override;
    bool onMouseRelease(const MouseEvent &event) override;

private:
    bool selected_ = false;
    std::string label_;
    Ptr<FontAtlas> font_;
    Color textColor_ = Colors::White;
    
    float circleSize_ = 20.0f;
    float spacing_ = 8.0f;
    
    Color selectedColor_ = Color(0.2f, 0.6f, 1.0f, 1.0f);
    Color unselectedColor_ = Color(0.3f, 0.3f, 0.3f, 1.0f);
    Color dotColor_ = Colors::White;
    
    int groupId_ = 0; // 用于分组，同组内只能选一个
    
    bool pressed_ = false;
    Function<void(bool)> onStateChange_;
};

// ============================================================================
// 单选按钮组管理器
// ============================================================================
class RadioButtonGroup {
public:
    void addButton(RadioButton *button);
    void removeButton(RadioButton *button);
    void selectButton(RadioButton *button);
    RadioButton *getSelectedButton() const { return selectedButton_; }
    
    void setOnSelectionChange(Function<void(RadioButton*)> callback);

private:
    std::vector<RadioButton*> buttons_;
    RadioButton *selectedButton_ = nullptr;
    Function<void(RadioButton*)> onSelectionChange_;
};

} // namespace extra2d
