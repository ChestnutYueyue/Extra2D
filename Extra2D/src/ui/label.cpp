#include <extra2d/ui/label.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 */
Label::Label() {
    setAnchor(0.0f, 0.0f);
}

/**
 * @brief 带文本的构造函数
 * @param text 初始文本内容
 */
Label::Label(const std::string &text) : text_(text) {
    setAnchor(0.0f, 0.0f);
    sizeDirty_ = true;
}

/**
 * @brief 创建空标签对象
 * @return 标签对象指针
 */
Ptr<Label> Label::create() {
    return makePtr<Label>();
}

/**
 * @brief 创建带文本的标签对象
 * @param text 文本内容
 * @return 标签对象指针
 */
Ptr<Label> Label::create(const std::string &text) {
    return makePtr<Label>(text);
}

/**
 * @brief 创建带文本和字体的标签对象
 * @param text 文本内容
 * @param font 字体图集
 * @return 标签对象指针
 */
Ptr<Label> Label::create(const std::string &text, Ptr<FontAtlas> font) {
    auto label = makePtr<Label>(text);
    label->setFont(font);
    return label;
}

/**
 * @brief 设置文本内容
 * @param text 新的文本内容
 */
void Label::setText(const std::string &text) {
    text_ = text;
    sizeDirty_ = true;
    updateSpatialIndex();
}

/**
 * @brief 设置字体
 * @param font 字体图集指针
 */
void Label::setFont(Ptr<FontAtlas> font) {
    font_ = font;
    sizeDirty_ = true;
    updateSpatialIndex();
}

/**
 * @brief 设置文本颜色
 * @param color 文本颜色
 */
void Label::setTextColor(const Color &color) {
    textColor_ = color;
}

/**
 * @brief 设置字体大小
 * @param size 字体大小
 */
void Label::setFontSize(int size) {
    fontSize_ = size;
    sizeDirty_ = true;
    updateSpatialIndex();
}

/**
 * @brief 设置水平对齐方式
 * @param align 对齐方式
 */
void Label::setHorizontalAlign(HorizontalAlign align) {
    hAlign_ = align;
}

/**
 * @brief 设置垂直对齐方式
 * @param align 垂直对齐方式
 */
void Label::setVerticalAlign(VerticalAlign align) {
    vAlign_ = align;
}

/**
 * @brief 设置阴影是否启用
 * @param enabled 是否启用
 */
void Label::setShadowEnabled(bool enabled) {
    shadowEnabled_ = enabled;
}

/**
 * @brief 设置阴影颜色
 * @param color 阴影颜色
 */
void Label::setShadowColor(const Color &color) {
    shadowColor_ = color;
}

/**
 * @brief 设置阴影偏移
 * @param offset 偏移向量
 */
void Label::setShadowOffset(const Vec2 &offset) {
    shadowOffset_ = offset;
}

/**
 * @brief 设置描边是否启用
 * @param enabled 是否启用
 */
void Label::setOutlineEnabled(bool enabled) {
    outlineEnabled_ = enabled;
}

/**
 * @brief 设置描边颜色
 * @param color 描边颜色
 */
void Label::setOutlineColor(const Color &color) {
    outlineColor_ = color;
}

/**
 * @brief 设置描边宽度
 * @param width 描边宽度
 */
void Label::setOutlineWidth(float width) {
    outlineWidth_ = width;
}

/**
 * @brief 设置是否多行模式
 * @param multiLine 是否多行
 */
void Label::setMultiLine(bool multiLine) {
    multiLine_ = multiLine;
    sizeDirty_ = true;
    updateSpatialIndex();
}

/**
 * @brief 设置行间距
 * @param spacing 行间距倍数
 */
void Label::setLineSpacing(float spacing) {
    lineSpacing_ = spacing;
    sizeDirty_ = true;
    updateSpatialIndex();
}

/**
 * @brief 设置最大宽度
 * @param maxWidth 最大宽度
 */
void Label::setMaxWidth(float maxWidth) {
    maxWidth_ = maxWidth;
    sizeDirty_ = true;
    updateSpatialIndex();
}

/**
 * @brief 获取文本尺寸
 * @return 文本的宽度和高度
 */
Vec2 Label::getTextSize() const {
    updateCache();
    return cachedSize_;
}

/**
 * @brief 获取行高
 * @return 行高值
 */
float Label::getLineHeight() const {
    if (font_) {
        return font_->getLineHeight() * lineSpacing_;
    }
    return static_cast<float>(fontSize_) * lineSpacing_;
}

/**
 * @brief 更新缓存
 */
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

/**
 * @brief 分割文本为多行
 * @return 分割后的行列表
 */
std::vector<std::string> Label::splitLines() const {
    std::vector<std::string> lines;
    if (text_.empty()) {
        return lines;
    }

    if (maxWidth_ <= 0.0f || !font_) {
        lines.push_back(text_);
        return lines;
    }

    size_t start = 0;
    size_t end = text_.find('\n');
    
    while (end != std::string::npos) {
        std::string line = text_.substr(start, end - start);
        
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
        
        start = end + 1;
        end = text_.find('\n', start);
    }
    
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

/**
 * @brief 计算绘制位置
 * @return 绘制位置坐标
 */
Vec2 Label::calculateDrawPosition() const {
    Vec2 pos = getPosition();
    Vec2 size = getTextSize();
    Size widgetSize = getSize();
    
    float refWidth = widgetSize.empty() ? size.x : widgetSize.width;
    float refHeight = widgetSize.empty() ? size.y : widgetSize.height;
    
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

/**
 * @brief 绘制文本
 * @param renderer 渲染后端
 * @param position 绘制位置
 * @param color 文本颜色
 */
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

/**
 * @brief 获取边界框
 * @return 边界矩形
 */
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

/**
 * @brief 绘制组件
 * @param renderer 渲染后端
 */
void Label::onDrawWidget(RenderBackend &renderer) {
    if (!font_ || text_.empty()) {
        return;
    }

    Vec2 pos = calculateDrawPosition();

    if (shadowEnabled_) {
        Vec2 shadowPos = pos + shadowOffset_;
        drawText(renderer, shadowPos, shadowColor_);
    }

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

    drawText(renderer, pos, textColor_);
}

} // namespace extra2d
