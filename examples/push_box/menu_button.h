#pragma once

#include <extra2d/extra2d.h>

namespace pushbox {

class MenuButton : public extra2d::Button {
public:
    static extra2d::Ptr<MenuButton> create(extra2d::Ptr<extra2d::FontAtlas> font,
                                           const extra2d::String& text,
                                           extra2d::Function<void()> onClick);

    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }

private:
    bool enabled_ = true;
    extra2d::Function<void()> onClick_;
};

} // namespace pushbox
