#pragma once

#include <extra2d/config/app_config.h>
#include <extra2d/core/types.h>
#include <string>

namespace extra2d {

// ============================================================================
// 配置加载结果
// ============================================================================
struct ConfigLoadResult {
    bool success = false;
    std::string errorMessage;
    int errorLine = -1;
    std::string errorField;

    static ConfigLoadResult ok() { return ConfigLoadResult{true, "", -1, ""}; }
    static ConfigLoadResult error(const std::string& msg, int line = -1, const std::string& field = "") {
        return ConfigLoadResult{false, msg, line, field};
    }
    
    bool isOk() const { return success; }
    bool hasError() const { return !success; }
};

// ============================================================================
// 配置保存结果
// ============================================================================
struct ConfigSaveResult {
    bool success = false;
    std::string errorMessage;

    static ConfigSaveResult ok() { return ConfigSaveResult{true, ""}; }
    static ConfigSaveResult error(const std::string& msg) {
        return ConfigSaveResult{false, msg};
    }
    
    bool isOk() const { return success; }
    bool hasError() const { return !success; }
};

// ============================================================================
// 配置加载器抽象接口
// ============================================================================
class ConfigLoader {
public:
    virtual ~ConfigLoader() = default;

    /**
     * @brief 从文件加载配置
     * @param filepath 配置文件路径
     * @param config 输出的配置对象
     * @return 加载结果
     */
    virtual ConfigLoadResult load(const std::string& filepath, AppConfig& config) = 0;

    /**
     * @brief 保存配置到文件
     * @param filepath 配置文件路径
     * @param config 要保存的配置对象
     * @return 保存结果
     */
    virtual ConfigSaveResult save(const std::string& filepath, const AppConfig& config) = 0;

    /**
     * @brief 从字符串加载配置
     * @param content 配置内容字符串
     * @param config 输出的配置对象
     * @return 加载结果
     */
    virtual ConfigLoadResult loadFromString(const std::string& content, AppConfig& config) = 0;

    /**
     * @brief 将配置序列化为字符串
     * @param config 配置对象
     * @return 序列化后的字符串
     */
    virtual std::string saveToString(const AppConfig& config) = 0;

    /**
     * @brief 获取支持的文件扩展名
     * @return 文件扩展名（不含点号，如 "json"）
     */
    virtual const char* extension() const = 0;

    /**
     * @brief 检查是否支持指定文件
     * @param filepath 文件路径
     * @return 如果支持返回 true
     */
    virtual bool supportsFile(const std::string& filepath) const = 0;

    /**
     * @brief 克隆加载器实例
     * @return 新的加载器实例
     */
    virtual UniquePtr<ConfigLoader> clone() const = 0;
};

// ============================================================================
// JSON 配置加载器
// ============================================================================
class JsonConfigLoader : public ConfigLoader {
public:
    JsonConfigLoader() = default;
    ~JsonConfigLoader() override = default;

    ConfigLoadResult load(const std::string& filepath, AppConfig& config) override;
    ConfigSaveResult save(const std::string& filepath, const AppConfig& config) override;
    ConfigLoadResult loadFromString(const std::string& content, AppConfig& config) override;
    std::string saveToString(const AppConfig& config) override;
    const char* extension() const override { return "json"; }
    bool supportsFile(const std::string& filepath) const override;
    UniquePtr<ConfigLoader> clone() const override;

private:
    ConfigLoadResult parseWindowConfig(const void* jsonValue, WindowConfigData& window);
    ConfigLoadResult parseRenderConfig(const void* jsonValue, RenderConfigData& render);
    ConfigLoadResult parseAudioConfig(const void* jsonValue, AudioConfigData& audio);
    ConfigLoadResult parseDebugConfig(const void* jsonValue, DebugConfigData& debug);
    ConfigLoadResult parseInputConfig(const void* jsonValue, InputConfigData& input);
    ConfigLoadResult parseResourceConfig(const void* jsonValue, ResourceConfigData& resource);
    
    void serializeWindowConfig(void* jsonValue, const WindowConfigData& window);
    void serializeRenderConfig(void* jsonValue, const RenderConfigData& render);
    void serializeAudioConfig(void* jsonValue, const AudioConfigData& audio);
    void serializeDebugConfig(void* jsonValue, const DebugConfigData& debug);
    void serializeInputConfig(void* jsonValue, const InputConfigData& input);
    void serializeResourceConfig(void* jsonValue, const ResourceConfigData& resource);
};

// ============================================================================
// INI 配置加载器
// ============================================================================
class IniConfigLoader : public ConfigLoader {
public:
    IniConfigLoader() = default;
    ~IniConfigLoader() override = default;

    ConfigLoadResult load(const std::string& filepath, AppConfig& config) override;
    ConfigSaveResult save(const std::string& filepath, const AppConfig& config) override;
    ConfigLoadResult loadFromString(const std::string& content, AppConfig& config) override;
    std::string saveToString(const AppConfig& config) override;
    const char* extension() const override { return "ini"; }
    bool supportsFile(const std::string& filepath) const override;
    UniquePtr<ConfigLoader> clone() const override;

private:
    std::string sectionKey(const std::string& section, const std::string& key) const;
    ConfigLoadResult parseInt(const std::string& value, int& result, const std::string& fieldName);
    ConfigLoadResult parseFloat(const std::string& value, float& result, const std::string& fieldName);
    ConfigLoadResult parseBool(const std::string& value, bool& result, const std::string& fieldName);
};

// ============================================================================
// 配置加载器工厂
// ============================================================================
class ConfigLoaderFactory {
public:
    /**
     * @brief 根据文件扩展名创建加载器
     * @param extension 文件扩展名（不含点号）
     * @return 配置加载器实例，如果不支持返回 nullptr
     */
    static UniquePtr<ConfigLoader> create(const std::string& extension);

    /**
     * @brief 根据文件路径创建加载器
     * @param filepath 文件路径
     * @return 配置加载器实例，如果不支持返回 nullptr
     */
    static UniquePtr<ConfigLoader> createForFile(const std::string& filepath);

    /**
     * @brief 检查是否支持指定扩展名
     * @param extension 文件扩展名
     * @return 如果支持返回 true
     */
    static bool isExtensionSupported(const std::string& extension);

    /**
     * @brief 获取所有支持的扩展名
     * @return 支持的扩展名列表
     */
    static std::vector<std::string> getSupportedExtensions();
};

} 
