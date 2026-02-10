#include "menu_button.h"

#include <extra2d/extra2d.h>

namespace pushbox {

extra2d::Ptr<MenuButton> MenuButton::create(extra2d::Ptr<extra2d::FontAtlas> font,
                                            const extra2d::String& text,
                                            extra2d::Function<void()> onClick) {
    auto btn = extra2d::makePtr<MenuButton>();
    btn->setFont(font);
    btn->setText(text);
    btn->setPadding(extra2d::Vec2(0.0f, 0.0f));
    btn->setBackgroundColor(extra2d::Colors::Transparent, extra2d::Colors::Transparent,
                            extra2d::Colors::Transparent);
    btn->setBorder(extra2d::Colors::Transparent, 0.0f);
    btn->setTextColor(extra2d::Colors::Black);

    btn->onClick_ = std::move(onClick);
    btn->setOnClick([wbtn = extra2d::WeakPtr<MenuButton>(btn)]() {
        if (auto self = wbtn.lock()) {
            if (self->enabled_ && self->onClick_) {
                self->onClick_();
            }
        }
    });

    // 使用事件监听来处理悬停效果
    // Note: Extra2D 的 Button 类可能有不同的悬停检测机制
    // 这里简化处理，仅保留基本功能

    return btn;
}

void MenuButton::setEnabled(bool enabled) {
    enabled_ = enabled;
    setTextColor(enabled ? extra2d::Colors::Black : extra2d::Colors::LightGray);
}

} // namespace pushbox
