#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/color.h>
#include <easy2d/scene/node.h>
#include <easy2d/graphics/texture.h>
#include <vector>
#include <functional>
#include <random>

namespace easy2d {

// ============================================================================
// 粒子数据
// ============================================================================
struct Particle {
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    float rotation;
    float angularVelocity;
    float size;
    float sizeDelta;
    Color color;
    Color colorDelta;
    float life;
    float maxLife;
    bool active;
    
    Particle() : position(Vec2::Zero()), velocity(Vec2::Zero()), 
                 acceleration(Vec2::Zero()), rotation(0.0f), 
                 angularVelocity(0.0f), size(1.0f), sizeDelta(0.0f),
                 color(Colors::White), colorDelta(Colors::Transparent),
                 life(0.0f), maxLife(1.0f), active(false) {}
};

// ============================================================================
// 发射器配置
// ============================================================================
struct EmitterConfig {
    // 发射速率
    float emissionRate = 100.0f;        // 每秒发射粒子数
    float emissionDuration = -1.0f;     // 发射持续时间（-1表示无限）
    
    // 粒子生命周期
    float minLife = 1.0f;
    float maxLife = 2.0f;
    
    // 粒子大小
    float minStartSize = 10.0f;
    float maxStartSize = 20.0f;
    float minEndSize = 0.0f;
    float maxEndSize = 5.0f;
    
    // 粒子速度
    Vec2 minVelocity = Vec2(-50.0f, -50.0f);
    Vec2 maxVelocity = Vec2(50.0f, 50.0f);
    
    // 粒子加速度
    Vec2 acceleration = Vec2(0.0f, -100.0f); // 重力
    
    // 粒子旋转
    float minRotation = 0.0f;
    float maxRotation = 360.0f;
    float minAngularVelocity = -90.0f;
    float maxAngularVelocity = 90.0f;
    
    // 颜色
    Color startColor = Colors::White;
    Color endColor = Colors::Transparent;
    
    // 发射形状
    enum class Shape {
        Point,      // 点发射
        Circle,     // 圆形区域
        Rectangle,  // 矩形区域
        Cone        // 锥形
    };
    Shape shape = Shape::Point;
    float shapeRadius = 50.0f;          // 圆形/锥形半径
    Vec2 shapeSize = Vec2(100.0f, 100.0f); // 矩形大小
    float coneAngle = 45.0f;            // 锥形角度
    
    // 纹理
    Ptr<Texture> texture = nullptr;
    
    // 混合模式
    BlendMode blendMode = BlendMode::Additive;
};

// ============================================================================
// 粒子发射器
// ============================================================================
class ParticleEmitter {
public:
    ParticleEmitter();
    ~ParticleEmitter() = default;

    // ------------------------------------------------------------------------
    // 初始化和关闭
    // ------------------------------------------------------------------------
    bool init(size_t maxParticles);
    void shutdown();

    // ------------------------------------------------------------------------
    // 配置
    // ------------------------------------------------------------------------
    void setConfig(const EmitterConfig& config) { config_ = config; }
    const EmitterConfig& getConfig() const { return config_; }
    
    // 链式配置API
    ParticleEmitter& withEmissionRate(float rate) { config_.emissionRate = rate; return *this; }
    ParticleEmitter& withLife(float minLife, float maxLife) { 
        config_.minLife = minLife; config_.maxLife = maxLife; return *this; 
    }
    ParticleEmitter& withSize(float minStart, float maxStart, float minEnd = 0.0f, float maxEnd = 0.0f) {
        config_.minStartSize = minStart; config_.maxStartSize = maxStart;
        config_.minEndSize = minEnd; config_.maxEndSize = maxEnd;
        return *this;
    }
    ParticleEmitter& withVelocity(const Vec2& minVel, const Vec2& maxVel) {
        config_.minVelocity = minVel; config_.maxVelocity = maxVel; return *this;
    }
    ParticleEmitter& withAcceleration(const Vec2& accel) { config_.acceleration = accel; return *this; }
    ParticleEmitter& withColor(const Color& start, const Color& end) {
        config_.startColor = start; config_.endColor = end; return *this;
    }
    ParticleEmitter& withTexture(Ptr<Texture> texture) { config_.texture = texture; return *this; }
    ParticleEmitter& withBlendMode(BlendMode mode) { config_.blendMode = mode; return *this; }

    // ------------------------------------------------------------------------
    // 发射控制
    // ------------------------------------------------------------------------
    void start();
    void stop();
    void burst(int count);              // 爆发发射
    void reset();
    bool isEmitting() const { return emitting_; }

    // ------------------------------------------------------------------------
    // 更新和渲染
    // ------------------------------------------------------------------------
    void update(float dt);
    void render(RenderBackend& renderer);

    // ------------------------------------------------------------------------
    // 状态查询
    // ------------------------------------------------------------------------
    size_t getActiveParticleCount() const { return activeCount_; }
    size_t getMaxParticles() const { return particles_.size(); }
    bool isActive() const { return activeCount_ > 0 || emitting_; }

    // ------------------------------------------------------------------------
    // 变换
    // ------------------------------------------------------------------------
    void setPosition(const Vec2& pos) { position_ = pos; }
    void setRotation(float rot) { rotation_ = rot; }
    Vec2 getPosition() const { return position_; }
    float getRotation() const { return rotation_; }

private:
    EmitterConfig config_;
    std::vector<Particle> particles_;
    size_t activeCount_ = 0;
    
    Vec2 position_ = Vec2::Zero();
    float rotation_ = 0.0f;
    
    bool emitting_ = false;
    float emissionTimer_ = 0.0f;
    float emissionTime_ = 0.0f;
    
    std::mt19937 rng_;
    
    void emitParticle();
    float randomFloat(float min, float max);
    Vec2 randomPointInShape();
    Vec2 randomVelocity();
};

// ============================================================================
// 粒子系统 - 管理多个发射器
// ============================================================================
class ParticleSystem : public Node {
public:
    ParticleSystem();
    ~ParticleSystem() override = default;

    // ------------------------------------------------------------------------
    // 静态创建方法
    // ------------------------------------------------------------------------
    static Ptr<ParticleSystem> create();

    // ------------------------------------------------------------------------
    // 发射器管理
    // ------------------------------------------------------------------------
    Ptr<ParticleEmitter> addEmitter(const EmitterConfig& config = {});
    void removeEmitter(Ptr<ParticleEmitter> emitter);
    void removeAllEmitters();
    size_t getEmitterCount() const { return emitters_.size(); }

    // ------------------------------------------------------------------------
    // 全局控制
    // ------------------------------------------------------------------------
    void startAll();
    void stopAll();
    void resetAll();

    // ------------------------------------------------------------------------
    // 预设
    // ------------------------------------------------------------------------
    static EmitterConfig PresetFire();
    static EmitterConfig PresetSmoke();
    static EmitterConfig PresetExplosion();
    static EmitterConfig PresetSparkle();
    static EmitterConfig PresetRain();
    static EmitterConfig PresetSnow();

    // ------------------------------------------------------------------------
    // 重写Node方法
    // ------------------------------------------------------------------------
    void onUpdate(float dt) override;
    void onDraw(RenderBackend& renderer) override;

private:
    std::vector<Ptr<ParticleEmitter>> emitters_;
};

// ============================================================================
// 粒子预设（便捷类）
// ============================================================================
class ParticlePreset {
public:
    static EmitterConfig Fire();
    static EmitterConfig Smoke();
    static EmitterConfig Explosion();
    static EmitterConfig Sparkle();
    static EmitterConfig Rain();
    static EmitterConfig Snow();
    static EmitterConfig Magic();
    static EmitterConfig Bubbles();
};

} // namespace easy2d
