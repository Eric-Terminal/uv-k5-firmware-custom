#include "ST7565.h"
#include "keyboard.h"
#include "system.h"
#include <stdbool.h>
#include <stdlib.h>

// 屏幕尺寸
#define LCD_W 128
#define LCD_H 64

// 恐龙参数
#define DINO_W 8
#define DINO_H 8
#define DINO_X 10
#define GROUND_Y (LCD_H - 14)

// 障碍物
#define OB_W 4
#define OB_H 10
#define OB_GAP 60

#define GRAVITY 2
#define JUMP_VY -14
#define FRAME_MS 40

typedef struct {
    int y;
    int vy;
    bool jumping;
} Dino;

typedef struct {
    int x;
    bool active;
} Obstacle;

static Dino dino;
static Obstacle obs;
static int score = 0;
static bool gameover = false;

// 简单8x8恐龙像素画
void draw_dino(int x, int y) {
    // 小“田”字当恐龙
    for (int dx = 2; dx < 6; ++dx)
        for (int dy = 2; dy < 6; ++dy)
            ST7565_DrawPixel(x + dx, y + dy, 1);
    // 头
    for (int dx = 5; dx < 8; ++dx)
        for (int dy = 0; dy < 3; ++dy)
            ST7565_DrawPixel(x + dx, y + dy, 1);
    // 小尾巴
    ST7565_DrawPixel(x, y+6， 1);
    ST7565_DrawPixel(x+1, y+7， 1);
}

// 画障碍物（小树桩）
void draw_obstacle(int x) {
    for (int dx = 0; dx < OB_W; ++dx)
        for (int dy = 0; dy < OB_H; ++dy)
            ST7565_DrawPixel(x + dx, GROUND_Y + DINO_H - OB_H + dy, 1);
}

// 画地面
void draw_ground() {
    for (int i = 0; i < LCD_W; ++i)
        ST7565_DrawPixel(i, GROUND_Y + DINO_H, 1);
}

// 画分数（用简单竖线）
void draw_score(int sc) {
    for (int i = 0; i < sc && i < 20; ++i)
        ST7565_DrawPixel(120， 5 + i*2， 1);
}

// 显示Game Over
void draw_gameover(int sc) {
    ST7565_Clear();
    // "GAME OVER" 简单大字体：用块
    for(int x=30;x<98;x++)
        for(int y=24;y<40;y+=6)
            ST7565_DrawPixel(x, y, 1);
    // 分数
    for (int i = 0; i < sc && i < 20; ++i)
        ST7565_DrawPixel(64+i*2, 50, 1);
    ST7565_BlitFullScreen();
    SYSTEM_DelayMs(2000);
}

void dino_reset() {
    dino.y = GROUND_Y;
    dino.vy = 0;
    dino.jumping = false;
    obs.x = LCD_W + 40;
    obs.active = true;
    score = 0;
    gameover = false;
}

void DOPPLER_Main(void) {
    dino_reset();

    while (!gameover) {
        // 1. 处理输入
        if (KEYBOARD_Poll() == KEY_PTT) {
            if (!dino.jumping && dino.y == GROUND_Y) {
                dino.vy = JUMP_VY;
                dino.jumping = true;
            }
        }

        // 2. 更新恐龙
        dino.y += dino.vy;
        dino.vy += GRAVITY;
        if (dino.y >= GROUND_Y) {
            dino.y = GROUND_Y;
            dino.vy = 0;
            dino.jumping = false;
        }

        // 3. 更新障碍物
        obs.x -= 4;
        if (obs.x + OB_W < 0) {
            obs.x = LCD_W + (rand() % OB_GAP);
            score++;
        }

        // 4. 碰撞检测
        if (obs.x < DINO_X + DINO_W &&
            obs.x + OB_W > DINO_X &&
            dino.y + DINO_H > GROUND_Y + DINO_H - OB_H) {
            gameover = true;
        }

        // 5. 绘制
        ST7565_Clear();
        draw_ground();
        draw_dino(DINO_X, dino.y);
        draw_obstacle(obs.x);
        draw_score(score);
        ST7565_BlitFullScreen();

        // 6. 延时
        SYSTEM_DelayMs(FRAME_MS);
    }

    // Game Over画面
    draw_gameover(score);
}
