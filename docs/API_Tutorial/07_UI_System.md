# Extra2D API 教程 - 07. UI 系统

## 按钮 (Button)

Extra2D 提供 Button 组件用于创建交互式按钮。

### 创建按钮

```cpp
// 创建按钮
auto button = Button::create();

// 设置位置
button->setPosition(Vec2(640.0f, 360.0f));

// 设置锚点（中心）
button->setAnchor(0.5f, 0.5f);

// 添加到场景
addChild(button);
```

### 设置按钮文本

```cpp
// 加载字体
auto font = resources.loadFont("assets/font.ttf", 28, true);

// 设置按钮字体和文本
button->setFont(font);
button->setText("点击我");
button->setTextColor(Colors::Black);
```

### 设置按钮样式

```cpp
// 设置背景颜色（普通、悬停、按下）
button->setBackgroundColor(
    Colors::White,      // 普通状态
    Colors::LightGray,  // 悬停状态
    Colors::Gray        // 按下状态
);

// 设置边框
button->setBorder(Colors::Black, 2.0f);

// 设置内边距
button->setPadding(Vec2(20.0f, 10.0f));

// 设置固定大小
button->setCustomSize(200.0f, 50.0f);
```

### 透明按钮

```cpp
// 创建透明按钮（仅文本可点击）
auto button = Button::create();
button->setFont(font);
button->setText("菜单项");
button->setTextColor(Colors::Black);

// 透明背景
button->setBackgroundColor(
    Colors::Transparent,
    Colors::Transparent,
    Colors::Transparent
);

// 无边框
button->setBorder(Colors::Transparent, 0.0f);

// 无内边距
button->setPadding(Vec2(0.0f, 0.0f));
```

## 菜单系统

### 创建菜单

```cpp
class MenuScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        auto &resources = Application::instance().resources();
        font_ = resources.loadFont("assets/font.ttf", 28, true);
        
        float centerX = 640.0f;
        float startY = 300.0f;
        
        // 创建菜单按钮
        createMenuButton("开始游戏", centerX, startY, 0);
        createMenuButton("继续游戏", centerX, startY + 50.0f, 1);
        createMenuButton("退出", centerX, startY + 100.0f, 2);
        
        menuCount_ = 3;
        selectedIndex_ = 0;
        updateMenuColors();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        auto &input = Application::instance().input();
        
        // 方向键切换选择
        if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_DPAD_UP)) {
            selectedIndex_ = (selectedIndex_ - 1 + menuCount_) % menuCount_;
            updateMenuColors();
        } else if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
            selectedIndex_ = (selectedIndex_ + 1) % menuCount_;
            updateMenuColors();
        }
        
        // A键确认
        if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_A)) {
            executeMenuItem();
        }
    }
    
private:
    Ptr<FontAtlas> font_;
    std::vector<Ptr<Button>> buttons_;
    int selectedIndex_ = 0;
    int menuCount_ = 0;
    
    void createMenuButton(const std::string &text, float x, float y, int index) {
        auto button = Button::create();
        button->setFont(font_);
        button->setText(text);
        button->setTextColor(Colors::Black);
        button->setBackgroundColor(
            Colors::Transparent,
            Colors::Transparent,
            Colors::Transparent
        );
        button->setBorder(Colors::Transparent, 0.0f);
        button->setPadding(Vec2(0.0f, 0.0f));
        button->setCustomSize(200.0f, 40.0f);
        button->setAnchor(0.5f, 0.5f);
        button->setPosition(x, y);
        addChild(button);
        buttons_.push_back(button);
    }
    
    void updateMenuColors() {
        for (int i = 0; i < buttons_.size(); ++i) {
            if (buttons_[i]) {
                buttons_[i]->setTextColor(
                    i == selectedIndex_ ? Colors::Red : Colors::Black
                );
            }
        }
    }
    
    void executeMenuItem() {
        switch (selectedIndex_) {
            case 0: startGame(); break;
            case 1: continueGame(); break;
            case 2: exitGame(); break;
        }
    }
    
    void startGame() {
        // 切换到游戏场景
    }
    
    void continueGame() {
        // 继续游戏
    }
    
    void exitGame() {
        Application::instance().quit();
    }
};
```

## 绘制文字

### 基本文字绘制

```cpp
void onRender(RenderBackend &renderer) override {
    Scene::onRender(renderer);
    
    if (font_) {
        // 绘制文字
        renderer.drawText(*font_, "Hello World", 
                         Vec2(100.0f, 100.0f), Colors::White);
        
        // 绘制带颜色的文字
        renderer.drawText(*font_, "红色文字", 
                         Vec2(100.0f, 150.0f), Colors::Red);
    }
}
```

### 格式化文字

```cpp
void onRender(RenderBackend &renderer) override {
    Scene::onRender(renderer);
    
    if (infoFont_) {
        auto &app = Application::instance();
        
        // 绘制 FPS
        std::stringstream ss;
        ss << "FPS: " << app.fps();
        renderer.drawText(*infoFont_, ss.str(), 
                         Vec2(50.0f, 50.0f), Colors::Yellow);
        
        // 绘制节点数量
        ss.str("");
        ss << "Nodes: " << nodes_.size();
        renderer.drawText(*infoFont_, ss.str(), 
                         Vec2(50.0f, 80.0f), Colors::White);
    }
}
```

## 完整示例

```cpp
class StartScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        auto &app = Application::instance();
        auto &resources = app.resources();
        
        // 加载背景
        auto bgTex = resources.loadTexture("assets/background.jpg");
        if (bgTex) {
            auto bg = Sprite::create(bgTex);
            bg->setAnchor(0.0f, 0.0f);
            addChild(bg);
        }
        
        // 加载字体
        font_ = resources.loadFont("assets/font.ttf", 32, true);
        
        float centerX = app.getConfig().width / 2.0f;
        
        // 创建标题
        titleBtn_ = Button::create();
        titleBtn_->setFont(font_);
        titleBtn_->setText("游戏标题");
        titleBtn_->setTextColor(Colors::Gold);
        titleBtn_->setBackgroundColor(
            Colors::Transparent,
            Colors::Transparent,
            Colors::Transparent
        );
        titleBtn_->setBorder(Colors::Transparent, 0.0f);
        titleBtn_->setAnchor(0.5f, 0.5f);
        titleBtn_->setPosition(centerX, 200.0f);
        addChild(titleBtn_);
        
        // 创建菜单按钮
        createMenuButton("新游戏", centerX, 350.0f, 0);
        createMenuButton("继续", centerX, 400.0f, 1);
        createMenuButton("退出", centerX, 450.0f, 2);
        
        menuCount_ = 3;
        updateMenuColors();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        auto &input = Application::instance().input();
        
        if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_DPAD_UP)) {
            selectedIndex_ = (selectedIndex_ - 1 + menuCount_) % menuCount_;
            updateMenuColors();
        } else if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
            selectedIndex_ = (selectedIndex_ + 1) % menuCount_;
            updateMenuColors();
        }
        
        if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_A)) {
            executeMenuItem();
        }
    }
    
private:
    Ptr<FontAtlas> font_;
    Ptr<Button> titleBtn_;
    std::vector<Ptr<Button>> menuButtons_;
    int selectedIndex_ = 0;
    int menuCount_ = 0;
    
    void createMenuButton(const std::string &text, float x, float y, int index) {
        auto btn = Button::create();
        btn->setFont(font_);
        btn->setText(text);
        btn->setTextColor(Colors::White);
        btn->setBackgroundColor(
            Colors::Transparent,
            Colors::Transparent,
            Colors::Transparent
        );
        btn->setBorder(Colors::Transparent, 0.0f);
        btn->setAnchor(0.5f, 0.5f);
        btn->setPosition(x, y);
        addChild(btn);
        menuButtons_.push_back(btn);
    }
    
    void updateMenuColors() {
        for (int i = 0; i < menuButtons_.size(); ++i) {
            if (menuButtons_[i]) {
                menuButtons_[i]->setTextColor(
                    i == selectedIndex_ ? Colors::Red : Colors::White
                );
            }
        }
    }
    
    void executeMenuItem() {
        switch (selectedIndex_) {
            case 0: /* 新游戏 */ break;
            case 1: /* 继续 */ break;
            case 2: Application::instance().quit(); break;
        }
    }
};
```

## 下一步

- [08. 音频系统](08_Audio_System.md)
