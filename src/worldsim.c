#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <worldsim.h>
#include <render.h>
#include <ascii.h>
#include <debug/rdebug.h>

sprite_t g_wSprites[WORLD_MAX_SPRITES];
u8 g_wSpriteIdx = 0;

pipepair_t g_wPipes[4];
u8 g_wPipesIdx = 0;

f32 g_wScrollDistance;

bool g_wUpdraft = 0;
bool g_wShowHitboxes = 0;
bool g_wGodMode = 0;

bool g_wAsciiMode = 0;
bool g_wUpdraftAnim = 0;

u32 g_wTextColor = COLOR_BLACK;
u32 g_wBackgroundColor = COLOR_AZURE;

asciiobj_t* g_wAsciiBird = NULL;
asciiobj_t* g_wAsciiPipeHeadTop = NULL;
asciiobj_t* g_wAsciiPipeHeadBot = NULL;
asciiobj_t* g_wAsciiPipeSection = NULL;

vec2f_t g_wBirdLocal[16] = {
    {  2,  33}, { 14,  -5}, { 31,  -5}, { 47,   2},
    {-12,   9}, {  3,  10}, { 32,   6}, { 55,  18},
    { 33,  10}, { 18,  32}, { 33,  32}, { 49,  32},
    { 46,  18}, { 17,  11}, { 17,  28}, { 34,  28}
};

vec2f_t g_wPipeHeadTopLocal[37] = {
    {  0, -14}, { 16, -14}, { 32, -14}, { 48, -14}, { 64, -14},             // top lines
    { 42,  12}, { 58,  12}, { 42,  28}, { 58,  28},                         // top pattern r
    {  1,  -2}, { 16,  12}, { 16,  28},                                     // top pattern l
    { 28,  12}, { 28,  28}, {  2,  28}, {  2,  17},                         // top fill mid
    {  3,   0}, { 22,   0}, { 42,   0}, { 61,   0},                         // top fill top
    { 66,  16}, { 66,  25}, { 66,  34},                                     // top fill r
    {-10,   1}, { 72,   1}, {-10,  17}, { 72,  17}, {-10,  33}, { 72,  33}, // side lines
    {  0,  45}, { 16,  41}, { 32,  41}, { 48,  41}, { 62,  45},             // taper down
    { 14,  41}, { 32,  41}, { 50,  41},                                     // taper down fill
};

vec2f_t g_wPipeHeadBotLocal[39] = {
    { 14, -23}, { 32, -23}, { 50, -23},                                     // taper down fill
    {  0,  -5}, { 16,  -7}, { 32,  -7}, { 48,  -7}, { 62,  -5},             // taper down
    {-10,  33}, { 72,  33}, {-10,  17}, { 72,  17}, {-10,   1}, { 72,   1}, // side lines
    { 66,  10}, { 66,  20}, { 66,  30},                                     // top fill r
    {  3,  34}, { 22,  34}, { 42,  34}, { 61,  34},                         // top fill top
    { 28,  22}, { 28,   6}, {  2,   6}, {  2,  25},                         // top fill mid
    {  1,  20}, { 16,  22}, { 16,   6},                                     // top pattern l
    { 42,  22}, { 58,  22}, { 42,   6}, { 58,   6},                         // top pattern r
    {  0,  32}, { 16,  32}, { 32,  32}, { 48,  32}, { 64,  32},             // top lines
    {  0, -12}, { 62, -12},
};

vec2f_t g_wPipeSectionLocal[9] = {
    {  0,  61}, { 10,  55}, { 24,  61}, { 34,  61}, { 47,  61}, { 62,  61}, // section pattern
    { 12,  67}, { 56,  63}, { 56,  69},                                     // section fill bot
};

static inline f32 randRange(i32 lbound, i32 ubound)
{
    rAssert(ubound > lbound);

    return (f32) (rand() % (lbound - ubound)) + lbound;
}

sprite_t* addSprite(u32 type, u16 width, u16 height, f32 xpos, f32 ypos)
{
    if (g_wSpriteIdx >= WORLD_MAX_SPRITES)
        return NULL;

    rAssert(width);
    rAssert(height);

    sprite_t* sprite = g_wSprites + g_wSpriteIdx++;

    sprite->spriteType = type;
    sprite->width = width;
    sprite->height = height;
    sprite->xpos = xpos;
    sprite->ypos = ypos;

    return sprite;
}

pipepair_t* addPipePair(f32 xpos)
{
    if (g_wPipesIdx >= 4)
        return NULL;

    pipepair_t* pair = g_wPipes + g_wPipesIdx++;

    pair->top = addSprite(SPRITE_PIPE, WORLD_STD_PIPE_WIDTH, 500, xpos, 0.0f);
    pair->bot = addSprite(SPRITE_PIPE, WORLD_STD_PIPE_WIDTH, 500, xpos, 0.0f);

    rAssert(pair->top && pair->bot);

    randomizePair(pair, 0);

    return pair;
}

void moveSprite(sprite_t* sprite, f32 dx, f32 dy)
{
    if (!sprite)
        return;

    if (dx > EPSILON || dx < -EPSILON)
        sprite->xpos += dx;

    if (dy > EPSILON || dy < -EPSILON)
        sprite->ypos += dy;
}

void inputUpdraft(void)
{
    g_wUpdraft = 1;
    g_wUpdraftAnim = 1;
}

void toggleHitboxes(void)
{
    g_wShowHitboxes = !g_wShowHitboxes;
}

void toggleAscii(void)
{
    g_wAsciiMode = !g_wAsciiMode;

    if (g_wAsciiMode) {
        setTextureColor(TEXTURE_CLOUD, COLOR_D_GRAY);
        g_wBackgroundColor = COLOR_BLACK;
        g_wTextColor = COLOR_WHITE;
    } else {    
        setTextureColor(TEXTURE_CLOUD, COLOR_WHITE);
        g_wBackgroundColor = COLOR_AZURE;
        g_wTextColor = COLOR_BLACK;
    }
}

void toggleGodMode(void)
{
    g_wGodMode = !g_wGodMode;
}

void initWorld(void)
{
    memset(g_wSprites, 0, sizeof(g_wSprites));
    memset(g_wPipes, 0, sizeof(g_wPipes));

    g_wSpriteIdx = 0;
    g_wPipesIdx = 0;
    g_wScrollDistance = WORLD_STD_SCROLL_V;

    (void) addSprite(SPRITE_BIRD, 64, 48, WORLD_STD_BIRD_XPOS, WORLD_STD_BIRD_YPOS);

    (void) addPipePair(WORLD_STD_FIRST_PIPE_D);
    (void) addPipePair(WORLD_STD_FIRST_PIPE_D + WORLD_STD_PIPE_DISTANCE);
    (void) addPipePair(WORLD_STD_FIRST_PIPE_D + (WORLD_STD_PIPE_DISTANCE * 2));

    u32 birdChars[16] = {
        '\\', '-', '-', '\\',
        '(', '@', 'O', '>',
        '\'', '_', '_', '/',
        '>', ')', 'B', 'D'};

    u32 birdColors[16] = {
        COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
        COLOR_WHITE, COLOR_GOLD, COLOR_WHITE, COLOR_RED,
        COLOR_BLACK, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
        COLOR_RED, COLOR_WHITE, COLOR_GOLD, COLOR_GOLD};

    u32 pipeHeadChars[37] = {
        '_', '_', '_', '_', '_',            // top lines
        '#', 'E', '#', 'E',                 // top pattern r
        '_', ']', ']',                      // top pattern l
        '!', '!', '#', '\"',                // top fill mid
        '=', '=', '=', '=',                 // top fill top
        '`', '`', '`',                      // top fill r
        '|', '|', '|', '|', '|', '|',       // side lines
        'T', '=', '=', '=', 'T',            // taper down
        '_', '_', '_',                      // taper down fill
    };

    u32 pipeHeadColors[37] = {
        COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN,
        COLOR_D_GREEN, COLOR_D_GREEN, COLOR_D_GREEN, COLOR_D_GREEN,
        COLOR_D_GREEN, COLOR_D_GREEN, COLOR_D_GREEN,
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
        COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN,
        COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN,
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
    };

    u32 pipeHeadBotChars[39] = {
        '_', '_', '_',                      // taper down fill
        '-', '=', '=', '=', '-',            // taper down
        '|', '|', '|', '|', '|', '|',       // side lines
        '`', '`', '`',                      // top fill r
        '=', '=', '=', '=',                 // top fill top
        '!', '!', '#', '\"',                // top fill mid
        '_', ']', ']',                      // top pattern l
        '#', 'E', '#', 'E',                 // top pattern r
        '_', '_', '_', '_', '_',            // top lines
        '|', '|',
    };

    u32 pipeHeadBotColors[39] = {
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
        COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN,
        COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN,
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
        COLOR_D_GREEN, COLOR_D_GREEN, COLOR_D_GREEN,
        COLOR_D_GREEN, COLOR_D_GREEN, COLOR_D_GREEN, COLOR_D_GREEN,
        COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN, COLOR_L_GREEN,
        COLOR_L_GREEN, COLOR_L_GREEN,
    };

    u32 pipeSectionChars[9] = {
        '|', '-', ']', '!', 'E', '|',       // section pattern
        '\"', '`', '`',                     // section fill top
    };

    u32 pipeSectionColors[9] = {
        COLOR_L_GREEN, COLOR_D_GREEN, COLOR_D_GREEN, COLOR_M_GREEN, COLOR_D_GREEN, COLOR_L_GREEN,
        COLOR_M_GREEN, COLOR_M_GREEN, COLOR_M_GREEN,
    };

    g_wAsciiBird = asciiObject2D(g_wBirdLocal, birdColors, birdChars, 16);
    g_wAsciiPipeHeadTop = asciiObject2D(g_wPipeHeadTopLocal, pipeHeadColors, pipeHeadChars, 37);
    g_wAsciiPipeHeadBot = asciiObject2D(g_wPipeHeadBotLocal, pipeHeadBotColors, pipeHeadBotChars, 39);
    g_wAsciiPipeSection = asciiObject2D(g_wPipeSectionLocal, pipeSectionColors, pipeSectionChars, 9);

    srand(time(NULL));
}

u32 updateWorld(u64 dt)
{
    static u16 scrollTimer = 0;
    static u16 speedupTimer = 0;
    static u32 score = 100;
    static sprite_t* bird = NULL;

    if (!dt) {
        scrollTimer = 0;
        speedupTimer = 0;
        score = 100;
        bird = NULL;
    }

    if (!bird)
        bird = getBird();

    rAssert(bird);

    scrollScreen(g_wScrollDistance * ((f32) dt / 1000));

    if ((speedupTimer += dt) >= WORLD_STD_SPEEDUP_INTERVAL) {
        g_wScrollDistance -= 5.0f;
        score += 100;
        speedupTimer = 0;
    }

    handleBirdVerticalSpeed(bird, dt, g_wUpdraft);

    g_wUpdraft = 0;

    if (checkCollision(bird) && !g_wGodMode) {
        clearScreen(COLOR_BLACK);
        return score;
    }

    if (g_wAsciiMode)
        handleAnimation(dt);

    clearScreen(g_wBackgroundColor);

    renderClouds(dt);
    renderPipes();
    renderBird(bird);
    renderStrColorFmt(23, 23, 0.25f, g_wTextColor, "Score: %5ld", score);
    renderStrColorFmt(1070, 23, 0.25f, g_wTextColor, "FPS: %.2f", (f32) 1000 / dt);

    return GAME_CONTINUE;
}

void handleAnimation(u64 dt)
{
    static u32 counter = 0;

    if (g_wUpdraftAnim && !counter) {
        setUpdraftAnimation();
        counter = 1;
    }

    if (g_wUpdraftAnim) {
        if ((counter += dt) >= 350) {
            resetUpdraftAnimation();
            g_wUpdraftAnim = 0;
            counter = 0;
        }
    }
}

void setUpdraftAnimation(void)
{
    g_wBirdLocal[4].y = 13.0f;
    g_wBirdLocal[5].y = 12.0f;
}

void resetUpdraftAnimation(void)
{
    g_wBirdLocal[4].y = 9.0f;
    g_wBirdLocal[5].y = 10.0f;
}

void renderClouds(u64 dt)
{
    static f32 xpos[3] = {100, 560, 1000};

    for (u8 i = 0; i < 3; i++) {
        if ((xpos[i] -= ((f32) dt / 7)) < -192.0f)
            xpos[i] = (f32) WINDOW_WIDTH;
    }

    renderTexture(roundf(xpos[0]), 160, 3, TEXTURE_CLOUD);
    renderTextureFlip(roundf(xpos[1]), 110, 3, 0, 1, TEXTURE_CLOUD);
    renderTexture(roundf(xpos[2]), 170, 3, TEXTURE_CLOUD);
}

void renderPipes(void)
{
    sprite_t* tmp;

    for (u8 i = 0; i < g_wSpriteIdx; i++) {
        tmp = g_wSprites + i;

        rAssert(tmp);

        if (tmp->spriteType != SPRITE_PIPE)
            continue;

        if (g_wAsciiMode)
        {
            if (tmp->ypos < WINDOW_HEIGHT / 2) {
                setPosAsciiObject2D(g_wAsciiPipeHeadBot, g_wPipeHeadBotLocal, tmp->xpos, tmp->ypos + tmp->height - 48.0f);
                setPosAsciiObject2D(g_wAsciiPipeSection, g_wPipeSectionLocal, tmp->xpos, tmp->ypos + tmp->height - 137.0f);

                renderAsciiPartial(g_wAsciiPipeHeadBot);

                i8 numSections = (i8) roundf((tmp->ypos + tmp->height - 126.0f) / 16) + 5;

                for (i8 k = 0; k < numSections; k++)
                    renderAsciiPartialOffset2D(g_wAsciiPipeSection, 0.0f, k * -16.0f);
            } else {
                setPosAsciiObject2D(g_wAsciiPipeHeadTop, g_wPipeHeadTopLocal, tmp->xpos, tmp->ypos);
                setPosAsciiObject2D(g_wAsciiPipeSection, g_wPipeSectionLocal, tmp->xpos, tmp->ypos);

                renderAsciiPartial(g_wAsciiPipeHeadTop);

                i8 numSections = (i8) roundf((WINDOW_HEIGHT - tmp->ypos) / 16) - 3;

                for (i8 k = 0; k < numSections; k++)
                    renderAsciiPartialOffset2D(g_wAsciiPipeSection, 0.0f, k * 16.0f);
            }
        }
        else
        {
            if (tmp->ypos < WINDOW_HEIGHT / 2)
                renderTextureFlip(roundf(tmp->xpos), roundf(tmp->ypos - tmp->height), 4, 1, 0, TEXTURE_PIPE);
            else
                renderTexture(roundf(tmp->xpos), roundf(tmp->ypos), 4, TEXTURE_PIPE);
        }

        if (g_wShowHitboxes)
            renderHitbox(tmp->xpos, tmp->ypos, tmp->width, tmp->height);
    }
}

void renderBird(sprite_t* bird)
{
    rAssert(bird);
    rAssert(bird->spriteType == SPRITE_BIRD);

    if (g_wAsciiMode)
    {
        renderAsciiPartialOffset2D(g_wAsciiBird, bird->xpos, bird->ypos);
    }
    else
    {
        renderTexture((i16) roundf(bird->xpos), (i16) roundf(bird->ypos), 4, TEXTURE_BIRD);
    }

    if (g_wShowHitboxes)
        renderHitbox(bird->xpos, bird->ypos, bird->width, bird->height);
}

sprite_t* getBird(void)
{
    for (u8 i = 0; i < g_wSpriteIdx; i++) {
        if (g_wSprites[i].spriteType == SPRITE_BIRD)
            return g_wSprites + i;
    }

    return NULL;
}

void scrollScreen(f32 dx)
{
    pipepair_t* tmp;

    for (u8 i = 0; i < g_wPipesIdx; i++) {
        tmp = g_wPipes + i;

        rAssert(tmp->top->spriteType == SPRITE_PIPE);
        rAssert(tmp->bot->spriteType == SPRITE_PIPE);

        if (tmp->top->xpos < -210.0f)
            randomizePair(tmp, 1);

        moveSprite(tmp->top, dx, 0.0f);
        moveSprite(tmp->bot, dx, 0.0f);
    }
}

void randomizePair(pipepair_t* pair, bool resetXPos)
{
    rAssert(pair);
    rAssert(pair->top);
    rAssert(pair->bot);

    if (resetXPos) {
        pair->top->xpos = WINDOW_WIDTH + 1.0f;
        pair->bot->xpos = WINDOW_WIDTH + 1.0f;
    }

    f32 gap = randRange(180, 280);

    pair->bot->ypos = randRange(WINDOW_HEIGHT / 2, WINDOW_HEIGHT - 70);
    pair->top->ypos = pair->bot->ypos - pair->top->height - gap;
}

bool checkCollision(sprite_t* bird)
{
    if (!bird)
        bird = getBird();

    rAssert(bird);
    rAssert(bird->spriteType == SPRITE_BIRD);
    rAssert(bird->xpos == WORLD_STD_BIRD_XPOS);

    sprite_t* tmp;

    for (u8 i = 0; i < g_wSpriteIdx; i++) {
        tmp = g_wSprites + i;

        if (tmp->spriteType != SPRITE_PIPE)
            continue;

        // check for horizontal collision
        if (tmp->xpos > WORLD_STD_BIRD_XPOS + bird->width || tmp->xpos + tmp->width < WORLD_STD_BIRD_XPOS)
            continue;

        // check for vertical collision
        if (tmp->ypos + tmp->height < bird->ypos || tmp->ypos > bird->ypos + bird->height)
            continue;

        // found collision
        return 1;
    }

    // no collision found
    return 0;
}

void handleBirdVerticalSpeed(sprite_t* bird, u64 dt, bool updraft)
{
    static f32 dy = 0.0f;

    if (!dt) {
        dy = 0.0f;
        return;
    }

    if (!bird)
        bird = getBird();

    rAssert(bird);
    rAssert(bird->spriteType == SPRITE_BIRD);

    if (updraft)
        dy = WORLD_STD_UPDRAFT_V;
    else if (dy < 0.0f)
        dy += 3 * WORLD_STD_GRAVITY_DV * (f32) dt / 1000;
    else
        dy += WORLD_STD_GRAVITY_DV * (f32) dt / 1000;

    bird->ypos += dy * (f32) dt / 1000;
}
