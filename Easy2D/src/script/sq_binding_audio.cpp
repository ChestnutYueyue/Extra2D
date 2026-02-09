#include <easy2d/script/sq_binding_audio.h>
#include <easy2d/script/sq_binding_types.h>
#include <easy2d/audio/audio_engine.h>
#include <easy2d/audio/sound.h>

namespace easy2d {
namespace sq {

// ============================================================================
// AudioEngine singleton
// ============================================================================
static SQInteger audioLoadSound(HSQUIRRELVM vm) {
    SQInteger argc = sq_gettop(vm);
    std::shared_ptr<Sound> snd;
    if (argc >= 3) {
        std::string name = getString(vm, 2);
        std::string path = getString(vm, 3);
        snd = AudioEngine::getInstance().loadSound(name, path);
    } else {
        std::string path = getString(vm, 2);
        snd = AudioEngine::getInstance().loadSound(path);
    }
    push(vm, snd != nullptr);
    return 1;
}

static SQInteger audioPlaySound(HSQUIRRELVM vm) {
    std::string name = getString(vm, 2);
    auto snd = AudioEngine::getInstance().getSound(name);
    if (snd) snd->play();
    return 0;
}

static SQInteger audioStopSound(HSQUIRRELVM vm) {
    std::string name = getString(vm, 2);
    auto snd = AudioEngine::getInstance().getSound(name);
    if (snd) snd->stop();
    return 0;
}

static SQInteger audioSetMasterVolume(HSQUIRRELVM vm) {
    float vol = static_cast<float>(getFloat(vm, 2));
    AudioEngine::getInstance().setMasterVolume(vol);
    return 0;
}

static SQInteger audioGetMasterVolume(HSQUIRRELVM vm) {
    push(vm, AudioEngine::getInstance().getMasterVolume());
    return 1;
}

static SQInteger audioStopAll(HSQUIRRELVM vm) {
    AudioEngine::getInstance().stopAll();
    return 0;
}

static SQInteger audioPauseAll(HSQUIRRELVM vm) {
    AudioEngine::getInstance().pauseAll();
    return 0;
}

static SQInteger audioResumeAll(HSQUIRRELVM vm) {
    AudioEngine::getInstance().resumeAll();
    return 0;
}

void registerAudioBindings(HSQUIRRELVM vm) {
    ClassDef(vm, "AudioEngineClass")
        .method("loadSound", audioLoadSound)
        .method("playSound", audioPlaySound)
        .method("stopSound", audioStopSound)
        .method("setMasterVolume", audioSetMasterVolume)
        .method("getMasterVolume", audioGetMasterVolume)
        .method("stopAll", audioStopAll)
        .method("pauseAll", audioPauseAll)
        .method("resumeAll", audioResumeAll)
        .commit();

    // Global "Audio" instance
    pushSingleton(vm, &AudioEngine::getInstance(), "AudioEngineClass");
    sq_pushroottable(vm);
    sq_pushstring(vm, "Audio", -1);
    sq_push(vm, -3);
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 2);
}

} // namespace sq
} // namespace easy2d
