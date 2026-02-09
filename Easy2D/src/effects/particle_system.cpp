#include <easy2d/effects/particle_system.h>
#include <easy2d/graphics/render_backend.h>
#include <easy2d/utils/logger.h>
#include <cmath>

namespace easy2d {

// ============================================================================
// ParticleEmitter实现
// ============================================================================

ParticleEmitter::ParticleEmitter() : rng_(std::random_device{}()) {}

bool ParticleEmitter::init(size_t maxParticles) {
    particles_.resize(maxParticles);
    activeCount_ = 0;
    return true;
}

void ParticleEmitter::shutdown() {
    particles_.clear();
    activeCount_ = 0;
}

void ParticleEmitter::start() {
    emitting_ = true;
    emissionTime_ = 0.0f;
}

void ParticleEmitter::stop() {
    emitting_ = false;
}

void ParticleEmitter::burst(int count) {
    for (int i = 0; i < count && activeCount_ < particles_.size(); ++i) {
        emitParticle();
    }
}

void ParticleEmitter::reset() {
    for (auto& particle : particles_) {
        particle.active = false;
    }
    activeCount_ = 0;
    emissionTimer_ = 0.0f;
    emissionTime_ = 0.0f;
}

void ParticleEmitter::update(float dt) {
    // 发射新粒子
    if (emitting_) {
        emissionTime_ += dt;
        
        // 检查持续时间
        if (config_.emissionDuration > 0 && emissionTime_ >= config_.emissionDuration) {
            emitting_ = false;
        }
        
        emissionTimer_ += dt;
        float emissionInterval = 1.0f / config_.emissionRate;
        
        while (emissionTimer_ >= emissionInterval && activeCount_ < particles_.size()) {
            emitParticle();
            emissionTimer_ -= emissionInterval;
        }
    }
    
    // 更新活跃粒子
    size_t newActiveCount = 0;
    for (size_t i = 0; i < activeCount_; ++i) {
        auto& p = particles_[i];
        
        if (!p.active) continue;
        
        // 更新生命周期
        p.life -= dt;
        if (p.life <= 0.0f) {
            p.active = false;
            continue;
        }
        
        // 更新物理
        p.velocity += p.acceleration * dt;
        p.position += p.velocity * dt;
        p.rotation += p.angularVelocity * dt;
        
        // 更新大小
        p.size += p.sizeDelta * dt;
        if (p.size < 0.0f) p.size = 0.0f;
        
        // 更新颜色
        p.color += p.colorDelta * dt;
        
        // 保持活跃
        if (newActiveCount != i) {
            particles_[newActiveCount] = p;
        }
        newActiveCount++;
    }
    
    activeCount_ = newActiveCount;
}

void ParticleEmitter::render(RenderBackend& renderer) {
    if (activeCount_ == 0) return;
    
    // 设置混合模式
    renderer.setBlendMode(config_.blendMode);
    
    // 渲染所有活跃粒子
    if (config_.texture) {
        // 使用纹理批量渲染
        renderer.beginSpriteBatch();
        
        for (size_t i = 0; i < activeCount_; ++i) {
            const auto& p = particles_[i];
            if (!p.active) continue;
            
            // 计算目标矩形
            float halfSize = p.size * 0.5f;
            Rect destRect(
                p.position.x - halfSize,
                p.position.y - halfSize,
                p.size,
                p.size
            );
            
            renderer.drawSprite(*config_.texture, destRect, 
                              Rect(0, 0, config_.texture->getWidth(), config_.texture->getHeight()),
                              p.color, p.rotation, Vec2(0.5f, 0.5f));
        }
        
        renderer.endSpriteBatch();
    } else {
        // 没有纹理，使用圆形填充渲染
        for (size_t i = 0; i < activeCount_; ++i) {
            const auto& p = particles_[i];
            if (!p.active) continue;
            
            // 渲染圆形粒子
            renderer.fillCircle(p.position, p.size * 0.5f, p.color);
        }
    }
}

void ParticleEmitter::emitParticle() {
    if (activeCount_ >= particles_.size()) return;
    
    Particle& p = particles_[activeCount_];
    p.active = true;
    
    // 位置
    p.position = position_ + randomPointInShape();
    
    // 速度
    p.velocity = randomVelocity();
    
    // 加速度
    p.acceleration = config_.acceleration;
    
    // 旋转
    p.rotation = randomFloat(config_.minRotation, config_.maxRotation);
    p.angularVelocity = randomFloat(config_.minAngularVelocity, config_.maxAngularVelocity);
    
    // 大小
    float startSize = randomFloat(config_.minStartSize, config_.maxStartSize);
    float endSize = randomFloat(config_.minEndSize, config_.maxEndSize);
    p.size = startSize;
    
    // 生命周期
    p.maxLife = randomFloat(config_.minLife, config_.maxLife);
    p.life = p.maxLife;
    
    // 计算每帧变化量
    if (p.maxLife > 0.0f) {
        p.sizeDelta = (endSize - startSize) / p.maxLife;
        p.colorDelta = (config_.endColor - config_.startColor) / p.maxLife;
    }
    
    // 颜色
    p.color = config_.startColor;
    
    activeCount_++;
}

float ParticleEmitter::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng_);
}

Vec2 ParticleEmitter::randomPointInShape() {
    switch (config_.shape) {
        case EmitterConfig::Shape::Point:
            return Vec2::Zero();
            
        case EmitterConfig::Shape::Circle: {
            float angle = randomFloat(0.0f, 2.0f * 3.14159265359f);
            float radius = randomFloat(0.0f, config_.shapeRadius);
            return Vec2(
                std::cos(angle) * radius,
                std::sin(angle) * radius
            );
        }
        
        case EmitterConfig::Shape::Rectangle:
            return Vec2(
                randomFloat(-config_.shapeSize.x * 0.5f, config_.shapeSize.x * 0.5f),
                randomFloat(-config_.shapeSize.y * 0.5f, config_.shapeSize.y * 0.5f)
            );
            
        case EmitterConfig::Shape::Cone: {
            float angle = randomFloat(-config_.coneAngle * 0.5f, config_.coneAngle * 0.5f);
            float radius = randomFloat(0.0f, config_.shapeRadius);
            float rad = angle * 3.14159265359f / 180.0f;
            return Vec2(
                std::cos(rad) * radius,
                std::sin(rad) * radius
            );
        }
    }
    
    return Vec2::Zero();
}

Vec2 ParticleEmitter::randomVelocity() {
    return Vec2(
        randomFloat(config_.minVelocity.x, config_.maxVelocity.x),
        randomFloat(config_.minVelocity.y, config_.maxVelocity.y)
    );
}

// ============================================================================
// ParticleSystem实现
// ============================================================================

ParticleSystem::ParticleSystem() {}

Ptr<ParticleSystem> ParticleSystem::create() {
    return std::make_shared<ParticleSystem>();
}

Ptr<ParticleEmitter> ParticleSystem::addEmitter(const EmitterConfig& config) {
    auto emitter = std::make_shared<ParticleEmitter>();
    emitter->setConfig(config);
    emitter->init(1000); // 默认最大1000个粒子
    emitters_.push_back(emitter);
    return emitter;
}

void ParticleSystem::removeEmitter(Ptr<ParticleEmitter> emitter) {
    auto it = std::find(emitters_.begin(), emitters_.end(), emitter);
    if (it != emitters_.end()) {
        (*it)->shutdown();
        emitters_.erase(it);
    }
}

void ParticleSystem::removeAllEmitters() {
    for (auto& emitter : emitters_) {
        emitter->shutdown();
    }
    emitters_.clear();
}

void ParticleSystem::startAll() {
    for (auto& emitter : emitters_) {
        emitter->start();
    }
}

void ParticleSystem::stopAll() {
    for (auto& emitter : emitters_) {
        emitter->stop();
    }
}

void ParticleSystem::resetAll() {
    for (auto& emitter : emitters_) {
        emitter->reset();
    }
}

void ParticleSystem::onUpdate(float dt) {
    // 获取粒子系统的世界位置
    auto worldPos = convertToWorldSpace(Vec2::Zero());
    
    for (auto& emitter : emitters_) {
        // 更新发射器位置为粒子系统的世界位置
        emitter->setPosition(worldPos);
        // 更新发射器
        emitter->update(dt);
    }
}

void ParticleSystem::onDraw(RenderBackend& renderer) {
    for (auto& emitter : emitters_) {
        emitter->render(renderer);
    }
}

// ============================================================================
// 预设实现
// ============================================================================

EmitterConfig ParticlePreset::Fire() {
    EmitterConfig config;
    config.emissionRate = 200.0f;
    config.minLife = 0.5f;
    config.maxLife = 1.5f;
    config.minStartSize = 20.0f;
    config.maxStartSize = 40.0f;
    config.minEndSize = 5.0f;
    config.maxEndSize = 10.0f;
    config.minVelocity = Vec2(-30.0f, -150.0f);  // 向上（负y）
    config.maxVelocity = Vec2(30.0f, -50.0f);
    config.acceleration = Vec2(0.0f, 0.0f);
    config.startColor = Color(1.0f, 0.8f, 0.2f, 1.0f);  // 黄色
    config.endColor = Color(1.0f, 0.2f, 0.0f, 0.0f);    // 红色透明
    config.blendMode = BlendMode::Additive;
    return config;
}

EmitterConfig ParticlePreset::Smoke() {
    EmitterConfig config;
    config.emissionRate = 50.0f;
    config.minLife = 2.0f;
    config.maxLife = 4.0f;
    config.minStartSize = 30.0f;
    config.maxStartSize = 60.0f;
    config.minEndSize = 80.0f;
    config.maxEndSize = 120.0f;
    config.minVelocity = Vec2(-20.0f, -60.0f);  // 向上（负y）
    config.maxVelocity = Vec2(20.0f, -30.0f);
    config.acceleration = Vec2(0.0f, -10.0f);  // 向上加速度
    config.startColor = Color(0.5f, 0.5f, 0.5f, 0.5f);  // 灰色半透明
    config.endColor = Color(0.3f, 0.3f, 0.3f, 0.0f);    // 深灰透明
    config.blendMode = BlendMode::Alpha;
    return config;
}

EmitterConfig ParticlePreset::Explosion() {
    EmitterConfig config;
    config.emissionRate = 1000.0f;
    config.emissionDuration = 0.1f;  // 瞬间爆发
    config.minLife = 0.5f;
    config.maxLife = 1.5f;
    config.minStartSize = 10.0f;
    config.maxStartSize = 30.0f;
    config.minEndSize = 0.0f;
    config.maxEndSize = 5.0f;
    config.minVelocity = Vec2(-300.0f, -300.0f);
    config.maxVelocity = Vec2(300.0f, 300.0f);
    config.acceleration = Vec2(0.0f, -50.0f);
    config.startColor = Color(1.0f, 1.0f, 0.5f, 1.0f);  // 亮黄
    config.endColor = Color(1.0f, 0.3f, 0.0f, 0.0f);    // 橙红透明
    config.blendMode = BlendMode::Additive;
    return config;
}

EmitterConfig ParticlePreset::Sparkle() {
    EmitterConfig config;
    config.emissionRate = 20.0f;
    config.minLife = 0.2f;
    config.maxLife = 0.8f;
    config.minStartSize = 2.0f;
    config.maxStartSize = 5.0f;
    config.minEndSize = 0.0f;
    config.maxEndSize = 2.0f;
    config.minVelocity = Vec2(-10.0f, -10.0f);
    config.maxVelocity = Vec2(10.0f, 10.0f);
    config.acceleration = Vec2(0.0f, 0.0f);
    config.startColor = Color(1.0f, 1.0f, 1.0f, 1.0f);  // 白色
    config.endColor = Color(1.0f, 1.0f, 1.0f, 0.0f);    // 透明
    config.blendMode = BlendMode::Additive;
    return config;
}

EmitterConfig ParticlePreset::Rain() {
    EmitterConfig config;
    config.emissionRate = 500.0f;
    config.minLife = 1.0f;
    config.maxLife = 2.0f;
    config.minStartSize = 2.0f;
    config.maxStartSize = 4.0f;
    config.minEndSize = 2.0f;
    config.maxEndSize = 4.0f;
    config.minVelocity = Vec2(-100.0f, -400.0f);
    config.maxVelocity = Vec2(100.0f, -600.0f);
    config.acceleration = Vec2(0.0f, -100.0f);
    config.startColor = Color(0.7f, 0.8f, 1.0f, 0.6f);  // 淡蓝
    config.endColor = Color(0.7f, 0.8f, 1.0f, 0.3f);    // 淡蓝半透明
    config.shape = EmitterConfig::Shape::Rectangle;
    config.shapeSize = Vec2(800.0f, 100.0f);  // 在顶部区域发射
    config.blendMode = BlendMode::Alpha;
    return config;
}

EmitterConfig ParticlePreset::Snow() {
    EmitterConfig config;
    config.emissionRate = 100.0f;
    config.minLife = 3.0f;
    config.maxLife = 6.0f;
    config.minStartSize = 5.0f;
    config.maxStartSize = 10.0f;
    config.minEndSize = 5.0f;
    config.maxEndSize = 10.0f;
    config.minVelocity = Vec2(-30.0f, -30.0f);
    config.maxVelocity = Vec2(30.0f, -80.0f);
    config.acceleration = Vec2(0.0f, 0.0f);
    config.startColor = Color(1.0f, 1.0f, 1.0f, 0.8f);  // 白色
    config.endColor = Color(1.0f, 1.0f, 1.0f, 0.8f);    // 白色
    config.shape = EmitterConfig::Shape::Rectangle;
    config.shapeSize = Vec2(800.0f, 100.0f);
    config.blendMode = BlendMode::Alpha;
    return config;
}

EmitterConfig ParticlePreset::Magic() {
    EmitterConfig config;
    config.emissionRate = 100.0f;
    config.minLife = 1.0f;
    config.maxLife = 2.0f;
    config.minStartSize = 5.0f;
    config.maxStartSize = 15.0f;
    config.minEndSize = 0.0f;
    config.maxEndSize = 5.0f;
    config.minVelocity = Vec2(-50.0f, -50.0f);  // 主要向上
    config.maxVelocity = Vec2(50.0f, -50.0f);
    config.acceleration = Vec2(0.0f, -20.0f);  // 向上加速度
    config.startColor = Color(0.5f, 0.2f, 1.0f, 1.0f);  // 紫色
    config.endColor = Color(0.2f, 0.8f, 1.0f, 0.0f);    // 青色透明
    config.blendMode = BlendMode::Additive;
    return config;
}

EmitterConfig ParticlePreset::Bubbles() {
    EmitterConfig config;
    config.emissionRate = 30.0f;
    config.minLife = 2.0f;
    config.maxLife = 4.0f;
    config.minStartSize = 5.0f;
    config.maxStartSize = 15.0f;
    config.minEndSize = 5.0f;
    config.maxEndSize = 15.0f;
    config.minVelocity = Vec2(-20.0f, 20.0f);
    config.maxVelocity = Vec2(20.0f, 60.0f);
    config.acceleration = Vec2(0.0f, 30.0f);
    config.startColor = Color(0.8f, 0.9f, 1.0f, 0.4f);  // 淡蓝透明
    config.endColor = Color(0.8f, 0.9f, 1.0f, 0.1f);    // 更透明
    config.blendMode = BlendMode::Alpha;
    return config;
}

} // namespace easy2d
