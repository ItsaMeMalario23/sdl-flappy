#ifndef RENDER_H
#define RENDER_H

#include <main.h>

#define COLOR_RED       0xff000000
#define COLOR_GREEN     0x00ff0000
#define COLOR_BLUE      0x0000ff00
#define COLOR_YELLOW    0xfce80300
#define COLOR_GOLD      0xfcd30300
#define COLOR_PURPLE    0xa103fc00
#define COLOR_AZURE     0x03ecfc00
#define COLOR_OLIVE     0x01750500
#define COLOR_BLACK     0x00000000
#define COLOR_WHITE     0xffffff00
#define COLOR_WWHITE    0xe0e0ce00

#define COLOR_HITBOXES  0xff000000  // red

#define MAX_TEXTURES    8

#define TEXTURE_LOGO    0
#define TEXTURE_BIRD    1
#define TEXTURE_PIPE    2
#define TEXTURE_CLOUD   3

void initRenderer(void);
void cleanupRenderer(void);

void clearScreen(u32 color);
void setColor(u32 color);

void renderRectangleColor(f32 xpos, f32 ypos, f32 width, f32 height, u32 color);
void renderRectangle(f32 xpos, f32 ypos, f32 width, f32 height);
void renderHitbox(f32 xpos, f32 ypos, f32 width, f32 height);

bool loadTextures(const texinfo_t* textures, u32 numTextures);
bool loadTexture(const char* path, bool noInterpolation, u8 textureID);

void renderTexture(i16 xpos, i16 ypos, u16 scale, u16 textureID);
void renderTextureFlip(i16 xpos, i16 ypos, u8 scale, bool vFlip, bool hFlip, u8 textureID);
void renderTextureRotate(i16 xpos, i16 ypos, u16 rotation, u8 scale, u8 textureID);

#endif