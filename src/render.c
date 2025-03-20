#include <SDL3/SDL.h>

#include <render.h>

void setColor(u32 color)
{
    SDL_SetRenderDrawColor(renderer, color >> 24, color >> 16, color >> 8, 255);
}

void renderRectangleColor(f32 xpos, f32 ypos, f32 width, f32 height, u32 color)
{
    setColor(color);

    renderRectangle(xpos, ypos, width, height);
}

void renderRectangle(f32 xpos, f32 ypos, f32 width, f32 height)
{
    SDL_FRect rect = {xpos, ypos, width, height};

    SDL_RenderFillRect(renderer, &rect);
}