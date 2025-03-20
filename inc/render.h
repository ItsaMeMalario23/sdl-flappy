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
#define COLOR_WHITE     0xffffff00
#define COLOR_BLACK     0x00000000

void setColor(u32 color);
void renderRectangleColor(f32 xpos, f32 ypos, f32 width, f32 height, u32 color);
void renderRectangle(f32 xpos, f32 ypos, f32 width, f32 height);

#endif