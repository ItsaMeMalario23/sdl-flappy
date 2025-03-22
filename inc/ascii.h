#ifndef ASCII_H
#define ASCII_H

#include <main.h>

//
//  Constants
//
#define RENDER_MAX_CHARS    256
#define RENDER_MAX_OBJECTS  64

#define RENDER_MODE_2D      0
#define RENDER_MODE_3D      1
#define RENDER_MODE_UNINIT  2

// ascii object data types
#define OBJ_TYPE_2D         RENDER_MODE_2D
#define OBJ_TYPE_3D         RENDER_MODE_3D

//
//  Typedefs
//
typedef i32 char2Idx;
typedef i32 char3Idx;
typedef struct pageinfo_s pageinfo_t;

// ascii char in 2D space
typedef struct ascii2_s {
    f32 xpos;
    f32 ypos;
    u32 color;
    u32 charID;
} ascii2_t;

// ascii char in 3D space
typedef struct ascii3_s {
    f32 xpos;
    f32 ypos;
    f32 zpos;
    u32 color;
    u64 charID;
} ascii3_t;

// struct to group chars into objects
typedef struct asciiobj_s {
    u32 len;
    u16 type;
    u16 paged;

    union {
        ascii2_t* ascii2;
        ascii3_t* ascii3;
    } data;

} asciiobj_t;

// linked list for paged memory
struct pageinfo_s {
    void*       ptr;
    u32         len;
    u32         type;
    pageinfo_t* next;
};

//
//  Public funtions
//
void asciiInit(u32 renderMode);
void asciiCleanup(void);
void asciiResetAll(void);
void asciiChangeMode(u32 renderMode);

void renderAscii(u32 clearScr, u32 backgroundColor);

char2Idx asciiChar2D(f32 xpos, f32 ypos, u32 color, u32 charID);
char3Idx asciiChar3D(f32 xpos, f32 ypos, f32 zpos, u32 color, u64 charID);

asciiobj_t* asciiObject2D(const vec2f_t* positions, const u32* colors, const u32* chars, u64 len);
asciiobj_t* asciiObject3D(const vec3f_t* positions, const u32* colors, const u32* chars, u64 len);

void moveAsciiChar2D(char2Idx idx, f32 dx, f32 dy);
void moveAsciiChar3D(char3Idx idx, f32 dx, f32 dy, f32 dz);
void moveAsciiObject2D(asciiobj_t* object, f32 dx, f32 dy);
void moveAsciiObject3D(asciiobj_t* object, f32 dx, f32 dy, f32 dz);
void setPosAsciiObject2D(asciiobj_t* object, vec2f_t* local, f32 xglobal, f32 yglobal, u64 len);
void setPosAsciiObject3D(asciiobj_t* object, vec3f_t* local, f32 xglobal, f32 yglobal, f32 zglobal, u32 len);

void changeAsciiCharColor2D(char2Idx idx, u32 color);
void changeAsciiCharColor3D(char3Idx idx, u32 color);

void removeAsciiChar2D(char2Idx idx);
void removeAsciiChar3D(char3Idx idx);
void removeAsciiChars2D(ascii2_t* chars, u32 numChars);
void removeAsciiChars3D(ascii3_t* chars, u32 numChars);
void removeAsciiObject(asciiobj_t* object);

//
//  Local functions
//
asciiobj_t* getAsciiObj(void);
bool        getAscii2Mem(ascii2_t** dst, u64 len);
bool        getAscii3Mem(ascii3_t** dst, u64 len);
char2Idx    getChar2(void);
char3Idx    getChar3(void);
void        renderPaged(void);
void        addPage(void* ptr, u32 len, u32 type);
void        removePaged(void* ptr);
void        freePaged(void);

#endif