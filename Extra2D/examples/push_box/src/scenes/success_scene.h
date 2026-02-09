#pragma once

#include <extra2d/extra2d.h>

namespace pushbox {

class SuccessScene : public extra2d::Scene {
public:
  SuccessScene();
  void onEnter() override;
};

} // namespace pushbox
