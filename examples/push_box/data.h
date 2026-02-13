#pragma once

#define MAX_LEVEL 8
#define GAME_WIDTH 640.0f
#define GAME_HEIGHT 480.0f

namespace pushbox {

enum TYPE { Empty, Wall, Ground, Box, Man };

struct Piece {
    TYPE type;
    bool isPoint;
};

struct Map {
    int width;
    int height;
    int roleX;
    int roleY;
    Piece value[12][12];
};

/**
 * @brief 移动记录 - 用于撤销功能（对象池示例）
 * 这个结构体演示如何使用对象池管理小对象
 */
struct MoveRecord {
    int fromX, fromY;
    int toX, toY;
    int boxFromX, boxFromY;
    int boxToX, boxToY;
    bool pushedBox;
    
    MoveRecord() = default;
    MoveRecord(int fx, int fy, int tx, int ty, bool pushed = false)
        : fromX(fx), fromY(fy), toX(tx), toY(ty)
        , boxFromX(-1), boxFromY(-1), boxToX(-1), boxToY(-1)
        , pushedBox(pushed) {}
};

extern Map g_Maps[MAX_LEVEL];
extern int g_CurrentLevel;
extern bool g_SoundOpen;
extern int g_Direct;
extern bool g_Pushing;

} // namespace pushbox
