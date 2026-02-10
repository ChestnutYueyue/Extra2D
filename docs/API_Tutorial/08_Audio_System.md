# Extra2D API 教程 - 08. 音频系统

## 音频系统概述

Extra2D 使用 SDL2_mixer 作为音频后端，支持 WAV、MP3、OGG 等格式。

## 音效播放

### 加载和播放音效

```cpp
// 获取资源管理器
auto &resources = Application::instance().resources();

// 加载音效
auto jumpSound = resources.loadSound("assets/jump.wav");
auto attackSound = resources::loadSound("assets/attack.ogg");

// 播放音效（一次）
jumpSound->play();

// 循环播放
backgroundMusic->play(true);

// 停止播放
jumpSound->stop();
```

## 音频控制器

### 创建音频控制器节点

```cpp
class AudioController : public Node {
public:
    static Ptr<AudioController> create() {
        return makePtr<AudioController>();
    }
    
    void onEnter() override {
        Node::onEnter();
        
        auto &resources = Application::instance().resources();
        
        // 加载音效
        sounds_["jump"] = resources.loadSound("assets/jump.wav");
        sounds_["attack"] = resources.loadSound("assets/attack.wav");
        sounds_["bgm"] = resources.loadSound("assets/bgm.mp3");
        
        // 播放背景音乐
        if (sounds_["bgm"]) {
            sounds_["bgm"]->play(true);
        }
    }
    
    void playSound(const std::string &name) {
        auto it = sounds_.find(name);
        if (it != sounds_.end() && it->second) {
            it->second->play();
        }
    }
    
    void setEnabled(bool enabled) {
        enabled_ = enabled;
        if (!enabled) {
            // 停止所有音效
            for (auto &[name, sound] : sounds_) {
                if (sound) {
                    sound->stop();
                }
            }
        }
    }
    
private:
    std::unordered_map<std::string, Ptr<Sound>> sounds_;
    bool enabled_ = true;
};
```

### 在场景中使用

```cpp
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 创建音频控制器
        auto audio = AudioController::create();
        audio->setName("audio_controller");
        addChild(audio);
        setAudioController(audio);
    }
    
    void playJumpSound() {
        if (auto audio = getAudioController()) {
            audio->playSound("jump");
        }
    }
    
    void playAttackSound() {
        if (auto audio = getAudioController()) {
            audio->playSound("attack");
        }
    }
    
    void toggleSound() {
        if (auto audio = getAudioController()) {
            audio->setEnabled(!audio->isEnabled());
        }
    }
    
private:
    Ptr<AudioController> audioController_;
    
    void setAudioController(Ptr<AudioController> controller) {
        audioController_ = controller;
    }
    
    AudioController* getAudioController() const {
        return audioController_.get();
    }
};
```

## 完整示例

```cpp
// AudioController.h
#pragma once
#include <extra2d/extra2d.h>
#include <unordered_map>

namespace game {

class AudioController : public extra2d::Node {
public:
    static extra2d::Ptr<AudioController> create();
    
    void onEnter() override;
    void playSound(const std::string &name);
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }
    
private:
    std::unordered_map<std::string, extra2d::Ptr<extra2d::Sound>> sounds_;
    bool enabled_ = true;
};

} // namespace game

// AudioController.cpp
#include "AudioController.h"

using namespace extra2d;

Ptr<AudioController> AudioController::create() {
    return makePtr<AudioController>();
}

void AudioController::onEnter() {
    Node::onEnter();
    
    auto &resources = Application::instance().resources();
    
    // 加载所有音效
    sounds_["jump"] = resources.loadSound("assets/audio/jump.wav");
    sounds_["attack"] = resources.loadSound("assets/audio/attack.wav");
    sounds_["hit"] = resources.loadSound("assets/audio/hit.wav");
    sounds_["bgm"] = resources.loadSound("assets/audio/background.mp3");
    
    // 播放背景音乐
    if (sounds_["bgm"]) {
        sounds_["bgm"]->play(true);
    }
}

void AudioController::playSound(const std::string &name) {
    if (!enabled_) return;
    
    auto it = sounds_.find(name);
    if (it != sounds_.end() && it->second) {
        it->second->play();
    }
}

void AudioController::setEnabled(bool enabled) {
    enabled_ = enabled;
    
    if (!enabled) {
        // 停止所有音效
        for (auto &[name, sound] : sounds_) {
            if (sound) {
                sound->stop();
            }
        }
    } else {
        // 重新播放背景音乐
        auto it = sounds_.find("bgm");
        if (it != sounds_.end() && it->second) {
            it->second->play(true);
        }
    }
}

// GameScene.cpp
#include "AudioController.h"

class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 创建音频控制器
        auto audio = game::AudioController::create();
        audio->setName("audio_controller");
        addChild(audio);
        audioController_ = audio;
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        auto &input = Application::instance().input();
        
        // A键跳跃
        if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_A)) {
            jump();
        }
        
        // B键攻击
        if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_B)) {
            attack();
        }
        
        // X键切换音效
        if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_X)) {
            toggleSound();
        }
    }
    
private:
    Ptr<game::AudioController> audioController_;
    
    void jump() {
        // 播放跳跃音效
        if (audioController_) {
            audioController_->playSound("jump");
        }
    }
    
    void attack() {
        // 播放攻击音效
        if (audioController_) {
            audioController_->playSound("attack");
        }
    }
    
    void toggleSound() {
        if (audioController_) {
            audioController_->setEnabled(!audioController_->isEnabled());
        }
    }
};
```

## 音频格式支持

| 格式 | 支持 |
|------|------|
| WAV | ✓ |
| MP3 | ✓ |
| OGG | ✓ |
| FLAC | ✓ |
| MOD | ✓ |

## 音量控制

```cpp
// 设置音效音量 (0-128)
Mix_Volume(-1, MIX_MAX_VOLUME / 2);  // 所有音效
Mix_Volume(channel, volume);          // 指定通道

// 设置音乐音量 (0-128)
Mix_VolumeMusic(volume);
```

## 最佳实践

1. **预加载音效**: 在 `onEnter()` 中加载所有需要的音效
2. **使用音频控制器**: 统一管理音效，方便控制开关
3. **音效开关**: 提供用户选项控制音效开关
4. **资源释放**: 音效资源会自动管理，无需手动释放

## 总结

Extra2D 的音频系统简单易用：

```cpp
// 加载
auto sound = resources.loadSound("assets/sound.wav");

// 播放
sound->play();      // 一次
sound->play(true);  // 循环

// 停止
sound->stop();
```

---

**教程完成！** 您已经学习了 Extra2D 的所有核心功能：

1. [快速开始](01_Quick_Start.md)
2. [场景系统](02_Scene_System.md)
3. [节点系统](03_Node_System.md)
4. [资源管理](04_Resource_Management.md)
5. [输入处理](05_Input_Handling.md)
6. [碰撞检测](06_Collision_Detection.md)
7. [UI 系统](07_UI_System.md)
8. [音频系统](08_Audio_System.md)
