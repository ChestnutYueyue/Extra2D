#include "audio_context.h"

namespace pushbox {

static extra2d::WeakPtr<AudioController> g_audioController;

void setAudioController(const extra2d::Ptr<AudioController> &controller) {
  g_audioController = controller;
}

extra2d::Ptr<AudioController> getAudioController() {
  return g_audioController.lock();
}

} // namespace pushbox
