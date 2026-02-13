# 07. UI 系统

Extra2D 提供了一套完整的 UI 系统，支持按钮、文本、标签、复选框、单选按钮、滑块、进度条等常用控件。

## UI 控件类型

```
Widget (UI控件基类)
├── Button (按钮)
├── Text (文本)
├── Label (标签)
├── CheckBox (复选框)
├── RadioButton (单选按钮)
├── Slider (滑块)
└── ProgressBar (进度条)
```

## 坐标空间

UI 控件支持三种坐标空间：

```cpp
enum class CoordinateSpace {
    World,    // 世界空间 - 随相机移动（默认）
    Screen,   // 屏幕空间 - 固定位置，不随相机移动
    Camera,   // 相机空间 - 相对于相机位置的偏移
};
```

设置坐标空间：

```cpp
// 屏幕空间（UI 常用）
button->setCoordinateSpace(CoordinateSpace::Screen);
button->setScreenPosition(100.0f, 50.0f);
```

## 通用设置方法

所有 UI 组件都支持以下设置方法（来自基类 Node 和 Widget）：

```cpp
// 来自 Node 基类
widget->setPosition(x, y);           // 设置位置
widget->setAnchor(x, y);             // 设置锚点 (0-1)

// 来自 Widget 基类
widget->setCoordinateSpace(space);   // 设置坐标空间
widget->setScreenPosition(x, y);     // 设置屏幕位置
widget->setCameraOffset(x, y);       // 设置相机偏移
```

## 按钮（Button）

### 创建按钮

```cpp
auto& resources = Application::instance().resources();
auto font = resources.loadFont("assets/font.ttf", 24);

// 创建按钮
auto button = Button::create();
button->setText("点击我");
button->setFont(font);
button->setPosition(Vec2(400, 300));
button->setSize(200, 60);
button->setTextColor(Colors::White);
button->setBackgroundColor(Colors::Blue, Colors::Green, Colors::Red);
button->setBorder(Colors::White, 2.0f);

// 设置点击回调
button->setOnClick([]() {
    E2D_LOG_INFO("按钮被点击！");
});

addChild(button);
```

### 按钮属性设置

```cpp
// 文本和字体
button->setText("新文本");
button->setFont(font);
button->setTextColor(Colors::White);

// 尺寸和内边距
button->setCustomSize(200.0f, 60.0f);
button->setPadding(Vec2(10.0f, 5.0f));

// 背景颜色（正常、悬停、按下三种状态）
button->setBackgroundColor(
    Colors::Blue,    // 正常状态
    Colors::Green,   // 悬停状态
    Colors::Red      // 按下状态
);

// 边框
button->setBorder(Colors::White, 2.0f);

// 圆角
button->setRoundedCornersEnabled(true);
button->setCornerRadius(8.0f);

// 图片背景（普通按钮）
button->setBackgroundImage(normalTex, hoverTex, pressedTex);
button->setBackgroundImageScaleMode(ImageScaleMode::ScaleFit);

// 悬停光标
button->setHoverCursor(CursorShape::Hand);
```

### 切换按钮模式（Toggle Button）

按钮支持切换模式，可以在 on/off 两种状态间切换：

```cpp
// 创建切换按钮
auto toggleBtn = Button::create();
toggleBtn->setToggleMode(true);  // 启用切换模式

// 设置状态图片（off状态图片, on状态图片）
toggleBtn->setStateBackgroundImage(soundOffTex, soundOnTex);

// 或设置带悬停/按下效果的状态图片
toggleBtn->setStateBackgroundImage(
    soundOffTex, soundOnTex,           // 普通状态
    soundOffHoverTex, soundOnHoverTex, // 悬停状态（可选）
    soundOffPressedTex, soundOnPressedTex  // 按下状态（可选）
);

// 设置状态文字
toggleBtn->setStateText("关闭", "开启");

// 设置状态文字颜色
toggleBtn->setStateTextColor(Colors::Red, Colors::Green);

// 设置初始状态
toggleBtn->setOn(true);

// 获取当前状态
bool isOn = toggleBtn->isOn();

// 手动切换状态
toggleBtn->toggle();

// 状态改变回调
toggleBtn->setOnStateChange([](bool isOn) {
    E2D_LOG_INFO("切换按钮状态: {}", isOn ? "开启" : "关闭");
});

// 点击回调（切换模式也会触发此回调）
toggleBtn->setOnClick([]() {
    E2D_LOG_INFO("按钮被点击");
});
```

### 图片缩放模式

```cpp
enum class ImageScaleMode {
    Original, // 使用原图大小
    Stretch,  // 拉伸填充
    ScaleFit, // 等比缩放，保持完整显示
    ScaleFill // 等比缩放，填充整个区域（可能裁剪）
};
```

### 透明按钮（菜单项）

```cpp
// 创建纯文本按钮（透明背景，用于菜单）
auto menuBtn = Button::create();
menuBtn->setFont(font);
menuBtn->setText("新游戏");
menuBtn->setTextColor(Colors::Black);
menuBtn->setBackgroundColor(
    Colors::Transparent,  // 正常
    Colors::Transparent,  // 悬停
    Colors::Transparent   // 按下
);
menuBtn->setBorder(Colors::Transparent, 0.0f);
menuBtn->setPadding(Vec2(0.0f, 0.0f));
menuBtn->setCustomSize(200.0f, 40.0f);
menuBtn->setAnchor(0.5f, 0.5f);  // 中心锚点
menuBtn->setPosition(centerX, centerY);
addChild(menuBtn);
```

### 按钮启用/禁用

Widget 基类提供了 `setEnabled()` 方法控制按钮的交互状态：

```cpp
// 禁用按钮
button->setEnabled(false);

// 启用按钮
button->setEnabled(true);

// 检查按钮状态
bool isEnabled = button->isEnabled();
```

### 完整示例：动画完成后启用按钮

参考 `examples/flappy_bird/GameOverLayer.cpp`：

```cpp
// GameOverLayer.h
class GameOverLayer : public extra2d::Node {
public:
    GameOverLayer(int score);
    void onEnter() override;
    void onUpdate(float dt) override;

private:
    void initButtons();
    
    int score_ = 0;
    bool animationDone_ = false;
    extra2d::Ptr<extra2d::Button> restartBtn_;
    extra2d::Ptr<extra2d::Button> menuBtn_;
};

// GameOverLayer.cpp
void GameOverLayer::onEnter() {
    Node::onEnter();
    
    // 初始化按钮（初始禁用）
    initButtons();
    
    // 创建动画
    auto moveAction = extra2d::makePtr<extra2d::MoveBy>(
        1.0f, extra2d::Vec2(0.0f, -screenHeight));
    
    // 动画完成后启用按钮
    moveAction->setCompletionCallback([this]() {
        animationDone_ = true;
        if (restartBtn_)
            restartBtn_->setEnabled(true);
        if (menuBtn_)
            menuBtn_->setEnabled(true);
    });
    
    runAction(moveAction);
}

void GameOverLayer::initButtons() {
    auto restartFrame = ResLoader::getKeyFrame("button_restart");
    if (restartFrame) {
        restartBtn_ = extra2d::Button::create();
        restartBtn_->setBackgroundImage(restartFrame->getTexture(),
                                        restartFrame->getRect());
        restartBtn_->setAnchor(extra2d::Vec2(0.5f, 0.5f));
        restartBtn_->setPosition(extra2d::Vec2(0.0f, 360.0f));
        restartBtn_->setEnabled(false);  // 初始禁用
        restartBtn_->setOnClick([]() {
            // 处理点击
        });
        addChild(restartBtn_);
    }
    
    // 菜单按钮类似...
}

void GameOverLayer::onUpdate(float dt) {
    Node::onUpdate(dt);
    
    // 动画完成后才响应手柄输入
    if (!animationDone_)
        return;
    
    auto &input = extra2d::Application::instance().input();
    if (input.isButtonPressed(extra2d::GamepadButton::A)) {
        // 重新开始
    }
}
```

### 使用场景

- **动画播放期间**：禁用按钮，防止用户过早操作
- **加载过程中**：禁用按钮，显示加载状态
- **条件限制**：当条件不满足时禁用按钮（如未选择关卡）
- **切换按钮**：音效开关、设置开关等需要显示两种状态的按钮

## 文本（Text）

### 创建文本

```cpp
// 创建文本
auto text = Text::create("Hello World", font);
text->setPosition(Vec2(100, 50));
text->setTextColor(Colors::White);
text->setFontSize(24);
text->setAlignment(Alignment::Center);

addChild(text);
```

### 文本属性设置

```cpp
// 设置文本
text->setText("新文本");
text->setFormat("Score: %d", score);  // 格式化文本

// 字体和颜色
text->setFont(font);
text->setTextColor(Colors::White);
text->setFontSize(24);

// 对齐方式
text->setAlignment(Alignment::Left);      // 水平：左/中/右
text->setVerticalAlignment(VerticalAlignment::Middle);  // 垂直：上/中/下

// 获取文本尺寸
Size size = text->getTextSize();
float lineHeight = text->getLineHeight();
```

### 对齐方式枚举

```cpp
enum class Alignment { Left, Center, Right };
enum class VerticalAlignment { Top, Middle, Bottom };
```

## 标签（Label）

Label 是功能更丰富的文本组件，支持阴影、描边、多行文本。

### 创建标签

```cpp
// 创建标签
auto label = Label::create("玩家名称", font);
label->setPosition(Vec2(100, 50));
label->setTextColor(Colors::White);

addChild(label);
```

### 标签特效

```cpp
// 阴影
label->setShadowEnabled(true);
label->setShadowColor(Colors::Black);
label->setShadowOffset(Vec2(2.0f, 2.0f));

// 描边
label->setOutlineEnabled(true);
label->setOutlineColor(Colors::Black);
label->setOutlineWidth(1.0f);

// 多行文本
label->setMultiLine(true);
label->setLineSpacing(1.2f);
label->setMaxWidth(300.0f);  // 自动换行宽度

// 对齐方式
label->setHorizontalAlign(HorizontalAlign::Center);
label->setVerticalAlign(VerticalAlign::Middle);
```

## 复选框（CheckBox）

### 创建复选框

```cpp
// 方式1：简单创建
auto checkBox = CheckBox::create();
checkBox->setPosition(Vec2(100, 200));
checkBox->setChecked(true);

// 方式2：带标签
checkBox = CheckBox::create("启用音效");
checkBox->setPosition(Vec2(100, 200));
checkBox->setFont(font);
checkBox->setTextColor(Colors::White);

// 状态改变回调
checkBox->setOnStateChange([](bool checked) {
    E2D_LOG_INFO("复选框状态: {}", checked);
});

addChild(checkBox);
```

### 复选框属性

```cpp
// 状态
checkBox->setChecked(true);
checkBox->toggle();
bool isChecked = checkBox->isChecked();

// 标签
checkBox->setLabel("新标签");
checkBox->setFont(font);
checkBox->setTextColor(Colors::White);

// 外观
checkBox->setBoxSize(20.0f);           // 复选框大小
checkBox->setSpacing(10.0f);           // 复选框与标签间距
checkBox->setCheckedColor(Colors::Green);
checkBox->setUncheckedColor(Colors::Gray);
checkBox->setCheckMarkColor(Colors::White);
```

## 单选按钮（RadioButton）

### 创建单选按钮

```cpp
// 创建单选按钮
auto radio1 = RadioButton::create("选项 A");
radio1->setPosition(Vec2(100, 300));
radio1->setSelected(true);

auto radio2 = RadioButton::create("选项 B");
radio2->setPosition(Vec2(100, 340));

auto radio3 = RadioButton::create("选项 C");
radio3->setPosition(Vec2(100, 380));

// 添加到组（互斥选择）
radio1->setGroupId(1);
radio2->setGroupId(1);
radio3->setGroupId(1);

// 或使用 RadioButtonGroup
auto group = std::make_shared<RadioButtonGroup>();
group->addButton(radio1.get());
group->addButton(radio2.get());
group->addButton(radio3.get());

// 选择改变回调
group->setOnSelectionChange([](RadioButton* selected) {
    if (selected) {
        E2D_LOG_INFO("选中: {}", selected->getLabel());
    }
});

addChild(radio1);
addChild(radio2);
addChild(radio3);
```

### 单选按钮属性

```cpp
// 状态
radio->setSelected(true);
bool isSelected = radio->isSelected();

// 标签
radio->setLabel("新标签");
radio->setFont(font);
radio->setTextColor(Colors::White);

// 外观
radio->setCircleSize(16.0f);           // 圆形大小
radio->setSpacing(10.0f);              // 圆形与标签间距
radio->setSelectedColor(Colors::Green);
radio->setUnselectedColor(Colors::Gray);
radio->setDotColor(Colors::White);

// 分组
radio->setGroupId(1);  // 相同 groupId 的按钮互斥
```

## 滑块（Slider）

### 创建滑块

```cpp
// 方式1：简单创建
auto slider = Slider::create();
slider->setPosition(Vec2(200, 400));
slider->setRange(0.0f, 100.0f);
slider->setValue(50.0f);

// 方式2：带初始值创建
auto slider = Slider::create(0.0f, 100.0f, 50.0f);

// 值改变回调
slider->setOnValueChange([](float value) {
    E2D_LOG_INFO("滑块值: {}", value);
});

// 拖动开始/结束回调
slider->setOnDragStart([]() {
    E2D_LOG_INFO("开始拖动");
});
slider->setOnDragEnd([]() {
    E2D_LOG_INFO("结束拖动");
});

addChild(slider);
```

### 滑块属性

```cpp
// 值和范围
slider->setRange(0.0f, 100.0f);
slider->setValue(50.0f);
slider->setStep(5.0f);  // 步进值，0表示无步进
float value = slider->getValue();
float min = slider->getMin();
float max = slider->getMax();

// 方向
slider->setVertical(false);  // false=水平, true=垂直

// 外观
slider->setTrackSize(4.0f);   // 轨道粗细
slider->setThumbSize(16.0f);  // 滑块大小

// 颜色
slider->setTrackColor(Colors::Gray);
slider->setFillColor(Colors::Green);
slider->setThumbColor(Colors::White);
slider->setThumbHoverColor(Colors::Yellow);
slider->setThumbPressedColor(Colors::Orange);

// 显示选项
slider->setShowThumb(true);   // 显示滑块
slider->setShowFill(true);    // 显示填充
slider->setTextEnabled(true); // 显示数值文本
slider->setTextFormat("{:.0f}%");  // 数值格式
slider->setFont(font);
slider->setTextColor(Colors::White);
```

## 进度条（ProgressBar）

### 创建进度条

```cpp
// 方式1：简单创建
auto progressBar = ProgressBar::create();
progressBar->setPosition(Vec2(200, 500));
progressBar->setSize(300.0f, 30.0f);
progressBar->setValue(75.0f);  // 75%

// 方式2：带范围创建
auto progressBar = ProgressBar::create(0.0f, 100.0f, 75.0f);

addChild(progressBar);
```

### 进度条属性

```cpp
// 值和范围
progressBar->setRange(0.0f, 100.0f);
progressBar->setValue(75.0f);
float value = progressBar->getValue();
float percent = progressBar->getPercent();  // 0.0-1.0

// 方向
progressBar->setDirection(Direction::LeftToRight);
// Direction::LeftToRight, RightToLeft, BottomToTop, TopToBottom

// 颜色
progressBar->setBackgroundColor(Colors::DarkGray);
progressBar->setFillColor(Colors::Green);

// 渐变填充
progressBar->setGradientFillEnabled(true);
progressBar->setFillColorEnd(Colors::LightGreen);

// 分段颜色（根据进度显示不同颜色）
progressBar->setSegmentedColorsEnabled(true);
progressBar->addColorSegment(0.3f, Colors::Red);     // <30% 红色
progressBar->addColorSegment(0.7f, Colors::Yellow);  // 30-70% 黄色
// >70% 使用默认填充色（绿色）

// 圆角
progressBar->setRoundedCornersEnabled(true);
progressBar->setCornerRadius(8.0f);

// 边框
progressBar->setBorderEnabled(true);
progressBar->setBorderColor(Colors::White);
progressBar->setBorderWidth(2.0f);
progressBar->setPadding(2.0f);

// 文本
progressBar->setTextEnabled(true);
progressBar->setTextFormat("{:.0f}%");
progressBar->setFont(font);
progressBar->setTextColor(Colors::White);

// 动画效果
progressBar->setAnimatedChangeEnabled(true);
progressBar->setAnimationSpeed(5.0f);

// 延迟显示效果
progressBar->setDelayedDisplayEnabled(true);
progressBar->setDelayTime(0.5f);
progressBar->setDelayedFillColor(Colors::Yellow);

// 条纹效果
progressBar->setStripedEnabled(true);
progressBar->setStripeColor(Colors::White);
progressBar->setStripeSpeed(1.0f);
```

## 完整示例：设置场景

```cpp
class SettingsScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("assets/font.ttf", 24);
        
        // 标题
        auto title = Text::create("设置", font_);
        title->setPosition(Vec2(400, 100));
        title->setAlignment(Alignment::Center);
        addChild(title);
        
        // 音效开关（使用切换按钮）
        auto soundLabel = Label::create("音效", font_);
        soundLabel->setPosition(Vec2(200, 200));
        addChild(soundLabel);
        
        auto soundOn = resources.loadTexture("assets/sound_on.png");
        auto soundOff = resources.loadTexture("assets/sound_off.png");
        
        soundToggle_ = Button::create();
        soundToggle_->setPosition(Vec2(350, 200));
        soundToggle_->setToggleMode(true);
        soundToggle_->setStateBackgroundImage(soundOff, soundOn);
        soundToggle_->setOn(true);
        soundToggle_->setOnStateChange([](bool isOn) {
            E2D_LOG_INFO("音效: {}", isOn ? "开启" : "关闭");
            AudioManager::instance().setEnabled(isOn);
        });
        addChild(soundToggle_);
        
        // 音量滑块
        auto volumeLabel = Label::create("音量", font_);
        volumeLabel->setPosition(Vec2(200, 280));
        addChild(volumeLabel);
        
        volumeSlider_ = Slider::create(0.0f, 1.0f, 0.8f);
        volumeSlider_->setPosition(Vec2(350, 280));
        volumeSlider_->setSize(200, 20);
        volumeSlider_->setOnValueChange([](float value) {
            E2D_LOG_INFO("音量: {:.0f}%", value * 100);
        });
        addChild(volumeSlider_);
        
        // 难度选择
        auto difficultyLabel = Label::create("难度", font_);
        difficultyLabel->setPosition(Vec2(200, 360));
        addChild(difficultyLabel);
        
        auto easyRadio = RadioButton::create("简单");
        easyRadio->setPosition(Vec2(350, 360));
        easyRadio->setSelected(true);
        easyRadio->setGroupId(1);
        
        auto normalRadio = RadioButton::create("普通");
        normalRadio->setPosition(Vec2(450, 360));
        normalRadio->setGroupId(1);
        
        auto hardRadio = RadioButton::create("困难");
        hardRadio->setPosition(Vec2(550, 360));
        hardRadio->setGroupId(1);
        
        addChild(easyRadio);
        addChild(normalRadio);
        addChild(hardRadio);
        
        // 返回按钮
        auto backBtn = Button::create("返回", font_);
        backBtn->setPosition(Vec2(400, 500));
        backBtn->setCustomSize(150.0f, 50.0f);
        backBtn->setBackgroundColor(
            Colors::Blue,
            Colors::LightBlue,
            Colors::DarkBlue
        );
        backBtn->setOnClick([]() {
            Application::instance().scenes().popScene();
        });
        addChild(backBtn);
    }
    
private:
    Ptr<FontAtlas> font_;
    Ptr<Button> soundToggle_;
    Ptr<Slider> volumeSlider_;
};
```

## 最佳实践

1. **使用屏幕空间** - UI 控件通常使用 `CoordinateSpace::Screen` 固定在屏幕上
2. **设置合适的锚点** - 使用锚点（0.5, 0.5）让控件中心对齐，方便布局
3. **复用字体资源** - 避免重复加载相同字体
4. **使用回调函数** - 使用 `setOnClick`、`setOnValueChange` 等回调响应用户操作
5. **使用切换按钮** - 对于需要显示两种状态的按钮（如开关），使用 `setToggleMode(true)`

## 下一步

- [08. 音频系统](./08_Audio_System.md) - 学习音频播放
