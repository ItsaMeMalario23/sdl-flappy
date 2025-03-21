#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <main.h>
#include <render.h>
#include <worldsim.h>

static SDL_Window* window = NULL;

SDL_Renderer* g_renderer = NULL;

u64 ticksPerSecond;
u64 currt;
u64 prevt = 0;

u8 state = 2;

const texinfo_t textures[3] = {
    {"..\\resources\\bird.bmp", TEXTURE_BIRD, INTERPOLATION_NONE},
    {"..\\resources\\pipe.bmp", TEXTURE_PIPE, INTERPOLATION_NONE},
    {"..\\resources\\cloud.bmp", TEXTURE_CLOUD, INTERPOLATION_NONE}
};

//
//  Start screen
//
void startScreen(void)
{
    // wait on spin
    while (prevt && ((SDL_GetPerformanceCounter() * 1000) / ticksPerSecond) - prevt < MENU_FRAMETIME);

    if (!prevt)
        prevt = (SDL_GetPerformanceCounter() * 1000) / ticksPerSecond;

    currt = (SDL_GetPerformanceCounter() * 1000) / ticksPerSecond;

    clearScreen(COLOR_BLACK);

    const char* title = "FLAPPY BIRD";
    static const i16 ypos = (WINDOW_WIDTH / 2) - 563;
    static i8 idx = -1;
    static u64 counter = 160;

    if (++counter > 240) {
        if (counter % 2 == 0)
            idx++;

        if (idx > 10) {
            counter = 0;
            idx = -1;
        }
    }

    for (u8 i = 0; i < 11; i++) {
        if (i == idx)
            renderCharColor(ypos + (i * 64 * 1.6f), 200, 1.6f, COLOR_WHITE, title[i]);
        else
            renderCharColor(ypos + (i * 64 * 1.6f), 200, 1.6f, COLOR_GOLD, title[i]);
    }

    renderStrColorCentered(480, 0.5f, COLOR_PURPLE, "- press [ENTER] to start! -");

    SDL_RenderPresent(g_renderer);

    prevt = currt;
}

//
//  Game over screen
//
void gameoverScreen(u32 score)
{
    // wait on spin
    while (prevt && ((SDL_GetPerformanceCounter() * 1000) / ticksPerSecond) - prevt < MENU_FRAMETIME);

    if (!prevt)
        prevt = (SDL_GetPerformanceCounter() * 1000) / ticksPerSecond;

    currt = (SDL_GetPerformanceCounter() * 1000) / ticksPerSecond;

    clearScreen(COLOR_BLACK);

    const char* str = "GAME OVER";
    static const i16 ypos = (WINDOW_WIDTH / 2) - 576;
    static i8 idx = -1;
    static u64 counter = 160;

    if (++counter > 240) {
        if (counter % 2 == 0)
            idx++;

        if (idx > 10) {
            counter = 0;
            idx = -1;
        }
    }

    for (u8 i = 0; i < 9; i++) {
        if (i == idx)
            renderCharColor(ypos + (i * 128), 160, 2.0f, COLOR_WHITE, str[i]);
        else
            renderCharColor(ypos + (i * 128), 160, 2.0f, COLOR_GOLD, str[i]);
    }

    renderStrColorFmtCentered(384, 1.0f, COLOR_PURPLE, "Score: %5ld", score);
    renderStrColorCentered(544, 0.25f, COLOR_BLUE, "- press [ENTER] to exit -");
    renderStrColorCentered(584, 0.25f, COLOR_BLUE, "- press [R] to reset -");

    SDL_RenderPresent(g_renderer);

    prevt = currt;
}

SDL_AppResult handleInput(SDL_Keycode input)
{
    if (input < 128)
        SDL_Log("Input: %c (%d)", (char) input, (char) input);
    else
        SDL_Log("Input: %ld", input);

    switch (input) {
    case SDLK_ESCAPE:
        return SDL_APP_SUCCESS;

    case SDLK_RETURN:
        if (!state)
            return SDL_APP_SUCCESS;
        else if (state == 2)
            state = 1;

        break;

    case SDLK_SPACE:
    case SDLK_UP:
    case SDLK_W:
        if (state == 1)
            inputUpdraft();
        break;

    case SDLK_H:
        toggleHitboxes();
        break;

    case SDLK_R:
        state = 1;
        prevt = 0;
        initWorld();
        break;
    }

    return SDL_APP_CONTINUE;
}

//
//  SDL Init
//
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialized SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("SDL-FLAPPY", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS, &window, &g_renderer)) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    initRenderer();

    if (!loadTextures(textures, 3))
        return SDL_APP_FAILURE;

    if (!loadCharTextures("..\\resources\\ascii\\pressstart\\", 95))
        return SDL_APP_FAILURE;

    ticksPerSecond = SDL_GetPerformanceFrequency();

    initWorld();

    return SDL_APP_CONTINUE;
}

//
//  SDL Event
//
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;
    else if (event->type == SDL_EVENT_KEY_DOWN)
        return handleInput(event->key.key);

    return SDL_APP_CONTINUE;
}

//
//  SDL Update
//
SDL_AppResult SDL_AppIterate(void* appstate)
{
    static u32 score = 0;

    switch (state) {
    case 2:
        startScreen();
        break;
    
    case 1:
        while (prevt && ((SDL_GetPerformanceCounter() * 1000) / ticksPerSecond) - prevt < FIXED_FRAMTIME);

        if (!prevt) {
            prevt = (SDL_GetPerformanceCounter() * 1000) / ticksPerSecond;

            clearScreen(COLOR_AZURE);
            updateWorld(0);
            SDL_RenderPresent(g_renderer);

            return SDL_APP_CONTINUE;
        }

        currt = (SDL_GetPerformanceCounter() * 1000) / ticksPerSecond;

        clearScreen(COLOR_AZURE);

        if ((score = updateWorld(currt - prevt)) != GAME_CONTINUE)
            state = 0;

        SDL_RenderPresent(g_renderer);

        prevt = currt;

        break;

    default:
        gameoverScreen(score);
        break;
    }

    return SDL_APP_CONTINUE;
}

//
//  SDL Exit
//
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    cleanupRenderer();
}