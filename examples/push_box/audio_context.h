#pragma once

#include <extra2d/extra2d.h>

namespace pushbox {

class AudioController;

void setAudioController(const extra2d::Ptr<AudioController>& controller);
extra2d::Ptr<AudioController> getAudioController();

} // namespace pushbox
