#include <algorithm>
#include <extra2d/utils/timer.h>

namespace extra2d {

uint32 Timer::nextId_ = 1;

/**
 * @brief 构造函数，创建定时器
 * @param interval 定时间隔（秒）
 * @param repeat 是否重复触发
 * @param callback 定时器回调函数
 */
Timer::Timer(float interval, bool repeat, Callback callback)
    : interval_(interval), elapsed_(0.0f), repeat_(repeat), paused_(false),
      valid_(true), callback_(std::move(callback)) {
  id_ = nextId_++;
}

/**
 * @brief 更新定时器状态
 * @param deltaTime 帧间隔时间（秒）
 * @return 如果定时器触发则返回true，否则返回false
 */
bool Timer::update(float deltaTime) {
  if (!valid_ || paused_) {
    return false;
  }

  elapsed_ += deltaTime;

  if (elapsed_ >= interval_) {
    if (callback_) {
      callback_();
    }

    if (repeat_) {
      elapsed_ = 0.0f;
      return true;
    } else {
      valid_ = false;
      return true;
    }
  }

  return false;
}

/**
 * @brief 重置定时器
 *
 * 重置已过时间，恢复定时器为有效且非暂停状态
 */
void Timer::reset() {
  elapsed_ = 0.0f;
  valid_ = true;
  paused_ = false;
}

/**
 * @brief 暂停定时器
 */
void Timer::pause() { paused_ = true; }

/**
 * @brief 恢复定时器
 */
void Timer::resume() { paused_ = false; }

/**
 * @brief 取消定时器
 *
 * 将定时器标记为无效状态
 */
void Timer::cancel() { valid_ = false; }

/**
 * @brief 获取剩余时间
 * @return 剩余时间（秒），如果定时器无效或已暂停则返回0
 */
float Timer::getRemaining() const {
  if (!valid_ || paused_) {
    return 0.0f;
  }
  return std::max(0.0f, interval_ - elapsed_);
}

// ============================================================================
// TimerManager 实现
// ============================================================================

/**
 * @brief 添加单次定时器
 * @param delay 延迟时间（秒）
 * @param callback 定时器回调函数
 * @return 定时器ID
 */
uint32 TimerManager::addTimer(float delay, Timer::Callback callback) {
  auto timer = std::make_unique<Timer>(delay, false, std::move(callback));
  uint32 id = timer->getId();
  timers_.emplace(id, std::move(timer));
  return id;
}

/**
 * @brief 添加重复定时器
 * @param interval 触发间隔（秒）
 * @param callback 定时器回调函数
 * @return 定时器ID
 */
uint32 TimerManager::addRepeatingTimer(float interval,
                                       Timer::Callback callback) {
  auto timer = std::make_unique<Timer>(interval, true, std::move(callback));
  uint32 id = timer->getId();
  timers_.emplace(id, std::move(timer));
  return id;
}

/**
 * @brief 取消指定定时器
 * @param timerId 定时器ID
 */
void TimerManager::cancelTimer(uint32 timerId) {
  auto it = timers_.find(timerId);
  if (it != timers_.end()) {
    it->second->cancel();
    timersToRemove_.push_back(timerId);
  }
}

/**
 * @brief 暂停指定定时器
 * @param timerId 定时器ID
 */
void TimerManager::pauseTimer(uint32 timerId) {
  auto it = timers_.find(timerId);
  if (it != timers_.end()) {
    it->second->pause();
  }
}

/**
 * @brief 恢复指定定时器
 * @param timerId 定时器ID
 */
void TimerManager::resumeTimer(uint32 timerId) {
  auto it = timers_.find(timerId);
  if (it != timers_.end()) {
    it->second->resume();
  }
}

/**
 * @brief 更新所有定时器
 * @param deltaTime 帧间隔时间（秒）
 *
 * 更新所有定时器状态，并移除已失效的定时器
 */
void TimerManager::update(float deltaTime) {
  timersToRemove_.clear();

  for (auto &[id, timer] : timers_) {
    timer->update(deltaTime);
    if (!timer->isValid()) {
      timersToRemove_.push_back(id);
    }
  }

  for (uint32 id : timersToRemove_) {
    timers_.erase(id);
  }
}

/**
 * @brief 清除所有定时器
 */
void TimerManager::clear() {
  timers_.clear();
  timersToRemove_.clear();
}

} // namespace extra2d
