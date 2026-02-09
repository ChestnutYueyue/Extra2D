#pragma once

#include <extra2d/scene/node.h>

namespace extra2d {

class Widget : public Node {
public:
  Widget();
  ~Widget() override = default;

  void setSize(const Size &size);
  void setSize(float width, float height);
  Size getSize() const { return size_; }

  Rect getBoundingBox() const override;

private:
  Size size_ = Size::Zero();
};

} // namespace extra2d
