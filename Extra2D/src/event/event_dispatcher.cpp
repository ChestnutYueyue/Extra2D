#include <extra2d/event/event_dispatcher.h>
#include <extra2d/event/event_queue.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 初始化事件分发器，设置下一个监听器ID为1
 */
EventDispatcher::EventDispatcher() : nextId_(1) {}

/**
 * @brief 添加事件监听器
 *
 * 为指定的事件类型注册一个回调函数，返回监听器ID用于后续移除
 *
 * @param type 要监听的事件类型
 * @param callback 事件触发时调用的回调函数
 * @return 新注册监听器的唯一ID
 */
ListenerId EventDispatcher::addListener(EventType type,
                                        EventCallback callback) {
  ListenerId id = nextId_++;
  listeners_[type].push_back({id, type, callback});
  return id;
}

/**
 * @brief 移除指定的事件监听器
 *
 * 根据监听器ID移除对应的事件监听器
 *
 * @param id 要移除的监听器ID
 */
void EventDispatcher::removeListener(ListenerId id) {
  for (auto &[type, listeners] : listeners_) {
    auto it = std::remove_if(listeners.begin(), listeners.end(),
                             [id](const Listener &l) { return l.id == id; });
    if (it != listeners.end()) {
      listeners.erase(it, listeners.end());
      return;
    }
  }
}

/**
 * @brief 移除指定类型的所有监听器
 *
 * 移除某个事件类型下的所有已注册监听器
 *
 * @param type 要移除监听器的事件类型
 */
void EventDispatcher::removeAllListeners(EventType type) {
  listeners_.erase(type);
}

/**
 * @brief 移除所有监听器
 *
 * 清除事件分发器中所有已注册的监听器
 */
void EventDispatcher::removeAllListeners() { listeners_.clear(); }

/**
 * @brief 分发事件
 *
 * 将事件分发给对应类型的所有监听器，直到事件被标记为已处理或所有监听器执行完毕
 *
 * @param event 要分发的事件对象（可修改）
 */
void EventDispatcher::dispatch(Event &event) {
  auto it = listeners_.find(event.type);
  if (it != listeners_.end()) {
    for (auto &listener : it->second) {
      if (event.handled)
        break;
      listener.callback(event);
    }
  }
}

/**
 * @brief 分发事件（常量版本）
 *
 * 创建事件的副本并分发，适用于常量事件对象
 *
 * @param event 要分发的常量事件对象
 */
void EventDispatcher::dispatch(const Event &event) {
  Event mutableEvent = event;
  dispatch(mutableEvent);
}

/**
 * @brief 处理事件队列
 *
 * 从事件队列中依次取出所有事件并分发
 *
 * @param queue 要处理的事件队列
 */
void EventDispatcher::processQueue(EventQueue &queue) {
  Event event;
  while (queue.poll(event)) {
    dispatch(event);
  }
}

/**
 * @brief 获取指定类型监听器的数量
 *
 * 返回某个事件类型下已注册的监听器数量
 *
 * @param type 要查询的事件类型
 * @return 该类型下的监听器数量
 */
size_t EventDispatcher::getListenerCount(EventType type) const {
  auto it = listeners_.find(type);
  return (it != listeners_.end()) ? it->second.size() : 0;
}

/**
 * @brief 获取所有监听器的总数量
 *
 * 返回所有事件类型的监听器总数
 *
 * @return 所有监听器的总数量
 */
size_t EventDispatcher::getTotalListenerCount() const {
  size_t count = 0;
  for (const auto &[type, listeners] : listeners_) {
    count += listeners.size();
  }
  return count;
}

} // namespace extra2d
