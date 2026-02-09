#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/color.h>
#include <easy2d/effects/particle_system.h>
#include <easy2d/effects/post_process.h>
#include <easy2d/graphics/shader_system.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

namespace easy2d {

// ============================================================================
// 自定义特效类型
// ============================================================================
enum class CustomEffectType {
    Particle,       // 粒子特效
    PostProcess,    // 后处理特效
    Shader,         // Shader特效
    Combined        // 组合特效
};

// ============================================================================
// 自定义特效配置
// ============================================================================
struct CustomEffectConfig {
    std::string name;                           // 特效名称
    CustomEffectType type;                      // 特效类型
    std::string description;                    // 描述
    
    // 粒子特效配置
    EmitterConfig emitterConfig;
    
    // 后处理特效配置
    std::string shaderVertPath;                 // 顶点着色器路径
    std::string shaderFragPath;                 // 片段着色器路径
    std::unordered_map<std::string, float> shaderParams;  // Shader参数
    
    // 通用配置
    float duration;                             // 持续时间(-1表示无限)
    bool loop;                                  // 是否循环
    float delay;                                // 延迟启动时间
};

// ============================================================================
// 自定义特效基类
// ============================================================================
class CustomEffect {
public:
    explicit CustomEffect(const CustomEffectConfig& config);
    virtual ~CustomEffect() = default;

    // ------------------------------------------------------------------------
    // 生命周期
    // ------------------------------------------------------------------------
    virtual bool init();
    virtual void update(float dt);
    virtual void render(RenderBackend& renderer);
    virtual void shutdown();

    // ------------------------------------------------------------------------
    // 控制
    // ------------------------------------------------------------------------
    void play();
    void pause();
    void stop();
    void reset();
    
    bool isPlaying() const { return playing_; }
    bool isFinished() const { return finished_; }
    float getElapsedTime() const { return elapsedTime_; }

    // ------------------------------------------------------------------------
    // 配置
    // ------------------------------------------------------------------------
    const std::string& getName() const { return config_.name; }
    const CustomEffectConfig& getConfig() const { return config_; }
    
    void setPosition(const Vec2& pos) { position_ = pos; }
    void setRotation(float rot) { rotation_ = rot; }
    void setScale(float scale) { scale_ = scale; }
    
    Vec2 getPosition() const { return position_; }
    float getRotation() const { return rotation_; }
    float getScale() const { return scale_; }

protected:
    CustomEffectConfig config_;
    Vec2 position_ = Vec2::Zero();
    float rotation_ = 0.0f;
    float scale_ = 1.0f;
    
    bool playing_ = false;
    bool paused_ = false;
    bool finished_ = false;
    float elapsedTime_ = 0.0f;
    float delayTimer_ = 0.0f;
};

// ============================================================================
// 自定义粒子特效
// ============================================================================
class CustomParticleEffect : public CustomEffect {
public:
    explicit CustomParticleEffect(const CustomEffectConfig& config);
    
    bool init() override;
    void update(float dt) override;
    void render(RenderBackend& renderer) override;
    void shutdown() override;
    
    void play();
    void stop();
    
    Ptr<ParticleEmitter> getEmitter() { return emitter_; }

private:
    Ptr<ParticleSystem> particleSystem_;
    Ptr<ParticleEmitter> emitter_;
};

// ============================================================================
// 自定义后处理特效
// ============================================================================
class CustomPostProcessEffect : public CustomEffect, public PostProcessEffect {
public:
    explicit CustomPostProcessEffect(const CustomEffectConfig& config);
    
    bool init() override;
    void update(float dt) override;
    void shutdown() override;
    
    void onShaderBind(GLShader& shader) override;
    
    void setParam(const std::string& name, float value);
    float getParam(const std::string& name) const;

private:
    std::unordered_map<std::string, float> runtimeParams_;
};

// ============================================================================
// 自定义特效工厂
// ============================================================================
class CustomEffectFactory {
public:
    using EffectCreator = std::function<Ptr<CustomEffect>(const CustomEffectConfig&)>;
    
    static CustomEffectFactory& getInstance();
    
    // 注册自定义特效创建器
    void registerEffect(const std::string& typeName, EffectCreator creator);
    
    // 创建特效
    Ptr<CustomEffect> create(const std::string& typeName, const CustomEffectConfig& config);
    
    // 检查是否已注册
    bool isRegistered(const std::string& typeName) const;
    
    // 获取所有已注册的类型
    std::vector<std::string> getRegisteredTypes() const;

private:
    CustomEffectFactory() = default;
    ~CustomEffectFactory() = default;
    CustomEffectFactory(const CustomEffectFactory&) = delete;
    CustomEffectFactory& operator=(const CustomEffectFactory&) = delete;
    
    std::unordered_map<std::string, EffectCreator> creators_;
};

// ============================================================================
// 自定义特效管理器
// ============================================================================
class CustomEffectManager {
public:
    static CustomEffectManager& getInstance();
    
    // ------------------------------------------------------------------------
    // 初始化和关闭
    // ------------------------------------------------------------------------
    bool init();
    void shutdown();
    
    // ------------------------------------------------------------------------
    // 特效管理
    // ------------------------------------------------------------------------
    
    /**
     * @brief 从文件加载特效配置（支持JSON和文本格式）
     * 自动检测格式：JSON格式优先，失败则回退到文本格式
     */
    bool loadFromFile(const std::string& filepath);
    
    /**
     * @brief 从文本文件加载特效配置（简化格式）
     * 格式：每行一个参数，如 EMISSION 100
     */
    bool loadFromTextFile(const std::string& filepath);
    
    /**
     * @brief 保存特效配置到文件
     * @param useJson true=JSON格式, false=文本格式
     */
    bool saveToFile(const std::string& name, const std::string& filepath, bool useJson = true);
    
    /**
     * @brief 保存所有特效配置到一个JSON文件
     */
    bool saveAllToFile(const std::string& filepath);
    
    /**
     * @brief 注册特效配置
     */
    void registerConfig(const std::string& name, const CustomEffectConfig& config);
    
    /**
     * @brief 获取特效配置
     */
    CustomEffectConfig* getConfig(const std::string& name);
    
    /**
     * @brief 移除特效配置
     */
    void removeConfig(const std::string& name);
    
    /**
     * @brief 获取所有配置名称
     */
    std::vector<std::string> getConfigNames() const;

    // ------------------------------------------------------------------------
    // 特效实例管理
    // ------------------------------------------------------------------------
    
    /**
     * @brief 创建特效实例
     */
    Ptr<CustomEffect> createEffect(const std::string& name);
    
    /**
     * @brief 从配置直接创建特效
     */
    Ptr<CustomEffect> createEffectFromConfig(const CustomEffectConfig& config);
    
    /**
     * @brief 销毁特效实例
     */
    void destroyEffect(Ptr<CustomEffect> effect);
    
    /**
     * @brief 更新所有特效
     */
    void update(float dt);
    
    /**
     * @brief 渲染所有特效
     */
    void render(RenderBackend& renderer);
    
    /**
     * @brief 停止所有特效
     */
    void stopAll();

    // ------------------------------------------------------------------------
    // 便捷方法
    // ------------------------------------------------------------------------
    
    /**
     * @brief 播放特效（简写）
     */
    Ptr<CustomEffect> play(const std::string& name, const Vec2& position);
    
    /**
     * @brief 播放特效并自动销毁
     */
    void playOneShot(const std::string& name, const Vec2& position);

private:
    CustomEffectManager() = default;
    ~CustomEffectManager() = default;
    CustomEffectManager(const CustomEffectManager&) = delete;
    CustomEffectManager& operator=(const CustomEffectManager&) = delete;
    
    std::unordered_map<std::string, CustomEffectConfig> configs_;
    std::vector<Ptr<CustomEffect>> activeEffects_;
};

// ============================================================================
// 便捷宏
// ============================================================================
#define E2D_CUSTOM_EFFECT_MANAGER() ::easy2d::CustomEffectManager::getInstance()
#define E2D_CUSTOM_EFFECT_FACTORY() ::easy2d::CustomEffectFactory::getInstance()

// ============================================================================
// 预设特效快速创建
// ============================================================================
class EffectBuilder {
public:
    // 粒子特效
    static CustomEffectConfig Particle(const std::string& name);
    static CustomEffectConfig Fire(const std::string& name);
    static CustomEffectConfig Smoke(const std::string& name);
    static CustomEffectConfig Explosion(const std::string& name);
    static CustomEffectConfig Magic(const std::string& name);
    static CustomEffectConfig Sparkle(const std::string& name);
    
    // 后处理特效
    static CustomEffectConfig Bloom(const std::string& name);
    static CustomEffectConfig Blur(const std::string& name);
    static CustomEffectConfig Vignette(const std::string& name);
    static CustomEffectConfig ColorGrading(const std::string& name);
};

} // namespace easy2d
