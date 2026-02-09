#include "menu_button.h"

#include <extra2d/core/color.h>
#include <extra2d/event/event.h>

namespace pushbox {

extra2d::Ptr<MenuButton>
MenuButton::create(extra2d::Ptr<extra2d::FontAtlas> font,
                   const extra2d::String &text,
                   extra2d::Function<void()> onClick) {
  auto btn = extra2d::makePtr<MenuButton>();
  btn->setFont(font);
  btn->setText(text);
  btn->setPadding(extra2d::Vec2(0.0f, 0.0f));
  btn->setBackgroundColor(extra2d::Colors::Transparent,
                          extra2d::Colors::Transparent,
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

  btn->getEventDispatcher().addListener(
      extra2d::EventType::UIHoverEnter,
      [wbtn = extra2d::WeakPtr<MenuButton>(btn)](extra2d::Event &) {
        if (auto self = wbtn.lock()) {
          if (self->enabled_) {
            self->setTextColor(extra2d::Colors::Blue);
          }
        }
      });

  btn->getEventDispatcher().addListener(
      extra2d::EventType::UIHoverExit,
      [wbtn = extra2d::WeakPtr<MenuButton>(btn)](extra2d::Event &) {
        if (auto self = wbtn.lock()) {
          if (self->enabled_) {
            self->setTextColor(extra2d::Colors::Black);
          }
        }
      });

  return btn;
}

void MenuButton::setEnabled(bool enabled) {
  enabled_ = enabled;
  setTextColor(enabled ? extra2d::Colors::Black : extra2d::Colors::LightGray);
}

} // namespace pushbox
