# DataStore 数据持久化系统

## 概述

`DataStore` 是 Easy2D 引擎的数据持久化类，提供 INI 文件格式的数据存储功能，并支持 Nintendo Switch 平台的官方存档系统。

## 特性

- ✅ INI 文件格式读写
- ✅ Nintendo Switch 官方存档系统支持
- ✅ 多用户存档隔离
- ✅ 事务支持（批量操作 + 回滚）
- ✅ 自动保存和脏数据检测
- ✅ 跨平台兼容（Switch/PC）

---

## 快速开始

### 基本使用

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

// 创建 DataStore 实例
DataStore data;

// 加载数据文件
data.load("config.ini");

// 读取数据
int level = data.getInt("Player", "Level", 1);
std::string name = data.getString("Player", "Name", "Unknown");

// 写入数据
data.setInt("Player", "Level", 10);
data.setString("Player", "Name", "Hero");

// 保存到文件
data.save("config.ini");
```

### Switch 存档系统使用

```cpp
DataStore saveData;

// 挂载存档（自动获取当前用户）
if (saveData.mountSaveData(SaveDataType::Account)) {
    // 从存档加载
    saveData.loadFromSave("savegame.ini");
    
    // 修改数据
    saveData.setInt("Progress", "Level", 5);
    saveData.setInt("Progress", "Score", 1000);
    
    // 保存到存档（自动提交）
    saveData.saveToSave("savegame.ini");
    
    // 卸载存档
    saveData.unmountSaveData();
}
```

---

## API 参考

### 枚举类型

#### SaveDataType

存档数据类型枚举：

| 值 | 说明 |
|---|---|
| `SaveDataType::Account` | 用户存档，与特定用户账户关联 |
| `SaveDataType::Common` | 公共存档，所有用户共享 |
| `SaveDataType::Cache` | 缓存数据，可被系统清理 |
| `SaveDataType::Device` | 设备存档，与设备绑定 |
| `SaveDataType::Temporary` | 临时数据，应用退出后清理 |

#### UserId

用户ID结构体：

```cpp
struct UserId {
    uint64_t uid[2];  // 用户唯一标识
    
    bool isValid() const;  // 检查ID是否有效
};
```

---

### 构造函数

```cpp
DataStore();
~DataStore();
```

**说明：**
- 析构时会自动提交未保存的事务
- 如果存档已挂载，会自动卸载

---

### 文件操作

#### load

```cpp
bool load(const std::string &filename);
```

加载 INI 文件。

**参数：**
- `filename` - 文件路径

**返回值：**
- `true` - 加载成功
- `false` - 加载失败

**示例：**
```cpp
if (!data.load("config.ini")) {
    E2D_LOG_WARN("配置文件不存在，将创建新文件");
}
```

---

#### save

```cpp
bool save(const std::string &filename = "");
```

保存到 INI 文件。

**参数：**
- `filename` - 文件路径，为空则使用上次加载的文件名

**返回值：**
- `true` - 保存成功
- `false` - 保存失败

**说明：**
- 如果在事务中，只标记为脏数据，不实际写入
- 事务外自动保存（当设置数据时）

---

### Switch 存档系统

#### mountSaveData

```cpp
bool mountSaveData(
    SaveDataType type = SaveDataType::Account,
    const UserId &userId = UserId(),
    const std::string &mountName = "save"
);
```

挂载 Switch 存档数据。

**参数：**
- `type` - 存档类型
- `userId` - 用户ID（Account 类型需要，为空则自动获取当前用户）
- `mountName` - 挂载点名称

**返回值：**
- `true` - 挂载成功
- `false` - 挂载失败

**示例：**
```cpp
// 挂载当前用户的存档
if (data.mountSaveData(SaveDataType::Account)) {
    // 存档已挂载，路径为 "save:/"
}

// 挂载公共存档
if (data.mountSaveData(SaveDataType::Common, UserId(), "common")) {
    // 公共存档已挂载，路径为 "common:/"
}
```

---

#### unmountSaveData

```cpp
void unmountSaveData(const std::string &mountName = "save");
```

卸载存档挂载。

**参数：**
- `mountName` - 挂载点名称

**说明：**
- 卸载前会自动提交未保存的更改

---

#### commitSaveData

```cpp
bool commitSaveData(const std::string &mountName = "save");
```

提交存档更改。

**⚠️ 重要：** 修改存档数据后必须调用此方法，否则更改不会持久化！

**参数：**
- `mountName` - 挂载点名称

**返回值：**
- `true` - 提交成功
- `false` - 提交失败

**示例：**
```cpp
data.setInt("Game", "Score", 100);
data.save();
data.commitSaveData();  // 必须提交！
```

---

#### isSaveDataMounted

```cpp
bool isSaveDataMounted() const;
```

检查存档是否已挂载。

**返回值：**
- `true` - 已挂载
- `false` - 未挂载

---

#### getSaveDataPath

```cpp
std::string getSaveDataPath(const std::string &path = "") const;
```

获取挂载点路径。

**参数：**
- `path` - 子路径

**返回值：**
- 完整路径（如 "save:/data/config.ini"）

**示例：**
```cpp
std::string fullPath = data.getSaveDataPath("config/settings.ini");
// 返回: "save:/config/settings.ini"
```

---

### 用户账户管理

#### getCurrentUserId

```cpp
static UserId getCurrentUserId();
```

获取当前预选用户ID。

**返回值：**
- 用户ID（无效时返回空ID）

**示例：**
```cpp
UserId user = DataStore::getCurrentUserId();
if (user.isValid()) {
    E2D_LOG_INFO("当前用户: 0x{:X}{:X}", user.uid[1], user.uid[0]);
}
```

---

#### setDefaultUserId / getDefaultUserId

```cpp
void setDefaultUserId(const UserId &userId);
UserId getDefaultUserId() const;
```

设置/获取默认用户ID。

**说明：**
- 用于多用户场景下的默认用户选择

---

### 数据读写

#### getString

```cpp
std::string getString(
    const std::string &section,
    const std::string &key,
    const std::string &defaultValue = ""
);
```

获取字符串值。

**参数：**
- `section` - 节名称
- `key` - 键名称
- `defaultValue` - 默认值（键不存在时返回）

**返回值：**
- 字符串值

---

#### getInt

```cpp
int getInt(
    const std::string &section,
    const std::string &key,
    int defaultValue = 0
);
```

获取整数值。

---

#### getFloat

```cpp
float getFloat(
    const std::string &section,
    const std::string &key,
    float defaultValue = 0.0f
);
```

获取浮点数值。

---

#### getBool

```cpp
bool getBool(
    const std::string &section,
    const std::string &key,
    bool defaultValue = false
);
```

获取布尔值。

**说明：**
- 支持 "true"/"false"、"yes"/"no"、"1"/"0" 等格式

---

#### setString

```cpp
void setString(
    const std::string &section,
    const std::string &key,
    const std::string &value
);
```

设置字符串值。

**说明：**
- 自动标记为脏数据
- 事务外自动保存

---

#### setInt

```cpp
void setInt(
    const std::string &section,
    const std::string &key,
    int value
);
```

设置整数值。

---

#### setFloat

```cpp
void setFloat(
    const std::string &section,
    const std::string &key,
    float value
);
```

设置浮点数值。

---

#### setBool

```cpp
void setBool(
    const std::string &section,
    const std::string &key,
    bool value
);
```

设置布尔值。

---

#### removeKey

```cpp
void removeKey(const std::string &section, const std::string &key);
```

删除键。

---

#### removeSection

```cpp
void removeSection(const std::string &section);
```

删除整个 section。

---

#### hasKey

```cpp
bool hasKey(const std::string &section, const std::string &key);
```

检查键是否存在。

---

#### hasSection

```cpp
bool hasSection(const std::string &section);
```

检查 section 是否存在。

---

#### clear

```cpp
void clear();
```

清除所有数据。

---

### 事务支持

#### beginTransaction

```cpp
void beginTransaction();
```

开始事务。

**说明：**
- 事务中的修改不会立即写入文件
- 支持批量操作，提高性能
- 事务可以回滚

**示例：**
```cpp
data.beginTransaction();

// 批量修改
data.setInt("Player", "Level", 10);
data.setInt("Player", "Exp", 1000);
data.setString("Player", "Title", "Knight");

// 提交事务
data.commit();
```

---

#### commit

```cpp
bool commit();
```

提交事务。

**返回值：**
- `true` - 提交成功
- `false` - 提交失败

**说明：**
- 写入文件并提交存档（如果已挂载）
- 事务结束后自动保存

---

#### rollback

```cpp
void rollback();
```

回滚事务。

**说明：**
- 放弃事务中的所有修改
- 重新加载文件恢复数据

**示例：**
```cpp
data.beginTransaction();
data.setInt("Test", "Value", 999);

// 放弃修改
data.rollback();

// Value 恢复为原来的值
```

---

#### isInTransaction

```cpp
bool isInTransaction() const;
```

检查是否在事务中。

---

### 工具方法

#### getAllSections

```cpp
std::vector<std::string> getAllSections() const;
```

获取所有 section 名称。

**返回值：**
- section 名称列表

---

#### getAllKeys

```cpp
std::vector<std::string> getAllKeys(const std::string &section) const;
```

获取指定 section 的所有 key。

**参数：**
- `section` - section 名称

**返回值：**
- key 名称列表

---

#### loadFromSave

```cpp
bool loadFromSave(const std::string &path);
```

从存档加载（自动处理挂载路径）。

**参数：**
- `path` - 存档内文件路径

**返回值：**
- `true` - 加载成功
- `false` - 加载失败（存档未挂载或文件不存在）

**示例：**
```cpp
if (data.loadFromSave("savegame.ini")) {
    // 加载成功
}
```

---

#### saveToSave

```cpp
bool saveToSave(const std::string &path);
```

保存到存档（自动处理挂载路径和提交）。

**参数：**
- `path` - 存档内文件路径

**返回值：**
- `true` - 保存成功
- `false` - 保存失败

**说明：**
- 自动调用 `commitSaveData()` 提交更改

---

## 完整示例

### 游戏存档管理

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

class SaveManager {
private:
    DataStore saveData_;
    bool mounted_ = false;

public:
    bool initialize() {
        // 挂载用户存档
        mounted_ = saveData_.mountSaveData(SaveDataType::Account);
        if (!mounted_) {
            E2D_LOG_ERROR("存档挂载失败");
            return false;
        }
        
        // 加载存档
        if (!saveData_.loadFromSave("game_save.ini")) {
            E2D_LOG_INFO("存档不存在，创建新存档");
            createNewSave();
        }
        
        return true;
    }
    
    void shutdown() {
        if (mounted_) {
            saveData_.unmountSaveData();
            mounted_ = false;
        }
    }
    
    void createNewSave() {
        saveData_.beginTransaction();
        
        saveData_.setString("Player", "Name", "Player1");
        saveData_.setInt("Player", "Level", 1);
        saveData_.setInt("Player", "Exp", 0);
        saveData_.setInt("Progress", "Chapter", 1);
        saveData_.setInt("Settings", "Difficulty", 1);
        
        saveData_.commit();
        saveData_.saveToSave("game_save.ini");
    }
    
    void saveGame(const PlayerData &player) {
        saveData_.beginTransaction();
        
        saveData_.setInt("Player", "Level", player.level);
        saveData_.setInt("Player", "Exp", player.exp);
        saveData_.setInt("Player", "Health", player.health);
        saveData_.setInt("Player", "Mana", player.mana);
        saveData_.setInt("Progress", "Chapter", player.chapter);
        saveData_.setString("Progress", "Checkpoint", player.checkpoint);
        
        if (saveData_.commit()) {
            saveData_.saveToSave("game_save.ini");
            E2D_LOG_INFO("游戏已保存");
        } else {
            E2D_LOG_ERROR("保存失败");
        }
    }
    
    void loadGame(PlayerData &player) {
        player.level = saveData_.getInt("Player", "Level", 1);
        player.exp = saveData_.getInt("Player", "Exp", 0);
        player.health = saveData_.getInt("Player", "Health", 100);
        player.mana = saveData_.getInt("Player", "Mana", 50);
        player.chapter = saveData_.getInt("Progress", "Chapter", 1);
        player.checkpoint = saveData_.getString("Progress", "Checkpoint", "start");
    }
    
    bool hasSaveFile() {
        return saveData_.hasSection("Player");
    }
};
```

### 设置管理

```cpp
class SettingsManager {
private:
    DataStore settings_;
    
public:
    void load() {
        // PC 平台使用普通文件
        #ifndef __SWITCH__
        settings_.load("settings.ini");
        #else
        // Switch 平台使用存档
        if (settings_.mountSaveData(SaveDataType::Common)) {
            settings_.loadFromSave("settings.ini");
        }
        #endif
    }
    
    void save() {
        #ifndef __SWITCH__
        settings_.save("settings.ini");
        #else
        settings_.saveToSave("settings.ini");
        settings_.unmountSaveData();
        #endif
    }
    
    int getVolume() {
        return settings_.getInt("Audio", "Volume", 80);
    }
    
    void setVolume(int volume) {
        settings_.setInt("Audio", "Volume", volume);
    }
    
    bool isFullscreen() {
        return settings_.getBool("Video", "Fullscreen", false);
    }
    
    void setFullscreen(bool fullscreen) {
        settings_.setBool("Video", "Fullscreen", fullscreen);
    }
};
```

---

## 注意事项

### Switch 平台

1. **必须提交存档**：修改存档数据后，必须调用 `commitSaveData()` 或 `saveToSave()`，否则更改不会持久化

2. **用户ID管理**：
   - `Account` 类型存档需要有效的用户ID
   - 可以使用 `getCurrentUserId()` 自动获取当前用户
   - 公共存档使用 `Common` 类型，不需要用户ID

3. **存档挂载**：
   - 每个挂载点需要唯一的名称
   - 应用退出时会自动卸载，但建议显式调用 `unmountSaveData()`

4. **存档空间**：
   - 注意存档空间限制
   - 大数据（如截图）建议使用 `Cache` 类型

### 通用

1. **事务使用**：
   - 批量修改时使用事务提高性能
   - 事务中的错误可以通过 `rollback()` 恢复

2. **自动保存**：
   - 事务外每次 `set` 操作都会触发自动保存
   - 频繁修改建议使用事务批量处理

3. **文件格式**：
   - 使用标准 INI 格式
   - 支持注释（以 `;` 或 `#` 开头）
   - Section 和 Key 不区分大小写

---

## 相关文件

- [data.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/utils/data.h) - 头文件
- [data.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/utils/data.cpp) - 实现文件
