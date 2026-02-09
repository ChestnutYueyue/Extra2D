#pragma once

#include "../core/data.h"
#include <extra2d/extra2d.h>

namespace pushbox {

class AudioController : public extra2d::Node {
public:
  static extra2d::Ptr<AudioController> create();

  void onEnter() override;

  void setEnabled(bool enabled);
  bool isEnabled() const { return enabled_; }

  void playManMove();
  void playBoxMove();

private:
  bool loaded_ = false;
  bool enabled_ = true;

  extra2d::Ptr<extra2d::Sound> background_;
  extra2d::Ptr<extra2d::Sound> manMove_;
  extra2d::Ptr<extra2d::Sound> boxMove_;
};

} // namespace pushbox
