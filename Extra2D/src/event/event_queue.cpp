#include <extra2d/event/event_queue.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 构造一个空的事件队列对象
 */
EventQueue::EventQueue() = default;

/**
 * @brief 将事件压入队列（左值引用版本）
 *
 * 将事件以拷贝方式添加到队列末尾，线程安全
 *
 * @param event 要添加的事件对象
 */
void EventQueue::push(const Event &event) {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(event);
}

/**
 * @brief 将事件压入队列（右值引用版本）
 *
 * 将事件以移动方式添加到队列末尾，线程安全
 *
 * @param event 要添加的事件对象（右值引用）
 */
void EventQueue::push(Event &&event) {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(std::move(event));
}

/**
 * @brief 从队列中取出事件
 *
 * 从队列头部取出一个事件，如果队列不为空则移除该事件，线程安全
 *
 * @param event 输出参数，用于存储取出的事件
 * @return 如果成功取出事件返回true，队列为空返回false
 */
bool EventQueue::poll(Event &event) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) {
    return false;
  }
  event = queue_.front();
  queue_.pop();
  return true;
}

/**
 * @brief 查看队列头部事件
 *
 * 获取队列头部的事件但不移除，线程安全
 *
 * @param event 输出参数，用于存储查看到的事件
 * @return 如果队列不为空返回true，队列为空返回false
 */
bool EventQueue::peek(Event &event) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) {
    return false;
  }
  event = queue_.front();
  return true;
}

/**
 * @brief 清空队列
 *
 * 移除队列中的所有事件，线程安全
 */
void EventQueue::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (!queue_.empty()) {
    queue_.pop();
  }
}

/**
 * @brief 检查队列是否为空
 *
 * 线程安全地检查队列中是否有事件
 *
 * @return 如果队列为空返回true，否则返回false
 */
bool EventQueue::empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}

/**
 * @brief 获取队列中的事件数量
 *
 * 线程安全地获取队列中当前存储的事件数量
 *
 * @return 队列中的事件数量
 */
size_t EventQueue::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

} // namespace extra2d
