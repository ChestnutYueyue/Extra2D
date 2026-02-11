# 08. 音频系统

Extra2D 提供了基于 SDL2_mixer 的音频播放系统，支持音效播放。

## 音频引擎

通过 `Application::instance().audio()` 访问音频引擎：

```cpp
auto& audio = Application::instance().audio();
```

## 播放音效

### 基本用法

```cpp
// 加载音效
auto sound = audio.loadSound("assets/audio/jump.wav");

// 播放音效
sound->play();

// 设置音量 (0.0 - 1.0)
sound->setVolume(0.8f);
```

### 音效控制

```cpp
// 停止播放
sound->stop();

// 暂停
sound->pause();

// 恢复
sound->resume();

// 循环播放
sound->setLooping(true);

// 检查播放状态
bool playing = sound->isPlaying();
bool paused = sound->isPaused();
```

### 音调和播放位置

```cpp
// 设置音调（当前实现不支持）
sound->setPitch(1.0f);

// 获取/设置播放位置（当前实现不支持）
float cursor = sound->getCursor();
sound->setCursor(0.0f);

// 获取音频时长（当前实现不支持）
float duration = sound->getDuration();
```

## 全局音量控制

```cpp
// 设置主音量
audio.setMasterVolume(0.8f);

// 获取主音量
float volume = audio.getMasterVolume();
```

## 全局播放控制

```cpp
// 暂停所有音效
audio.pauseAll();

// 恢复所有音效
audio.resumeAll();

// 停止所有音效
audio.stopAll();

// 卸载指定音效
audio.unloadSound("jump");

// 卸载所有音效
audio.unloadAllSounds();
```

## 完整示例

```cpp
// audio_manager.h
#pragma once

#include <extra2d/extra2d.h>

namespace pushbox {

class AudioManager {
public:
    static AudioManager& instance() {
        static AudioManager instance;
        return instance;
    }
    
    void init();
    void setEnabled(bool enabled);
    void playMoveSound();
    void playBoxMoveSound();
    void playWinSound();
    
private:
    AudioManager() = default;
    
    bool enabled_ = true;
    extra2d::Ptr<extra2d::Sound> moveSound_;
    extra2d::Ptr<extra2d::Sound> boxMoveSound_;
    extra2d::Ptr<extra2d::Sound> winSound_;
};

} // namespace pushbox
```

```cpp
// audio_manager.cpp
#include "audio_manager.h"

namespace pushbox {

void AudioManager::init() {
    auto& audio = extra2d::Application::instance().audio();
    
    // 加载音效
    moveSound_ = audio.loadSound("move", "assets/audio/manmove.wav");
    boxMoveSound_ = audio.loadSound("boxmove", "assets/audio/boxmove.wav");
    winSound_ = audio.loadSound("win", "assets/audio/win.wav");
}

void AudioManager::setEnabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled) {
        extra2d::Application::instance().audio().stopAll();
    }
}

void AudioManager::playMoveSound() {
    if (enabled_ && moveSound_) {
        moveSound_->play();
    }
}

void AudioManager::playBoxMoveSound() {
    if (enabled_ && boxMoveSound_) {
        boxMoveSound_->play();
    }
}

void AudioManager::playWinSound() {
    if (enabled_ && winSound_) {
        winSound_->play();
    }
}

} // namespace pushbox
```

## 使用音频管理器

```cpp
// main.cpp
int main(int argc, char** argv) {
    // ... 初始化应用 ...
    
    // 初始化音频管理器
    pushbox::AudioManager::instance().init();
    
    // ... 运行应用 ...
}

// PlayScene.cpp
void PlayScene::move(int dx, int dy, int direct) {
    // ... 移动逻辑 ...
    
    if (isBoxMoved) {
        // 播放推箱子音效
        pushbox::AudioManager::instance().playBoxMoveSound();
    } else {
        // 播放移动音效
        pushbox::AudioManager::instance().playMoveSound();
    }
}
```

## 音频开关控制

```cpp
// 在菜单中切换音效
void StartScene::onUpdate(float dt) {
    auto& input = Application::instance().input();
    
    // X键切换音效
    if (input.isButtonPressed(GamepadButton::X)) {
        g_SoundOpen = !g_SoundOpen;
        AudioManager::instance().setEnabled(g_SoundOpen);
        updateSoundIcon();
    }
}
```

## 支持的音频格式

- WAV
- OGG
- MP3（需要 SDL2_mixer 支持）

## 最佳实践

1. **使用单例管理器** - 集中管理音频资源
2. **预加载常用音效** - 在初始化时加载
3. **提供开关选项** - 让用户控制音效
4. **合理设置音量** - 避免音量过大
5. **及时卸载不用的音效** - 释放内存资源

## API 参考

### Sound 类

| 方法 | 说明 |
|------|------|
| `play()` | 播放音效 |
| `pause()` | 暂停播放 |
| `resume()` | 恢复播放 |
| `stop()` | 停止播放 |
| `isPlaying()` | 是否正在播放 |
| `isPaused()` | 是否已暂停 |
| `setVolume(float)` | 设置音量 (0.0-1.0) |
| `getVolume()` | 获取音量 |
| `setLooping(bool)` | 设置循环播放 |
| `isLooping()` | 是否循环播放 |
| `setPitch(float)` | 设置音调（当前不支持） |
| `getPitch()` | 获取音调 |
| `getDuration()` | 获取时长（当前不支持） |
| `getCursor()` | 获取播放位置（当前不支持） |
| `setCursor(float)` | 设置播放位置（当前不支持） |

### AudioEngine 类

| 方法 | 说明 |
|------|------|
| `getInstance()` | 获取单例实例 |
| `initialize()` | 初始化音频引擎 |
| `shutdown()` | 关闭音频引擎 |
| `loadSound(path)` | 加载音效（以路径为名称） |
| `loadSound(name, path)` | 加载音效（指定名称） |
| `getSound(name)` | 获取已加载的音效 |
| `unloadSound(name)` | 卸载指定音效 |
| `unloadAllSounds()` | 卸载所有音效 |
| `setMasterVolume(float)` | 设置主音量 |
| `getMasterVolume()` | 获取主音量 |
| `pauseAll()` | 暂停所有音效 |
| `resumeAll()` | 恢复所有音效 |
| `stopAll()` | 停止所有音效 |

## 总结

至此，你已经学习了 Extra2D 引擎的核心功能：

1. [快速开始](./01_Quick_Start.md) - 引擎基础
2. [场景系统](./02_Scene_System.md) - 场景管理
3. [节点系统](./03_Node_System.md) - 游戏对象
4. [资源管理](./04_Resource_Management.md) - 资源加载
5. [输入处理](./05_Input_Handling.md) - 输入控制
6. [碰撞检测](./06_Collision_Detection.md) - 空间索引
7. [UI 系统](./07_UI_System.md) - 界面控件
8. [音频系统](./08_Audio_System.md) - 音频播放

开始你的游戏开发之旅吧！
