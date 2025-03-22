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

vec2f_t g_wBirdLocal[16] = {
    {2.0f, 33.0f}, {14.0f, -5.0f}, {31.0f, -5.0f}, {47.0f, 2.0f},
    {-8.0f, 9.0f}, {5.0f, 10.0f}, {32.0f, 6.0f}, {55.0f, 18.0f},
    {33.0f, 10.0f}, {18.0f, 32.0f}, {33.0f, 32.0f}, {49.0f, 32.0f},
    {46.0f, 18.0f}, {17.0f, 11.0f}, {17.0f, 28.0f}, {34.0f, 28.0f}
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
        g_wAsciiMode = 1;
    } else {    
        setTextureColor(TEXTURE_CLOUD, COLOR_WHITE);
        g_wBackgroundColor = COLOR_AZURE;
        g_wTextColor = COLOR_BLACK;
        g_wAsciiMode = 0;
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

    u32 chars[16] = {
        '\\', '-', '-', '\\',
        '(', '@', 'O', '>',
        '\'', '_', '_', '/',
        '>', ')', 'B', 'D'};

    u32 colors[16] = {
        COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
        COLOR_WHITE, COLOR_GOLD, COLOR_WHITE, COLOR_RED,
        COLOR_BLACK, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
        COLOR_RED, COLOR_WHITE, COLOR_GOLD, COLOR_GOLD};

    g_wAsciiBird = asciiObject2D(g_wBirdLocal, colors, chars, 16);

    srand(time(NULL));
}

u32 updateWorld(u64 dt)
{
    static u16 scrollTimer = 0;
    static u16 speedupTimer = 0;
    static u32 score = 100;
    static sprite_t* bird = NULL;

    static u16 animCounter = 0;

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

        if (tmp->ypos < WINDOW_HEIGHT / 2)
            renderTextureFlip(roundf(tmp->xpos), roundf(tmp->ypos - tmp->height), 4, 1, 0, TEXTURE_PIPE);
        else
            renderTexture(roundf(tmp->xpos), roundf(tmp->ypos), 4, TEXTURE_PIPE);

        if (g_wShowHitboxes)
            renderHitbox(tmp->xpos, tmp->ypos, tmp->width, tmp->height);
    }
}

void renderBird(sprite_t* bird)
{
    rAssert(bird);
    rAssert(bird->spriteType == SPRITE_BIRD);

    if (g_wAsciiMode) {
        setPosAsciiObject2D(g_wAsciiBird, g_wBirdLocal, bird->xpos, bird->ypos, 16);
        renderAscii(0, 0);
    } else {
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
