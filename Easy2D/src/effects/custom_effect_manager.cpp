#include <easy2d/effects/custom_effect_manager.h>
#include <easy2d/utils/logger.h>
#include <json/json.hpp>
#include <fstream>
#include <sstream>

namespace easy2d {

using json = nlohmann::json;

// ============================================================================
// JSON序列化辅助函数
// ============================================================================

/**
 * @brief 将Vec2转换为JSON数组
 */
static json vec2ToJson(const Vec2& v) {
    return json::array({v.x, v.y});
}

/**
 * @brief 从JSON数组解析Vec2
 */
static Vec2 jsonToVec2(const json& j) {
    if (j.is_array() && j.size() >= 2) {
        return Vec2(j[0].get<float>(), j[1].get<float>());
    }
    return Vec2();
}

/**
 * @brief 将Color转换为JSON数组
 */
static json colorToJson(const Color& c) {
    return json::array({c.r, c.g, c.b, c.a});
}

/**
 * @brief 从JSON数组解析Color
 */
static Color jsonToColor(const json& j) {
    if (j.is_array() && j.size() >= 4) {
        return Color(j[0].get<float>(), j[1].get<float>(), 
                     j[2].get<float>(), j[3].get<float>());
    }
    return Colors::White;
}

/**
 * @brief 将EmitterConfig转换为JSON
 */
static json emitterConfigToJson(const EmitterConfig& config) {
    json j;
    j["emissionRate"] = config.emissionRate;
    j["life"] = json::array({config.minLife, config.maxLife});
    j["startSize"] = json::array({config.minStartSize, config.maxStartSize});
    j["endSize"] = json::array({config.minEndSize, config.maxEndSize});
    j["velocity"] = json::object({
        {"min", vec2ToJson(config.minVelocity)},
        {"max", vec2ToJson(config.maxVelocity)}
    });
    j["acceleration"] = vec2ToJson(config.acceleration);
    j["startColor"] = colorToJson(config.startColor);
    j["endColor"] = colorToJson(config.endColor);
    j["blendMode"] = static_cast<int>(config.blendMode);
    j["shape"] = static_cast<int>(config.shape);
    j["shapeRadius"] = config.shapeRadius;
    return j;
}

/**
 * @brief 从JSON解析EmitterConfig
 */
static EmitterConfig jsonToEmitterConfig(const json& j) {
    EmitterConfig config;
    
    if (j.contains("emissionRate")) {
        config.emissionRate = j["emissionRate"].get<float>();
    }
    if (j.contains("life") && j["life"].is_array() && j["life"].size() >= 2) {
        config.minLife = j["life"][0].get<float>();
        config.maxLife = j["life"][1].get<float>();
    }
    if (j.contains("startSize") && j["startSize"].is_array() && j["startSize"].size() >= 2) {
        config.minStartSize = j["startSize"][0].get<float>();
        config.maxStartSize = j["startSize"][1].get<float>();
    }
    if (j.contains("endSize") && j["endSize"].is_array() && j["endSize"].size() >= 2) {
        config.minEndSize = j["endSize"][0].get<float>();
        config.maxEndSize = j["endSize"][1].get<float>();
    }
    if (j.contains("velocity")) {
        const auto& vel = j["velocity"];
        if (vel.contains("min")) {
            config.minVelocity = jsonToVec2(vel["min"]);
        }
        if (vel.contains("max")) {
            config.maxVelocity = jsonToVec2(vel["max"]);
        }
    }
    if (j.contains("acceleration")) {
        config.acceleration = jsonToVec2(j["acceleration"]);
    }
    if (j.contains("startColor")) {
        config.startColor = jsonToColor(j["startColor"]);
    }
    if (j.contains("endColor")) {
        config.endColor = jsonToColor(j["endColor"]);
    }
    if (j.contains("blendMode")) {
        config.blendMode = static_cast<BlendMode>(j["blendMode"].get<int>());
    }
    if (j.contains("shape")) {
        config.shape = static_cast<EmitterConfig::Shape>(j["shape"].get<int>());
    }
    if (j.contains("shapeRadius")) {
        config.shapeRadius = j["shapeRadius"].get<float>();
    }
    
    return config;
}

/**
 * @brief 将CustomEffectConfig转换为JSON
 */
static json effectConfigToJson(const CustomEffectConfig& config) {
    json j;
    j["name"] = config.name;
    j["type"] = static_cast<int>(config.type);
    j["description"] = config.description;
    j["duration"] = config.duration;
    j["loop"] = config.loop;
    j["delay"] = config.delay;
    
    if (config.type == CustomEffectType::Particle) {
        j["emitter"] = emitterConfigToJson(config.emitterConfig);
    } else if (config.type == CustomEffectType::PostProcess) {
        j["shaderVert"] = config.shaderVertPath;
        j["shaderFrag"] = config.shaderFragPath;
        j["params"] = config.shaderParams;
    }
    
    return j;
}

/**
 * @brief 从JSON解析CustomEffectConfig
 */
static CustomEffectConfig jsonToEffectConfig(const json& j) {
    CustomEffectConfig config;
    
    if (j.contains("name")) {
        config.name = j["name"].get<std::string>();
    }
    if (j.contains("type")) {
        config.type = static_cast<CustomEffectType>(j["type"].get<int>());
    }
    if (j.contains("description")) {
        config.description = j["description"].get<std::string>();
    }
    if (j.contains("duration")) {
        config.duration = j["duration"].get<float>();
    }
    if (j.contains("loop")) {
        config.loop = j["loop"].get<bool>();
    }
    if (j.contains("delay")) {
        config.delay = j["delay"].get<float>();
    }
    if (j.contains("emitter")) {
        config.emitterConfig = jsonToEmitterConfig(j["emitter"]);
    }
    if (j.contains("shaderVert")) {
        config.shaderVertPath = j["shaderVert"].get<std::string>();
    }
    if (j.contains("shaderFrag")) {
        config.shaderFragPath = j["shaderFrag"].get<std::string>();
    }
    if (j.contains("params")) {
        for (auto& [key, value] : j["params"].items()) {
            config.shaderParams[key] = value.get<float>();
        }
    }
    
    return config;
}

// ============================================================================
// CustomEffect实现
// ============================================================================

CustomEffect::CustomEffect(const CustomEffectConfig& config) : config_(config) {}

bool CustomEffect::init() {
    return true;
}

void CustomEffect::update(float dt) {
    if (!playing_ || paused_ || finished_) return;
    
    // 处理延迟
    if (delayTimer_ < config_.delay) {
        delayTimer_ += dt;
        return;
    }
    
    elapsedTime_ += dt;
    
    // 检查是否结束
    if (config_.duration > 0 && elapsedTime_ >= config_.duration) {
        if (config_.loop) {
            elapsedTime_ = 0.0f;
        } else {
            finished_ = true;
            playing_ = false;
        }
    }
}

void CustomEffect::render(RenderBackend& renderer) {
    // 基类不渲染任何内容
}

void CustomEffect::shutdown() {
    playing_ = false;
    paused_ = false;
    finished_ = true;
}

void CustomEffect::play() {
    if (finished_) {
        reset();
    }
    playing_ = true;
    paused_ = false;
}

void CustomEffect::pause() {
    paused_ = true;
}

void CustomEffect::stop() {
    playing_ = false;
    paused_ = false;
}

void CustomEffect::reset() {
    elapsedTime_ = 0.0f;
    delayTimer_ = 0.0f;
    finished_ = false;
    playing_ = false;
    paused_ = false;
}

// ============================================================================
// CustomParticleEffect实现
// ============================================================================

CustomParticleEffect::CustomParticleEffect(const CustomEffectConfig& config)
    : CustomEffect(config) {}

bool CustomParticleEffect::init() {
    particleSystem_ = ParticleSystem::create();
    if (!particleSystem_) {
        E2D_ERROR("创建粒子系统失败");
        return false;
    }
    
    emitter_ = particleSystem_->addEmitter(config_.emitterConfig);
    if (!emitter_) {
        E2D_ERROR("创建粒子发射器失败");
        return false;
    }
    
    // 初始化时启动发射器
    emitter_->start();
    
    return true;
}

void CustomParticleEffect::play() {
    CustomEffect::play();
    if (emitter_) {
        emitter_->start();
    }
}

void CustomParticleEffect::stop() {
    CustomEffect::stop();
    if (emitter_) {
        emitter_->stop();
    }
}

void CustomParticleEffect::update(float dt) {
    CustomEffect::update(dt);
    
    if (particleSystem_) {
        particleSystem_->setPosition(position_);
        particleSystem_->onUpdate(dt);
    }

}

void CustomParticleEffect::render(RenderBackend& renderer) {
    if (particleSystem_) {
        particleSystem_->onDraw(renderer);
    }
}

void CustomParticleEffect::shutdown() {
    if (emitter_) {
        emitter_->stop();
        emitter_.reset();
    }
    if (particleSystem_) {
        particleSystem_->removeAllEmitters();
        particleSystem_.reset();
    }
    CustomEffect::shutdown();
}

// ============================================================================
// CustomPostProcessEffect实现
// ============================================================================

CustomPostProcessEffect::CustomPostProcessEffect(const CustomEffectConfig& config)
    : CustomEffect(config), PostProcessEffect(config.name) {}

bool CustomPostProcessEffect::init() {
    if (!config_.shaderVertPath.empty() && !config_.shaderFragPath.empty()) {
        if (!loadShaderFromFile(config_.shaderVertPath, config_.shaderFragPath)) {
            E2D_ERROR("加载后处理Shader失败");
            return false;
        }
    }
    
    runtimeParams_ = config_.shaderParams;
    return true;
}

void CustomPostProcessEffect::update(float dt) {
    CustomEffect::update(dt);
}

void CustomPostProcessEffect::shutdown() {
    PostProcessEffect::shutdown();
    CustomEffect::shutdown();
}

void CustomPostProcessEffect::onShaderBind(GLShader& shader) {
    for (const auto& [name, value] : runtimeParams_) {
        shader.setFloat(name, value);
    }
}

void CustomPostProcessEffect::setParam(const std::string& name, float value) {
    runtimeParams_[name] = value;
}

float CustomPostProcessEffect::getParam(const std::string& name) const {
    auto it = runtimeParams_.find(name);
    if (it != runtimeParams_.end()) {
        return it->second;
    }
    return 0.0f;
}

// ============================================================================
// CustomEffectFactory实现
// ============================================================================

CustomEffectFactory& CustomEffectFactory::getInstance() {
    static CustomEffectFactory instance;
    return instance;
}

void CustomEffectFactory::registerEffect(const std::string& typeName, EffectCreator creator) {
    creators_[typeName] = creator;
    E2D_INFO("注册自定义特效类型: {}", typeName);
}

Ptr<CustomEffect> CustomEffectFactory::create(const std::string& typeName, const CustomEffectConfig& config) {
    auto it = creators_.find(typeName);
    if (it != creators_.end()) {
        return it->second(config);
    }
    
    // 默认创建器
    if (typeName == "Particle") {
        return std::make_shared<CustomParticleEffect>(config);
    } else if (typeName == "PostProcess") {
        return std::make_shared<CustomPostProcessEffect>(config);
    }
    
    E2D_ERROR("未知的特效类型: {}", typeName);
    return nullptr;
}

bool CustomEffectFactory::isRegistered(const std::string& typeName) const {
    return creators_.find(typeName) != creators_.end();
}

std::vector<std::string> CustomEffectFactory::getRegisteredTypes() const {
    std::vector<std::string> types;
    for (const auto& [name, _] : creators_) {
        types.push_back(name);
    }
    return types;
}

// ============================================================================
// CustomEffectManager实现
// ============================================================================

CustomEffectManager& CustomEffectManager::getInstance() {
    static CustomEffectManager instance;
    return instance;
}

bool CustomEffectManager::init() {
    E2D_INFO("初始化自定义特效管理器...");
    
    // 注册默认特效类型
    auto& factory = E2D_CUSTOM_EFFECT_FACTORY();
    factory.registerEffect("Particle", [](const CustomEffectConfig& config) {
        return std::make_shared<CustomParticleEffect>(config);
    });
    factory.registerEffect("PostProcess", [](const CustomEffectConfig& config) {
        return std::make_shared<CustomPostProcessEffect>(config);
    });
    
    E2D_INFO("自定义特效管理器初始化完成");
    return true;
}

void CustomEffectManager::shutdown() {
    E2D_INFO("关闭自定义特效管理器...");
    stopAll();
    activeEffects_.clear();
    configs_.clear();
}

bool CustomEffectManager::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        E2D_ERROR("无法打开特效配置文件: {}", filepath);
        return false;
    }
    
    try {
        // 尝试解析为JSON
        json j;
        file >> j;
        file.close();
        
        if (j.is_array()) {
            // 多个特效配置数组
            for (const auto& effectJson : j) {
                auto config = jsonToEffectConfig(effectJson);
                if (!config.name.empty()) {
                    registerConfig(config.name, config);
                }
            }
        } else if (j.is_object()) {
            // 单个特效配置
            auto config = jsonToEffectConfig(j);
            if (!config.name.empty()) {
                registerConfig(config.name, config);
            }
        }
        
        E2D_INFO("从JSON文件加载特效配置: {}", filepath);
        return true;
        
    } catch (const json::exception& e) {
        // JSON解析失败，回退到文本格式
        file.close();
        return loadFromTextFile(filepath);
    }
}

bool CustomEffectManager::loadFromTextFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        E2D_ERROR("无法打开特效配置文件: {}", filepath);
        return false;
    }
    
    // 简化格式：每行一个特效配置
    // 格式: EFFECT name type
    //       PARAM key value
    //       END
    
    std::string line;
    CustomEffectConfig currentConfig;
    bool inEffect = false;
    
    while (std::getline(file, line)) {
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "EFFECT") {
            // 开始新特效
            if (inEffect) {
                // 保存上一个特效
                registerConfig(currentConfig.name, currentConfig);
            }
            inEffect = true;
            currentConfig = CustomEffectConfig();
            
            std::string type;
            iss >> currentConfig.name >> type;
            
            if (type == "Particle") {
                currentConfig.type = CustomEffectType::Particle;
            } else if (type == "PostProcess") {
                currentConfig.type = CustomEffectType::PostProcess;
            } else {
                currentConfig.type = CustomEffectType::Particle;
            }
        } else if (cmd == "DESC") {
            std::getline(iss, currentConfig.description);
            // 去除前导空格
            if (!currentConfig.description.empty() && currentConfig.description[0] == ' ') {
                currentConfig.description = currentConfig.description.substr(1);
            }
        } else if (cmd == "DURATION") {
            iss >> currentConfig.duration;
        } else if (cmd == "LOOP") {
            std::string val;
            iss >> val;
            currentConfig.loop = (val == "true" || val == "1");
        } else if (cmd == "EMISSION") {
            iss >> currentConfig.emitterConfig.emissionRate;
        } else if (cmd == "LIFE") {
            iss >> currentConfig.emitterConfig.minLife >> currentConfig.emitterConfig.maxLife;
        } else if (cmd == "SIZE_START") {
            iss >> currentConfig.emitterConfig.minStartSize >> currentConfig.emitterConfig.maxStartSize;
        } else if (cmd == "SIZE_END") {
            iss >> currentConfig.emitterConfig.minEndSize >> currentConfig.emitterConfig.maxEndSize;
        } else if (cmd == "VELOCITY") {
            iss >> currentConfig.emitterConfig.minVelocity.x 
                >> currentConfig.emitterConfig.minVelocity.y
                >> currentConfig.emitterConfig.maxVelocity.x
                >> currentConfig.emitterConfig.maxVelocity.y;
        } else if (cmd == "ACCEL") {
            iss >> currentConfig.emitterConfig.acceleration.x 
                >> currentConfig.emitterConfig.acceleration.y;
        } else if (cmd == "COLOR_START") {
            iss >> currentConfig.emitterConfig.startColor.r 
                >> currentConfig.emitterConfig.startColor.g
                >> currentConfig.emitterConfig.startColor.b
                >> currentConfig.emitterConfig.startColor.a;
        } else if (cmd == "COLOR_END") {
            iss >> currentConfig.emitterConfig.endColor.r 
                >> currentConfig.emitterConfig.endColor.g
                >> currentConfig.emitterConfig.endColor.b
                >> currentConfig.emitterConfig.endColor.a;
        } else if (cmd == "BLEND") {
            std::string mode;
            iss >> mode;
            if (mode == "Additive") {
                currentConfig.emitterConfig.blendMode = BlendMode::Additive;
            } else if (mode == "Alpha") {
                currentConfig.emitterConfig.blendMode = BlendMode::Alpha;
            } else {
                currentConfig.emitterConfig.blendMode = BlendMode::None;
            }
        } else if (cmd == "END") {
            // 结束当前特效
            if (inEffect) {
                registerConfig(currentConfig.name, currentConfig);
                inEffect = false;
            }
        }
    }
    
    // 保存最后一个特效
    if (inEffect) {
        registerConfig(currentConfig.name, currentConfig);
    }
    
    file.close();
    E2D_INFO("从文本文件加载特效配置: {}", filepath);
    return true;
}

bool CustomEffectManager::saveToFile(const std::string& name, const std::string& filepath, bool useJson) {
    auto it = configs_.find(name);
    if (it == configs_.end()) {
        E2D_ERROR("特效配置不存在: {}", name);
        return false;
    }
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        E2D_ERROR("无法创建文件: {}", filepath);
        return false;
    }
    
    if (useJson) {
        // 保存为JSON格式
        json j = effectConfigToJson(it->second);
        file << j.dump(2);  // 缩进2个空格，更易读
        E2D_INFO("保存特效配置到JSON文件: {}", filepath);
    } else {
        // 保存为文本格式
        const auto& config = it->second;
        
        file << "# Easy2D Custom Effect Config\n";
        file << "# Generated automatically\n\n";
        
        file << "EFFECT " << config.name << " ";
        if (config.type == CustomEffectType::Particle) {
            file << "Particle\n";
        } else if (config.type == CustomEffectType::PostProcess) {
            file << "PostProcess\n";
        } else {
            file << "Particle\n";
        }
        
        file << "DESC " << config.description << "\n";
        file << "DURATION " << config.duration << "\n";
        file << "LOOP " << (config.loop ? "true" : "false") << "\n";
        
        if (config.type == CustomEffectType::Particle) {
            const auto& ec = config.emitterConfig;
            file << "EMISSION " << ec.emissionRate << "\n";
            file << "LIFE " << ec.minLife << " " << ec.maxLife << "\n";
            file << "SIZE_START " << ec.minStartSize << " " << ec.maxStartSize << "\n";
            file << "SIZE_END " << ec.minEndSize << " " << ec.maxEndSize << "\n";
            file << "VELOCITY " << ec.minVelocity.x << " " << ec.minVelocity.y << " "
                 << ec.maxVelocity.x << " " << ec.maxVelocity.y << "\n";
            file << "ACCEL " << ec.acceleration.x << " " << ec.acceleration.y << "\n";
            file << "COLOR_START " << ec.startColor.r << " " << ec.startColor.g << " "
                 << ec.startColor.b << " " << ec.startColor.a << "\n";
            file << "COLOR_END " << ec.endColor.r << " " << ec.endColor.g << " "
                 << ec.endColor.b << " " << ec.endColor.a << "\n";
            
            file << "BLEND ";
            switch (ec.blendMode) {
                case BlendMode::Additive:
                    file << "Additive\n";
                    break;
                case BlendMode::Alpha:
                    file << "Alpha\n";
                    break;
                default:
                    file << "None\n";
                    break;
            }
        }
        
        file << "END\n";
        E2D_INFO("保存特效配置到文本文件: {}", filepath);
    }
    
    file.close();
    return true;
}

bool CustomEffectManager::saveAllToFile(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        E2D_ERROR("无法创建文件: {}", filepath);
        return false;
    }
    
    json effectsArray = json::array();
    for (const auto& [name, config] : configs_) {
        effectsArray.push_back(effectConfigToJson(config));
    }
    
    file << effectsArray.dump(2);
    file.close();
    
    E2D_INFO("保存所有特效配置到: {}", filepath);
    return true;
}

void CustomEffectManager::registerConfig(const std::string& name, const CustomEffectConfig& config) {
    configs_[name] = config;
    E2D_INFO("注册特效配置: {}", name);
}

CustomEffectConfig* CustomEffectManager::getConfig(const std::string& name) {
    auto it = configs_.find(name);
    if (it != configs_.end()) {
        return &it->second;
    }
    return nullptr;
}

void CustomEffectManager::removeConfig(const std::string& name) {
    configs_.erase(name);
}

std::vector<std::string> CustomEffectManager::getConfigNames() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : configs_) {
        names.push_back(name);
    }
    return names;
}

Ptr<CustomEffect> CustomEffectManager::createEffect(const std::string& name) {
    auto config = getConfig(name);
    if (!config) {
        E2D_ERROR("特效配置不存在: {}", name);
        return nullptr;
    }
    
    return createEffectFromConfig(*config);
}

Ptr<CustomEffect> CustomEffectManager::createEffectFromConfig(const CustomEffectConfig& config) {
    std::string typeName;
    switch (config.type) {
        case CustomEffectType::Particle:
            typeName = "Particle";
            break;
        case CustomEffectType::PostProcess:
            typeName = "PostProcess";
            break;
        default:
            typeName = "Particle";
            break;
    }
    
    auto effect = E2D_CUSTOM_EFFECT_FACTORY().create(typeName, config);
    if (effect && effect->init()) {
        activeEffects_.push_back(effect);
        return effect;
    }
    
    return nullptr;
}

void CustomEffectManager::destroyEffect(Ptr<CustomEffect> effect) {
    if (!effect) return;
    
    effect->shutdown();
    
    auto it = std::find(activeEffects_.begin(), activeEffects_.end(), effect);
    if (it != activeEffects_.end()) {
        activeEffects_.erase(it);
    }
}

void CustomEffectManager::update(float dt) {
    for (auto& effect : activeEffects_) {
        if (effect->isPlaying()) {
            effect->update(dt);
        }
    }
    
    // 清理已完成的特效
    activeEffects_.erase(
        std::remove_if(activeEffects_.begin(), activeEffects_.end(),
            [](const Ptr<CustomEffect>& effect) {
                return effect->isFinished();
            }),
        activeEffects_.end()
    );
}

void CustomEffectManager::render(RenderBackend& renderer) {
    for (auto& effect : activeEffects_) {
        if (effect->isPlaying()) {
            effect->render(renderer);
        }
    }
}

void CustomEffectManager::stopAll() {
    for (auto& effect : activeEffects_) {
        effect->stop();
    }
}

Ptr<CustomEffect> CustomEffectManager::play(const std::string& name, const Vec2& position) {
    auto effect = createEffect(name);
    if (effect) {
        effect->setPosition(position);
        effect->play();
    }
    return effect;
}

void CustomEffectManager::playOneShot(const std::string& name, const Vec2& position) {
    auto effect = play(name, position);
    if (effect) {
        // 设置非循环，播放一次后自动销毁
        effect->play();
    }
}

// ============================================================================
// EffectBuilder实现
// ============================================================================

CustomEffectConfig EffectBuilder::Particle(const std::string& name) {
    CustomEffectConfig config;
    config.name = name;
    config.type = CustomEffectType::Particle;
    config.duration = -1.0f;
    config.loop = true;
    config.delay = 0.0f;
    
    // 默认粒子配置
    config.emitterConfig.emissionRate = 100.0f;
    config.emitterConfig.minLife = 1.0f;
    config.emitterConfig.maxLife = 2.0f;
    config.emitterConfig.minStartSize = 10.0f;
    config.emitterConfig.maxStartSize = 20.0f;
    config.emitterConfig.minEndSize = 0.0f;
    config.emitterConfig.maxEndSize = 5.0f;
    config.emitterConfig.minVelocity = Vec2(-50.0f, -50.0f);
    config.emitterConfig.maxVelocity = Vec2(50.0f, 50.0f);
    config.emitterConfig.acceleration = Vec2(0.0f, 0.0f);
    config.emitterConfig.startColor = Colors::White;
    config.emitterConfig.endColor = Colors::Transparent;
    config.emitterConfig.blendMode = BlendMode::Additive;
    
    return config;
}

CustomEffectConfig EffectBuilder::Fire(const std::string& name) {
    CustomEffectConfig config = Particle(name);
    config.emitterConfig = ParticlePreset::Fire();
    return config;
}

CustomEffectConfig EffectBuilder::Smoke(const std::string& name) {
    CustomEffectConfig config = Particle(name);
    config.emitterConfig = ParticlePreset::Smoke();
    return config;
}

CustomEffectConfig EffectBuilder::Explosion(const std::string& name) {
    CustomEffectConfig config = Particle(name);
    config.emitterConfig = ParticlePreset::Explosion();
    config.duration = 2.0f;
    config.loop = false;
    return config;
}

CustomEffectConfig EffectBuilder::Magic(const std::string& name) {
    CustomEffectConfig config = Particle(name);
    config.emitterConfig = ParticlePreset::Magic();
    return config;
}

CustomEffectConfig EffectBuilder::Sparkle(const std::string& name) {
    CustomEffectConfig config = Particle(name);
    config.emitterConfig = ParticlePreset::Sparkle();
    return config;
}

CustomEffectConfig EffectBuilder::Bloom(const std::string& name) {
    CustomEffectConfig config;
    config.name = name;
    config.type = CustomEffectType::PostProcess;
    config.duration = -1.0f;
    config.loop = true;
    config.shaderParams["intensity"] = 1.5f;
    config.shaderParams["threshold"] = 0.8f;
    return config;
}

CustomEffectConfig EffectBuilder::Blur(const std::string& name) {
    CustomEffectConfig config;
    config.name = name;
    config.type = CustomEffectType::PostProcess;
    config.duration = -1.0f;
    config.loop = true;
    config.shaderParams["radius"] = 2.0f;
    return config;
}

CustomEffectConfig EffectBuilder::Vignette(const std::string& name) {
    CustomEffectConfig config;
    config.name = name;
    config.type = CustomEffectType::PostProcess;
    config.duration = -1.0f;
    config.loop = true;
    config.shaderParams["intensity"] = 0.5f;
    return config;
}

CustomEffectConfig EffectBuilder::ColorGrading(const std::string& name) {
    CustomEffectConfig config;
    config.name = name;
    config.type = CustomEffectType::PostProcess;
    config.duration = -1.0f;
    config.loop = true;
    config.shaderParams["brightness"] = 1.0f;
    config.shaderParams["contrast"] = 1.0f;
    config.shaderParams["saturation"] = 1.0f;
    return config;
}

} // namespace easy2d
