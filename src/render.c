#include <stdarg.h>
#include <stdio.h>
#include <SDL3/SDL.h>

#include <render.h>
#include <debug/rdebug.h>

texture_t r_textures[MAX_TEXTURES];
SDL_Texture* r_asciitex[MAX_ASCII_TEXTURES];
u32 r_asciicolors[MAX_ASCII_TEXTURES];

// initialize renderer
void initRenderer(void)
{
    memset(r_textures, 0, sizeof(r_textures));
    memset(r_asciitex, 0, sizeof(r_asciitex));
    memset(r_asciicolors, 1, sizeof(r_asciicolors));
}

// cleanup renderer
void cleanupRenderer(void)
{
    for (u8 i = 0; i < MAX_TEXTURES; i++) {
        if (r_textures[i].sdltex)
            SDL_DestroyTexture(r_textures[i].sdltex);
    }

    for (u8 i = 0; i < MAX_ASCII_TEXTURES; i++) {
        if (r_asciitex[i])
            SDL_DestroyTexture(r_asciitex[i]);
    }
}

// clear screen and set background color
void clearScreen(u32 color)
{
    setColor(color);

    SDL_RenderClear(g_renderer);
}

// set color for simple drawing operations
void setColor(u32 color)
{
    rAssert(g_renderer);

    SDL_SetRenderDrawColor(g_renderer, color >> 24, color >> 16, color >> 8, 255);
}

void renderRectangleColor(f32 xpos, f32 ypos, f32 width, f32 height, u32 color)
{
    setColor(color);

    renderRectangle(xpos, ypos, width, height);
}

void renderRectangle(f32 xpos, f32 ypos, f32 width, f32 height)
{
    rAssert(g_renderer);

    SDL_FRect rect = {xpos, ypos, width, height};

    SDL_RenderFillRect(g_renderer, &rect);
}

void renderHitbox(f32 xpos, f32 ypos, f32 width, f32 height)
{
    rAssert(g_renderer);

    setColor(COLOR_HITBOXES);

    SDL_FRect rect = {xpos, ypos, width, height};

    SDL_RenderRect(g_renderer, &rect);
}

// load multiple textures
bool loadTextures(const texinfo_t* textures, u32 numTextures)
{
    rAssert(textures);
    rAssert(numTextures);

    for (u32 i = 0; i < numTextures; i++) {
        if (!loadTexture(textures[i].path, textures[i].interpolation, textures[i].textureID))
            return 0;
    }

    return 1;
}

// load singular texture and assign it to textureID
bool loadTexture(const char* path, bool noInterpolation, u8 textureID)
{
    rAssert(path);
    rAssert(g_renderer);
    rAssert(textureID < MAX_TEXTURES);

    SDL_Surface* surf = NULL;

    char* fpath = NULL;

    SDL_asprintf(&fpath, "%s%s", SDL_GetBasePath(), path);

    surf = SDL_LoadBMP(fpath);

    if (!surf) {
        SDL_Log("Failed to load texture %d: %s", textureID, SDL_GetError());
        return 0;
    }

    SDL_free(fpath);

    SDL_Log("Loaded texture %d: %ldx%ld", textureID, surf->w, surf->h);

    r_textures[textureID].width = surf->w;
    r_textures[textureID].height = surf->h;
    r_textures[textureID].sdltex = SDL_CreateTextureFromSurface(g_renderer, surf);

    if (!r_textures[textureID].sdltex) {
        SDL_Log("Failed to create static texture %d: %s", textureID, SDL_GetError());
        return 0;
    }

    if (noInterpolation)
        SDL_SetTextureScaleMode(r_textures[textureID].sdltex, SDL_SCALEMODE_NEAREST);

    SDL_DestroySurface(surf);

    return 1;
}

bool loadCharTextures(const char* path, u32 numChars)
{
    rAssert(path);

    const char* bpath = SDL_GetBasePath();

    SDL_Surface* surf;

    char pbuf[128];

    for (u32 i = 0; i < numChars; i++) {
        snprintf(pbuf, 128, "%s%s00000%ld.bmp", bpath, path, i + 1);

        surf = SDL_LoadBMP(pbuf);

        if (!surf) {
            SDL_Log("Failed to load char %d: %s", i + 1, SDL_GetError());
            return 0;
        }

        // SDL_Log("Loaded char %d: %ldx%ld", i + 1, surf->w, surf->h);

        r_asciitex[i] = SDL_CreateTextureFromSurface(g_renderer, surf);
        r_asciicolors[i] = COLOR_WHITE;

        if (!r_asciitex[i]) {
            SDL_Log("Failed to create static texture for char %d: %s", i + 1, SDL_GetError());
            return 0;
        }

        SDL_SetTextureScaleMode(r_asciitex[i], SDL_SCALEMODE_NEAREST);

        SDL_DestroySurface(surf);
    }

    return 1;
}

void setTextureColor(u32 textureID, u32 color)
{
    rAssert(textureID < MAX_TEXTURES);
    rAssert(r_textures[textureID].sdltex);

    SDL_SetTextureColorMod(r_textures[textureID].sdltex, color >> 24, color >> 16, color >> 8);
}

// render texture at textureID with scale >= 1
void renderTexture(i16 xpos, i16 ypos, u16 scale, u16 textureID)
{
    rAssert(scale);
    rAssert(g_renderer);
    rAssert(textureID < MAX_TEXTURES);
    rAssert(r_textures[textureID].sdltex);

    SDL_FRect dst = {
        (f32) xpos,
        (f32) ypos,
        (f32) r_textures[textureID].width * scale,
        (f32) r_textures[textureID].height * scale
    };

    SDL_RenderTexture(g_renderer, r_textures[textureID].sdltex, NULL, &dst);
}

// render texture with either vertical or horizontal flip
void renderTextureFlip(i16 xpos, i16 ypos, u8 scale, bool vFlip, bool hFlip, u8 textureID)
{
    rAssert(scale);
    rAssert(g_renderer);
    rAssert(textureID < MAX_TEXTURES);
    rAssert(r_textures[textureID].sdltex);

    SDL_FRect dst = {
        (f32) xpos,
        (f32) ypos,
        (f32) r_textures[textureID].width * scale,
        (f32) r_textures[textureID].height * scale
    };

    SDL_FlipMode flip = vFlip ? SDL_FLIP_VERTICAL : (hFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

    SDL_RenderTextureRotated(g_renderer, r_textures[textureID].sdltex, NULL, &dst, 0.0f, NULL, flip);
}

// render texture with rotation around texture center
void renderTextureRotate(i16 xpos, i16 ypos, u16 rotation, u8 scale, u8 textureID)
{
    rAssert(scale);
    rAssert(g_renderer);
    rAssert(textureID < MAX_TEXTURES);
    rAssert(r_textures[textureID].sdltex);

    SDL_FRect dst = {
        (f32) xpos,
        (f32) ypos,
        (f32) r_textures[textureID].width * scale,
        (f32) r_textures[textureID].height * scale
    };

    SDL_RenderTextureRotated(g_renderer, r_textures[textureID].sdltex, NULL, &dst, (f64) rotation, NULL, SDL_FLIP_NONE);
}

void renderChar(i16 xpos, i16 ypos, f32 scale, char charID)
{
    rAssert(g_renderer);
    rAssert(charID > 31);
    rAssert(charID < 127);
    rAssert(r_asciitex[charID - 32]);

    SDL_FRect dst = {
        (f32) xpos,
        (f32) ypos,
        (f32) 64 * scale,
        (f32) 64 * scale
    };

    if (!SDL_RenderTexture(g_renderer, r_asciitex[charID - 32], NULL, &dst))
        SDL_Log("Failed to render char: %s", SDL_GetError());
}

void renderCharColor(i16 xpos, i16 ypos, f32 scale, u32 color, char charID)
{
    rAssert(charID > 31);
    rAssert(charID < 127);

    // only change color mod if necessary
    if (r_asciicolors[charID - 32] != color) {
        SDL_SetTextureColorMod(r_asciitex[charID - 32], color >> 24, color >> 16, color >> 8);
        r_asciicolors[charID - 32] = color;
    }

    renderChar(xpos, ypos, scale, charID);
}

void renderStr(i16 xpos, i16 ypos, f32 scale, const char* str)
{
    rAssert(str);

    for (u32 i = 0; i < strnlen(str, RENDER_TXT_MAX_LEN); i++) {
        if (str[i] < 33 || str[i] > 125)
            continue;
        
        renderChar(xpos + (i * 64 * scale), ypos, scale, str[i]);
    }
}

void renderStrColor(i32 xpos, i32 ypos, f32 scale, u32 color, const char* str)
{
    rAssert(str);

    for (u32 i = 0; i < strnlen(str, RENDER_TXT_MAX_LEN); i++) {
        if (str[i] < 33 || str[i] > 125)
            continue;

        renderCharColor(xpos + (i * 64 * scale), ypos, scale, color, str[i]);
    }
}

void renderStrColorFmt(i32 xpos, i32 ypos, f32 scale, u32 color, const char* fmt, ... )
{
    va_list args;

    va_start(args, fmt);

    char str[RENDER_TXT_MAX_LEN];

    vsnprintf(str, RENDER_TXT_MAX_LEN, fmt, args);

    str[RENDER_TXT_MAX_LEN - 1] = '\0';

    renderStrColor(xpos, ypos, scale, color, str);

    va_end(args);
}

void renderStrColorCentered(i64 ypos, f32 scale, u32 color, const char* str)
{
    f32 xpos = (WINDOW_WIDTH / 2) - ((strnlen(str, RENDER_TXT_MAX_LEN) * 64 * scale) / 2);

    renderStrColor(xpos, ypos, scale, color, str);
}

void renderStrColorFmtCentered(i64 ypos, f32 scale, u32 color, const char* fmt, ... )
{
    va_list args;

    va_start(args, fmt);

    char str[RENDER_TXT_MAX_LEN];

    f32 xpos = scale * 64 * vsnprintf(str, RENDER_TXT_MAX_LEN, fmt, args);

    xpos = (WINDOW_WIDTH / 2) - (xpos / 2);

    str[RENDER_TXT_MAX_LEN - 1] = '\0';

    renderStrColor(xpos, ypos, scale, color, str);

    va_end(args);
}