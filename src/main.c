#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <main.h>
#include <render.h>
#include <worldsim.h>

static SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

u64 ticksPerSecond;

SDL_AppResult handleInput(SDL_Keycode input)
{
    if (input < 128)
        SDL_Log("Input: %c (%d)", (char) input, (char) input);
    else
        SDL_Log("Input: %ld", input);

    switch (input) {
    case SDLK_ESCAPE:
        return SDL_APP_SUCCESS;
    case SDLK_SPACE:
    case SDLK_UP:
    case SDLK_W:
        inputUpdraft();
        break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialized SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("SDL-FLAPPY", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS, &window, &renderer)) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    ticksPerSecond = SDL_GetPerformanceFrequency();

    initWorld();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;
    else if (event->type == SDL_EVENT_KEY_DOWN)
        return handleInput(event->key.key);

    return SDL_APP_CONTINUE;
}

u64 currt;
u64 prevt = 0;

SDL_AppResult SDL_AppIterate(void* appstate)
{
    while (prevt && ((SDL_GetPerformanceCounter() * 1000) / ticksPerSecond) - prevt < FIXED_FRAMTIME);

    if (!prevt) {
        prevt = (SDL_GetPerformanceCounter() * 1000) / ticksPerSecond;

        updateWorld(0);

        return SDL_APP_CONTINUE;
    }

    currt = (SDL_GetPerformanceCounter() * 1000) / ticksPerSecond;

    if (updateWorld(currt - prevt) != GAME_CONTINUE)
        return SDL_APP_SUCCESS;

    prevt = currt;

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) { }