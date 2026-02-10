#pragma once

#include <extra2d/extra2d.h>

namespace pushbox {

class SuccessScene : public extra2d::Scene {
public:
    SuccessScene();
    void onEnter() override;
    void onUpdate(float dt) override;

private:
    extra2d::Ptr<extra2d::Text> selectorText_;
};

} // namespace pushbox
