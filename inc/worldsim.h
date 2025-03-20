#ifndef WORLDSIM_H
#define WORLDSIM_H

#include <main.h>

#define WORLD_MAX_SPRITES   8

#define GAME_CONTINUE       0

#define SPRITE_UNDEF        0
#define SPRITE_BIRD         1
#define SPRITE_PIPE         2

#define WORLD_STD_GRAVITY_DV        (  400.0f)
#define WORLD_STD_UPDRAFT_V         (- 670.0f)
#define WORLD_STD_UPDRAFT_DAMPING   (    0.93f)
#define WORLD_STD_SCROLL_SPEED      (    2)
#define WORLD_STD_SCROLL_DISTANCE   (-   3.0f)
#define WORLD_STD_BIRD_XPOS         (  200.0f)
#define WORLD_STD_BIRD_YPOS         (  200.0f)
#define WORLD_STD_FIRST_PIPE_D      ( 1000.0f)
#define WORLD_STD_PIPE_DISTANCE     (  500.0f)
#define WORLD_STD_PIPE_WIDTH        (   80)
#define WORLD_STD_SPEEDUP_INTERVAL  (  100)

typedef struct sprite_s {
    u32 spriteType;
    u16 width;
    u16 height;
    f32 xpos;
    f32 ypos;
} sprite_t;

typedef struct pipepair_s {
    sprite_t* top;
    sprite_t* bot;
} pipepair_t;

sprite_t* addSprite(u32 type, u16 width, u16 height, f32 xpos, f32 ypos);
pipepair_t* addPipePair(f32 xpos);

void moveSprite(sprite_t* sprite, f32 dx, f32 dy);
void inputUpdraft(void);

void initWorld(void);
u32  updateWorld(u64 dt);

void renderPipes(void);
void renderBird(sprite_t* bird);

sprite_t* getBird(void);
void scrollScreen(void);
void randomizePair(pipepair_t* pair, bool resetXPos);
bool checkCollision(sprite_t* bird);
void handleBirdVerticalSpeed(sprite_t* bird, u64 dt, bool updraft);

#endif