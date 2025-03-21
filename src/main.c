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
        SDL_SetRenderScale(g_renderer, 1.0f, 1.0f);
        break;
    }

    return SDL_APP_CONTINUE;
}

//
//  Init
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

    ticksPerSecond = SDL_GetPerformanceFrequency();

    initWorld();

    return SDL_APP_CONTINUE;
}

//
//  Event
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
//  Update
//

SDL_AppResult SDL_AppIterate(void* appstate)
{
    static u32 score = 0;

    switch (state) {
    case 2:
        clearScreen(COLOR_WWHITE);

        setColor(COLOR_OLIVE);

        SDL_SetRenderScale(g_renderer, 12.0f, 12.0f);
        SDL_RenderDebugText(g_renderer, 5.33f, 16.0f, "FLAPPY BIRD");

        setColor(COLOR_PURPLE);

        SDL_SetRenderScale(g_renderer, 4.0f, 4.0f);
        SDL_RenderDebugText(g_renderer, 16.0f, 100.0f, "- press [ENTER] to start! -");

        SDL_RenderPresent(g_renderer);

        SDL_SetRenderScale(g_renderer, 1.0f, 1.0f);

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
        clearScreen(COLOR_WWHITE);

        setColor(COLOR_OLIVE);

        SDL_SetRenderScale(g_renderer, 16.0f, 16.0f);
        SDL_RenderDebugText(g_renderer, 4.0f, 10.0f, "GAME OVER");

        setColor(COLOR_PURPLE);

        SDL_SetRenderScale(g_renderer, 4.0f, 4.0f);
        SDL_RenderDebugTextFormat(g_renderer, 16.0f, 96.0f, "Score: %6ld", score);

        setColor(COLOR_OLIVE);

        SDL_SetRenderScale(g_renderer, 2.0f, 2.0f);
        SDL_RenderDebugText(g_renderer, 32.0f, 252.0f, "- press [R] to reset -");
        SDL_RenderDebugText(g_renderer, 32.0f, 272.0f, "- press [ENTER] to exit -");

        SDL_RenderPresent(g_renderer);

        break;
    }

    return SDL_APP_CONTINUE;
}

//
//  Exit
//
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    cleanupRenderer();
}