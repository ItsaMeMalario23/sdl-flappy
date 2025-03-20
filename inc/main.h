#ifndef MAIN_H
#define MAIN_H

#include <SDL3/SDL.h>

#define R_DEBUG

#define WINDOW_WIDTH    1280
#define WINDOW_HEIGHT   720

#define FIXED_FRAMTIME  7

#define EPSILON         (0.0001f)

typedef signed char i8;
typedef unsigned char u8;

typedef signed short int i16;
typedef unsigned short int u16;

typedef signed long int i32;
typedef unsigned long int u32;

typedef signed long long i64;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

//typedef unsigned char bool;

extern SDL_Renderer* renderer;

#endif