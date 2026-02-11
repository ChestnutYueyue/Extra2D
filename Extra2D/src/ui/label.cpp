#include <extra2d/ui/label.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>

namespace extra2d {

Label::Label() {
    // 标签默认锚点为左上角
    setAnchor(0.0f, 0.0f);
}

Label::Label(const std::string &text) : text_(text) {
    setAnchor(0.0f, 0.0f);
    sizeDirty_ = true;
}

Ptr<Label> Label::create() {
    return makePtr<Label>();
}

Ptr<Label> Label::create(const std::string &text) {
    return makePtr<Label>(text);
}

Ptr<Label> Label::create(const std::string &text, Ptr<FontAtlas> font) {
    auto label = makePtr<Label>(text);
    label->setFont(font);
    return label;
}

// ------------------------------------------------------------------------
// 链式调用构建器方法
// ------------------------------------------------------------------------
Label *Label::withPosition(float x, float y) {
    setPosition(x, y);
    return this;
}

Label *Label::withPosition(const Vec2 &pos) {
    setPosition(pos);
    return this;
}

Label *Label::withAnchor(float x, float y) {
    setAnchor(x, y);
    return this;
}

Label *Label::withAnchor(const Vec2 &anchor) {
    setAnchor(anchor);
    return this;
}

Label *Label::withText(const std::string &text) {
    setText(text);
    return this;
}

Label *Label::withFont(Ptr<FontAtlas> font) {
    setFont(font);
    return this;
}

Label *Label::withTextColor(const Color &color) {
    setTextColor(color);
    return this;
}

Label *Label::withFontSize(int size) {
    setFontSize(size);
    return this;
}

// ------------------------------------------------------------------------
// 坐标空间设置（链式调用）
// ------------------------------------------------------------------------
Label *Label::withCoordinateSpace(CoordinateSpace space) {
    setCoordinateSpace(space);
    return this;
}

Label *Label::withScreenPosition(float x, float y) {
    setScreenPosition(x, y);
    return this;
}

Label *Label::withScreenPosition(const Vec2 &pos) {
    setScreenPosition(pos);
    return this;
}

Label *Label::withCameraOffset(float x, float y) {
    setCameraOffset(x, y);
    return this;
}

Label *Label::withCameraOffset(const Vec2 &offset) {
    setCameraOffset(offset);
    return this;
}

void Label::setText(const std::string &text) {
    text_ = text;
    sizeDirty_ = true;
    updateSpatialIndex();
}

void Label::setFont(Ptr<FontAtlas> font) {
    font_ = font;
    sizeDirty_ = true;
    updateSpatialIndex();
}

void Label::setTextColor(const Color &color) {
    textColor_ = color;
}

void Label::setFontSize(int size) {
    fontSize_ = size;
    sizeDirty_ = true;
    updateSpatialIndex();
}

void Label::setHorizontalAlign(HorizontalAlign align) {
    hAlign_ = align;
}

void Label::setVerticalAlign(VerticalAlign align) {
    vAlign_ = align;
}

void Label::setShadowEnabled(bool enabled) {
    shadowEnabled_ = enabled;
}

void Label::setShadowColor(const Color &color) {
    shadowColor_ = color;
}

void Label::setShadowOffset(const Vec2 &offset) {
    shadowOffset_ = offset;
}

void Label::setOutlineEnabled(bool enabled) {
    outlineEnabled_ = enabled;
}

void Label::setOutlineColor(const Color &color) {
    outlineColor_ = color;
}

void Label::setOutlineWidth(float width) {
    outlineWidth_ = width;
}

void Label::setMultiLine(bool multiLine) {
    multiLine_ = multiLine;
    sizeDirty_ = true;
    updateSpatialIndex();
}

void Label::setLineSpacing(float spacing) {
    lineSpacing_ = spacing;
    sizeDirty_ = true;
    updateSpatialIndex();
}

void Label::setMaxWidth(float maxWidth) {
    maxWidth_ = maxWidth;
    sizeDirty_ = true;
    updateSpatialIndex();
}

Vec2 Label::getTextSize() const {
    updateCache();
    return cachedSize_;
}

float Label::getLineHeight() const {
    if (font_) {
        return font_->getLineHeight() * lineSpacing_;
    }
    return static_cast<float>(fontSize_) * lineSpacing_;
}

void Label::updateCache() const {
    if (!sizeDirty_ || !font_) {
        return;
    }

    if (multiLine_) {
        auto lines = splitLines();
        float maxWidth = 0.0f;
        float totalHeight = 0.0f;
        float lineHeight = getLineHeight();
        
        for (size_t i = 0; i < lines.size(); ++i) {
            Vec2 lineSize = font_->measureText(lines[i]);
            maxWidth = std::max(maxWidth, lineSize.x);
            totalHeight += lineHeight;
        }
        
        cachedSize_ = Vec2(maxWidth, totalHeight);
    } else {
        cachedSize_ = font_->measureText(text_);
    }
    
    sizeDirty_ = false;
}

std::vector<std::string> Label::splitLines() const {
    std::vector<std::string> lines;
    if (text_.empty()) {
        return lines;
    }

    if (maxWidth_ <= 0.0f || !font_) {
        lines.push_back(text_);
        return lines;
    }

    // 按换行符分割
    size_t start = 0;
    size_t end = text_.find('\n');
    
    while (end != std::string::npos) {
        std::string line = text_.substr(start, end - start);
        
        // 如果单行超过最大宽度，需要自动换行
        Vec2 lineSize = font_->measureText(line);
        if (lineSize.x > maxWidth_) {
            // 简单实现：按字符逐个尝试
            std::string currentLine;
            for (size_t i = 0; i < line.length(); ++i) {
                std::string testLine = currentLine + line[i];
                Vec2 testSize = font_->measureText(testLine);
                if (testSize.x > maxWidth_ && !currentLine.empty()) {
                    lines.push_back(currentLine);
                    currentLine = line[i];
                } else {
                    currentLine = testLine;
                }
            }
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }
        } else {
            lines.push_back(line);
        }
        
        start = end + 1;
        end = text_.find('\n', start);
    }
    
    // 处理最后一行
    if (start < text_.length()) {
        std::string line = text_.substr(start);
        Vec2 lineSize = font_->measureText(line);
        if (lineSize.x > maxWidth_) {
            std::string currentLine;
            for (size_t i = 0; i < line.length(); ++i) {
                std::string testLine = currentLine + line[i];
                Vec2 testSize = font_->measureText(testLine);
                if (testSize.x > maxWidth_ && !currentLine.empty()) {
                    lines.push_back(currentLine);
                    currentLine = line[i];
                } else {
                    currentLine = testLine;
                }
            }
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }
        } else {
            lines.push_back(line);
        }
    }
    
    return lines;
}

Vec2 Label::calculateDrawPosition() const {
    Vec2 pos = getPosition();
    Vec2 size = getTextSize();
    Size widgetSize = getSize();
    
    // 如果设置了控件大小，使用控件大小作为对齐参考
    float refWidth = widgetSize.empty() ? size.x : widgetSize.width;
    float refHeight = widgetSize.empty() ? size.y : widgetSize.height;
    
    // 水平对齐
    switch (hAlign_) {
        case HorizontalAlign::Center:
            pos.x += (refWidth - size.x) * 0.5f;
            break;
        case HorizontalAlign::Right:
            pos.x += refWidth - size.x;
            break;
        case HorizontalAlign::Left:
        default:
            break;
    }
    
    // 垂直对齐
    switch (vAlign_) {
        case VerticalAlign::Middle:
            pos.y += (refHeight - size.y) * 0.5f;
            break;
        case VerticalAlign::Bottom:
            pos.y += refHeight - size.y;
            break;
        case VerticalAlign::Top:
        default:
            break;
    }
    
    return pos;
}

void Label::drawText(RenderBackend &renderer, const Vec2 &position, const Color &color) {
    if (!font_ || text_.empty()) {
        return;
    }

    if (multiLine_) {
        auto lines = splitLines();
        float lineHeight = getLineHeight();
        Vec2 pos = position;
        
        for (const auto &line : lines) {
            renderer.drawText(*font_, line, pos, color);
            pos.y += lineHeight;
        }
    } else {
        renderer.drawText(*font_, text_, position, color);
    }
}

Rect Label::getBoundingBox() const {
    if (!font_ || text_.empty()) {
        return Rect();
    }

    updateCache();
    Vec2 size = cachedSize_;
    if (size.x <= 0.0f || size.y <= 0.0f) {
        return Rect();
    }

    Vec2 pos = calculateDrawPosition();
    return Rect(pos.x, pos.y, size.x, size.y);
}

void Label::onDrawWidget(RenderBackend &renderer) {
    if (!font_ || text_.empty()) {
        return;
    }

    Vec2 pos = calculateDrawPosition();

    // 绘制阴影
    if (shadowEnabled_) {
        Vec2 shadowPos = pos + shadowOffset_;
        drawText(renderer, shadowPos, shadowColor_);
    }

    // 绘制描边（简化实现：向8个方向偏移绘制）
    if (outlineEnabled_) {
        float w = outlineWidth_;
        drawText(renderer, pos + Vec2(-w, -w), outlineColor_);
        drawText(renderer, pos + Vec2(0, -w), outlineColor_);
        drawText(renderer, pos + Vec2(w, -w), outlineColor_);
        drawText(renderer, pos + Vec2(-w, 0), outlineColor_);
        drawText(renderer, pos + Vec2(w, 0), outlineColor_);
        drawText(renderer, pos + Vec2(-w, w), outlineColor_);
        drawText(renderer, pos + Vec2(0, w), outlineColor_);
        drawText(renderer, pos + Vec2(w, w), outlineColor_);
    }

    // 绘制主文本
    drawText(renderer, pos, textColor_);
}

} // namespace extra2d
