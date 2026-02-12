// ============================================================================
// Number.cpp - 数字显示类实现
// ============================================================================

#include "Number.h"
#include "ResLoader.h"

namespace flappybird {

Number::Number() : number_(0) {
}

void Number::setNumber(int number) {
    number_ = number;
    createNumberSprites(number, "number_big_");
}

void Number::setLittleNumber(int number) {
    number_ = number;
    createNumberSprites(number, "number_medium_");
}

void Number::createNumberSprites(int number, const std::string& prefix) {
    // 清除之前的数字精灵
    removeAllChildren();

    // 获取数字 0 的高度作为参考
    auto zeroFrame = ResLoader::getKeyFrame(prefix + "0");
    float digitHeight = zeroFrame ? zeroFrame->getRect().size.height : 36.0f;

    // 收集所有数字位
    std::vector<int> digits;
    if (number == 0) {
        digits.push_back(0);
    } else {
        while (number > 0) {
            digits.push_back(number % 10);
            number /= 10;
        }
    }

    // 计算总宽度
    float totalWidth = 0.0f;
    std::vector<float> digitWidths;
    for (int digit : digits) {
        auto frame = ResLoader::getKeyFrame(prefix + std::to_string(digit));
        float width = frame ? frame->getRect().size.width : 24.0f;
        digitWidths.push_back(width);
        totalWidth += width;
    }

    // 创建数字精灵并居中排列
    float currentX = -totalWidth / 2.0f;
    for (size_t i = 0; i < digits.size(); ++i) {
        auto frame = ResLoader::getKeyFrame(prefix + std::to_string(digits[i]));
        if (frame) {
            auto digitSprite = extra2d::Sprite::create(frame->getTexture(), frame->getRect());
            digitSprite->setAnchor(extra2d::Vec2(0.0f, 0.0f));
            digitSprite->setPosition(extra2d::Vec2(currentX, -digitHeight / 2.0f));
            addChild(digitSprite);
        }
        currentX += digitWidths[i];
    }
}

} // namespace flappybird
