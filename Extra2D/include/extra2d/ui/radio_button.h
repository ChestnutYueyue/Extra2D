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
    // 选择状态
    // ------------------------------------------------------------------------
    void setSelected(bool selected);
    bool isSelected() const { return selected_; }

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
    // 圆形尺寸
    // ------------------------------------------------------------------------
    void setCircleSize(float size);
    float getCircleSize() const { return circleSize_; }

    // ------------------------------------------------------------------------
    // 间距
    // ------------------------------------------------------------------------
    void setSpacing(float spacing);
    float getSpacing() const { return spacing_; }

    // ------------------------------------------------------------------------
    // 颜色设置
    // ------------------------------------------------------------------------
    void setSelectedColor(const Color &color);
    Color getSelectedColor() const { return selectedColor_; }

    void setUnselectedColor(const Color &color);
    Color getUnselectedColor() const { return unselectedColor_; }

    void setDotColor(const Color &color);
    Color getDotColor() const { return dotColor_; }

    // ------------------------------------------------------------------------
    // 分组
    // ------------------------------------------------------------------------
    void setGroupId(int groupId);
    int getGroupId() const { return groupId_; }

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
    bool selected_ = false;
    std::string label_;
    Ptr<FontAtlas> font_;
    Color textColor_ = Colors::White;
    
    float circleSize_ = 20.0f;
    float spacing_ = 8.0f;
    
    Color selectedColor_ = Color(0.2f, 0.6f, 1.0f, 1.0f);
    Color unselectedColor_ = Color(0.3f, 0.3f, 0.3f, 1.0f);
    Color dotColor_ = Colors::White;
    
    int groupId_ = 0;
    
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
