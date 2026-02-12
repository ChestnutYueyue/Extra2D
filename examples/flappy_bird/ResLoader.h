// ============================================================================
// ResLoader.h - 资源加载器
// 描述: 管理游戏资源的加载和访问
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>
#include <map>
#include <string>

namespace flappybird {

/**
 * @brief 音频类型枚举
 */
enum class MusicType {
    Click,  // 按键声音
    Hit,    // 小鸟死亡声音
    Fly,    // 小鸟飞翔声音
    Point,  // 得分声音
    Swoosh  // 转场声音
};

/**
 * @brief 资源加载器类
 * 管理纹理图集、精灵帧和音频资源的加载
 */
class ResLoader {
public:
    /**
     * @brief 初始化资源加载器
     */
    static void init();

    /**
     * @brief 获取精灵帧
     * @param name 帧名称
     * @return 精灵帧指针
     */
    static extra2d::Ptr<extra2d::SpriteFrame> getKeyFrame(const std::string& name);

    /**
     * @brief 播放音效
     * @param type 音效类型
     */
    static void playMusic(MusicType type);

private:
    /**
     * @brief 图片信息结构
     * 对应 atlas.txt 格式: 元素名 width height x y
     */
    struct ImageInfo {
        float width, height, x, y;
    };

    static extra2d::Ptr<extra2d::Texture> atlasTexture_;                    // 图集纹理
    static std::map<std::string, ImageInfo> imageMap_;                      // 图片信息映射
    static std::map<MusicType, extra2d::Ptr<extra2d::Sound>> soundMap_;     // 音效映射
};

} // namespace flappybird
