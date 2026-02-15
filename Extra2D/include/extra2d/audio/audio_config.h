#pragma once

namespace extra2d {

/**
 * @file audio_config.h
 * @brief 音频模块配置
 * 
 * 定义音频相关的配置数据结构，由 AudioModule 管理。
 */

/**
 * @brief 音频配置数据结构
 */
struct AudioConfigData {
    bool enabled = true;
    int masterVolume = 100;     
    int musicVolume = 100;      
    int sfxVolume = 100;        
    int voiceVolume = 100;      
    int ambientVolume = 100;    
    int frequency = 44100;      
    int channels = 2;           
    int chunkSize = 2048;       
    int maxChannels = 16;       
    bool spatialAudio = false;  
    float listenerPosition[3] = {0.0f, 0.0f, 0.0f};

    /**
     * @brief 验证音量值是否有效
     * @param volume 要验证的音量值
     * @return 如果音量在0-100范围内返回 true
     */
    bool isValidVolume(int volume) const { return volume >= 0 && volume <= 100; }

    /**
     * @brief 将音量值转换为浮点数
     * @param volume 音量值（0-100）
     * @return 浮点数音量值（0.0-1.0）
     */
    float volumeToFloat(int volume) const { return static_cast<float>(volume) / 100.0f; }
};

} 
