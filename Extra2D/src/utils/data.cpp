#include <extra2d/utils/data.h>
#include <extra2d/utils/logger.h>
#include <simpleini/SimpleIni.h>

// Switch 平台特定头文件
#ifdef __SWITCH__
#include <switch.h>
#include <switch/services/fs.h>
#endif

namespace extra2d {

class DataStore::Impl {
public:
  CSimpleIniA ini;
};

DataStore::DataStore() : impl_(makeUnique<Impl>()) {}

DataStore::~DataStore() {
  // 如果在事务中，尝试提交
  if (inTransaction_ && dirty_) {
    commit();
  }
  // 如果存档已挂载，卸载
  if (saveDataMounted_) {
    unmountSaveData(mountName_);
  }
}

// ============================================================================
// 文件操作
// ============================================================================

bool DataStore::load(const std::string &filename) {
  filename_ = filename;
  SI_Error rc = impl_->ini.LoadFile(filename.c_str());
  dirty_ = false;
  return rc >= 0;
}

bool DataStore::save(const std::string &filename) {
  // 如果在事务中，只标记为脏，不实际写入
  if (inTransaction_) {
    dirty_ = true;
    return true;
  }

  const std::string &targetFile = filename.empty() ? filename_ : filename;
  if (targetFile.empty()) {
    E2D_LOG_ERROR("DataStore::save: 没有指定文件名");
    return false;
  }

  return internalSave(targetFile);
}

bool DataStore::internalSave(const std::string &filename) {
  SI_Error rc = impl_->ini.SaveFile(filename.c_str());
  if (rc < 0) {
    E2D_LOG_ERROR("DataStore::save: 保存文件失败: {}", filename);
    return false;
  }
  dirty_ = false;
  return true;
}

// ============================================================================
// Switch 存档系统支持
// ============================================================================

#ifdef __SWITCH__

bool DataStore::mountSaveData(SaveDataType type, const UserId &userId,
                              const std::string &mountName) {
  // 如果已经挂载，先卸载
  if (saveDataMounted_) {
    unmountSaveData(mountName_);
  }

  Result rc = 0;
  AccountUid uid = {userId.uid[0], userId.uid[1]};

  // 如果没有提供用户ID，尝试获取当前用户
  if (type == SaveDataType::Account && !userId.isValid()) {
    UserId currentUid = getCurrentUserId();
    uid.uid[0] = currentUid.uid[0];
    uid.uid[1] = currentUid.uid[1];
    if (uid.uid[0] == 0 && uid.uid[1] == 0) {
      E2D_LOG_ERROR("DataStore::mountSaveData: 无法获取当前用户ID");
      return false;
    }
  }

  // 使用 fsdevMountSaveData 挂载
  // 注意：这里使用当前应用程序ID (0 表示当前应用)
  u64 applicationId = 0;
  rc = fsdevMountSaveData(mountName.c_str(), applicationId, uid);

  if (R_FAILED(rc)) {
    E2D_LOG_ERROR("DataStore::mountSaveData: 挂载失败: 0x{:X}", rc);
    return false;
  }

  mountName_ = mountName;
  saveDataMounted_ = true;
  defaultUserId_ = UserId{uid.uid[0], uid.uid[1]};

  E2D_LOG_INFO("DataStore::mountSaveData: 成功挂载存档: {}", mountName);
  return true;
}

void DataStore::unmountSaveData(const std::string &mountName) {
  if (!saveDataMounted_) {
    return;
  }

  // 先提交更改
  if (dirty_) {
    commitSaveData(mountName_);
  }

  fsdevUnmountDevice(mountName.c_str());
  saveDataMounted_ = false;
  mountName_.clear();

  E2D_LOG_INFO("DataStore::unmountSaveData: 已卸载存档");
}

bool DataStore::commitSaveData(const std::string &mountName) {
  if (!saveDataMounted_) {
    E2D_LOG_WARN("DataStore::commitSaveData: 存档未挂载");
    return false;
  }

  Result rc = fsdevCommitDevice(mountName.c_str());
  if (R_FAILED(rc)) {
    E2D_LOG_ERROR("DataStore::commitSaveData: 提交失败: 0x{:X}", rc);
    return false;
  }

  E2D_LOG_DEBUG("DataStore::commitSaveData: 提交成功");
  return true;
}

std::string DataStore::getSaveDataPath(const std::string &path) const {
  if (!saveDataMounted_) {
    return path;
  }
  return mountName_ + ":/" + path;
}

UserId DataStore::getCurrentUserId() {
  UserId result;

  Result rc = accountInitialize(AccountServiceType_Application);
  if (R_FAILED(rc)) {
    E2D_LOG_ERROR("DataStore::getCurrentUserId: accountInitialize 失败: 0x{:X}",
                  rc);
    return result;
  }

  AccountUid uid;
  rc = accountGetPreselectedUser(&uid);
  accountExit();

  if (R_SUCCEEDED(rc)) {
    result.uid[0] = uid.uid[0];
    result.uid[1] = uid.uid[1];
    E2D_LOG_DEBUG("DataStore::getCurrentUserId: 获取成功: 0x{:X}{:X}",
                  result.uid[1], result.uid[0]);
  } else {
    E2D_LOG_ERROR("DataStore::getCurrentUserId: 获取失败: 0x{:X}", rc);
  }

  return result;
}

#else

// 非 Switch 平台的存根实现

bool DataStore::mountSaveData(SaveDataType type, const UserId &userId,
                              const std::string &mountName) {
  (void)type;
  (void)userId;
  (void)mountName;
  E2D_LOG_WARN("DataStore::mountSaveData: 非 Switch 平台，存档功能不可用");
  return false;
}

void DataStore::unmountSaveData(const std::string &mountName) {
  (void)mountName;
  saveDataMounted_ = false;
}

bool DataStore::commitSaveData(const std::string &mountName) {
  (void)mountName;
  return true;
}

std::string DataStore::getSaveDataPath(const std::string &path) const {
  return path;
}

UserId DataStore::getCurrentUserId() {
  return UserId();
}

#endif

// ============================================================================
// 数据读写
// ============================================================================

std::string DataStore::getString(const std::string &section,
                                 const std::string &key,
                                 const std::string &defaultValue) {
  const char *value =
      impl_->ini.GetValue(section.c_str(), key.c_str(), defaultValue.c_str());
  return value ? value : defaultValue;
}

int DataStore::getInt(const std::string &section, const std::string &key,
                      int defaultValue) {
  return static_cast<int>(
      impl_->ini.GetLongValue(section.c_str(), key.c_str(), defaultValue));
}

float DataStore::getFloat(const std::string &section, const std::string &key,
                          float defaultValue) {
  const char *value =
      impl_->ini.GetValue(section.c_str(), key.c_str(), nullptr);
  if (value) {
    try {
      return std::stof(value);
    } catch (...) {
      return defaultValue;
    }
  }
  return defaultValue;
}

bool DataStore::getBool(const std::string &section, const std::string &key,
                        bool defaultValue) {
  return impl_->ini.GetBoolValue(section.c_str(), key.c_str(), defaultValue);
}

void DataStore::setString(const std::string &section, const std::string &key,
                          const std::string &value) {
  impl_->ini.SetValue(section.c_str(), key.c_str(), value.c_str());
  dirty_ = true;

  // 不在事务中时自动保存
  if (!inTransaction_ && !filename_.empty()) {
    save("");
  }
}

void DataStore::setInt(const std::string &section, const std::string &key,
                       int value) {
  impl_->ini.SetLongValue(section.c_str(), key.c_str(), value);
  dirty_ = true;

  if (!inTransaction_ && !filename_.empty()) {
    save("");
  }
}

void DataStore::setFloat(const std::string &section, const std::string &key,
                         float value) {
  impl_->ini.SetValue(section.c_str(), key.c_str(),
                      std::to_string(value).c_str());
  dirty_ = true;

  if (!inTransaction_ && !filename_.empty()) {
    save("");
  }
}

void DataStore::setBool(const std::string &section, const std::string &key,
                        bool value) {
  impl_->ini.SetBoolValue(section.c_str(), key.c_str(), value);
  dirty_ = true;

  if (!inTransaction_ && !filename_.empty()) {
    save("");
  }
}

void DataStore::removeKey(const std::string &section, const std::string &key) {
  impl_->ini.Delete(section.c_str(), key.c_str());
  dirty_ = true;

  if (!inTransaction_ && !filename_.empty()) {
    save("");
  }
}

void DataStore::removeSection(const std::string &section) {
  impl_->ini.Delete(section.c_str(), nullptr);
  dirty_ = true;

  if (!inTransaction_ && !filename_.empty()) {
    save("");
  }
}

bool DataStore::hasKey(const std::string &section, const std::string &key) {
  return impl_->ini.GetValue(section.c_str(), key.c_str(), nullptr) != nullptr;
}

bool DataStore::hasSection(const std::string &section) {
  return impl_->ini.GetSection(section.c_str()) != nullptr;
}

void DataStore::clear() {
  impl_->ini.Reset();
  dirty_ = true;

  if (!inTransaction_ && !filename_.empty()) {
    save("");
  }
}

// ============================================================================
// 事务支持
// ============================================================================

void DataStore::beginTransaction() {
  if (inTransaction_) {
    E2D_LOG_WARN("DataStore::beginTransaction: 已经处于事务中");
    return;
  }

  inTransaction_ = true;
  dirty_ = false;

  E2D_LOG_DEBUG("DataStore::beginTransaction: 事务开始");
}

bool DataStore::commit() {
  if (!inTransaction_) {
    E2D_LOG_WARN("DataStore::commit: 不在事务中");
    return false;
  }

  // 如果有文件名，写入文件
  bool result = true;
  if (!filename_.empty() && dirty_) {
    result = internalSave(filename_);

    // 如果挂载了存档，提交更改
    if (result && saveDataMounted_) {
      result = commitSaveData(mountName_);
    }
  }

  inTransaction_ = false;

  E2D_LOG_DEBUG("DataStore::commit: 事务提交 {}", result ? "成功" : "失败");
  return result;
}

void DataStore::rollback() {
  if (!inTransaction_) {
    E2D_LOG_WARN("DataStore::rollback: 不在事务中");
    return;
  }

  // 重新加载文件来恢复数据
  if (!filename_.empty()) {
    impl_->ini.Reset();
    SI_Error rc = impl_->ini.LoadFile(filename_.c_str());
    if (rc < 0) {
      E2D_LOG_ERROR("DataStore::rollback: 重新加载文件失败: {}", filename_);
    }
  } else {
    // 如果没有文件名，清空数据
    impl_->ini.Reset();
  }

  inTransaction_ = false;
  dirty_ = false;

  E2D_LOG_DEBUG("DataStore::rollback: 事务已回滚");
}

// ============================================================================
// 工具方法
// ============================================================================

std::vector<std::string> DataStore::getAllSections() const {
  std::vector<std::string> sections;
  CSimpleIniA::TNamesDepend sectionList;
  impl_->ini.GetAllSections(sectionList);

  for (const auto &section : sectionList) {
    sections.emplace_back(section.pItem);
  }

  return sections;
}

std::vector<std::string> DataStore::getAllKeys(const std::string &section) const {
  std::vector<std::string> keys;
  CSimpleIniA::TNamesDepend keyList;
  impl_->ini.GetAllKeys(section.c_str(), keyList);

  for (const auto &key : keyList) {
    keys.emplace_back(key.pItem);
  }

  return keys;
}

bool DataStore::loadFromSave(const std::string &path) {
  if (!saveDataMounted_) {
    E2D_LOG_ERROR("DataStore::loadFromSave: 存档未挂载");
    return false;
  }

  std::string fullPath = getSaveDataPath(path);
  return load(fullPath);
}

bool DataStore::saveToSave(const std::string &path) {
  if (!saveDataMounted_) {
    E2D_LOG_ERROR("DataStore::saveToSave: 存档未挂载");
    return false;
  }

  std::string fullPath = getSaveDataPath(path);
  bool result = save(fullPath);

  // 自动提交
  if (result) {
    result = commitSaveData(mountName_);
  }

  return result;
}

} // namespace extra2d
