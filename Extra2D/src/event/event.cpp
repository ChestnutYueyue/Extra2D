#include <extra2d/event/event.h>

namespace extra2d {

/**
 * @brief 创建窗口大小改变事件
 *
 * 创建一个表示窗口尺寸变化的Event对象
 *
 * @param width 新的窗口宽度（像素）
 * @param height 新的窗口高度（像素）
 * @return 包含窗口大小改变信息的Event对象
 */
Event Event::createWindowResize(int width, int height) {
  Event event;
  event.type = EventType::WindowResize;
  event.data = WindowResizeEvent{width, height};
  return event;
}

/**
 * @brief 创建窗口关闭事件
 *
 * 创建一个表示窗口请求关闭的Event对象
 *
 * @return 表示窗口关闭请求的Event对象
 */
Event Event::createWindowClose() {
  Event event;
  event.type = EventType::WindowClose;
  return event;
}

/**
 * @brief 创建键盘按键按下事件
 *
 * 创建一个表示键盘按键被按下的Event对象
 *
 * @param keyCode 按键码
 * @param scancode 扫描码
 * @param mods 修饰键状态（如Shift、Ctrl等）
 * @return 包含按键按下信息的Event对象
 */
Event Event::createKeyPress(int keyCode, int scancode, int mods) {
  Event event;
  event.type = EventType::KeyPressed;
  event.data = KeyEvent{keyCode, scancode, mods};
  return event;
}

/**
 * @brief 创建键盘按键释放事件
 *
 * 创建一个表示键盘按键被释放的Event对象
 *
 * @param keyCode 按键码
 * @param scancode 扫描码
 * @param mods 修饰键状态（如Shift、Ctrl等）
 * @return 包含按键释放信息的Event对象
 */
Event Event::createKeyRelease(int keyCode, int scancode, int mods) {
  Event event;
  event.type = EventType::KeyReleased;
  event.data = KeyEvent{keyCode, scancode, mods};
  return event;
}

/**
 * @brief 创建鼠标按钮按下事件
 *
 * 创建一个表示鼠标按钮被按下的Event对象
 *
 * @param button 鼠标按钮编号
 * @param mods 修饰键状态
 * @param pos 鼠标按下时的位置坐标
 * @return 包含鼠标按钮按下信息的Event对象
 */
Event Event::createMouseButtonPress(int button, int mods, const Vec2 &pos) {
  Event event;
  event.type = EventType::MouseButtonPressed;
  event.data = MouseButtonEvent{button, mods, pos};
  return event;
}

/**
 * @brief 创建鼠标按钮释放事件
 *
 * 创建一个表示鼠标按钮被释放的Event对象
 *
 * @param button 鼠标按钮编号
 * @param mods 修饰键状态
 * @param pos 鼠标释放时的位置坐标
 * @return 包含鼠标按钮释放信息的Event对象
 */
Event Event::createMouseButtonRelease(int button, int mods, const Vec2 &pos) {
  Event event;
  event.type = EventType::MouseButtonReleased;
  event.data = MouseButtonEvent{button, mods, pos};
  return event;
}

/**
 * @brief 创建鼠标移动事件
 *
 * 创建一个表示鼠标移动的Event对象
 *
 * @param pos 鼠标当前位置坐标
 * @param delta 鼠标移动的位移量
 * @return 包含鼠标移动信息的Event对象
 */
Event Event::createMouseMove(const Vec2 &pos, const Vec2 &delta) {
  Event event;
  event.type = EventType::MouseMoved;
  event.data = MouseMoveEvent{pos, delta};
  return event;
}

/**
 * @brief 创建鼠标滚轮滚动事件
 *
 * 创建一个表示鼠标滚轮滚动的Event对象
 *
 * @param offset 滚轮滚动的偏移量
 * @param pos 滚动时鼠标的位置坐标
 * @return 包含鼠标滚轮滚动信息的Event对象
 */
Event Event::createMouseScroll(const Vec2 &offset, const Vec2 &pos) {
  Event event;
  event.type = EventType::MouseScrolled;
  event.data = MouseScrollEvent{offset, pos};
  return event;
}

} // namespace extra2d
