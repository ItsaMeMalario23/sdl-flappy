#ifndef ASCII_H
#define ASCII_H

#include <main.h>

//
//  Constants
//
#define ASCII_RENDER_BUF_SIZE   16384   // bytes
#define ASCII_CHAR_BUF_SIZE     8192    // bytes
#define ASCII_OBJ_BUF_SIZE      32      // asciiobj elements

#define ASCII_MAX_2D_CHARS      ((u16) (ASCII_RENDER_BUF_SIZE / sizeof(ascii2_t)))
#define ASCII_MAX_3D_CHARS      ((u16) (ASCII_RENDER_BUF_SIZE / sizeof(ascii3_t)))

#define ASCII_RENDER_MODE_2D    0
#define ASCII_RENDER_MODE_3D    1
#define ASCII_RENDER_UNINIT     2

#define ASCII_RENDER_SCALE      (0.25f)

#define ASCII_OBJ_2D            ASCII_RENDER_MODE_2D
#define ASCII_OBJ_3D            ASCII_RENDER_MODE_3D

//
//  Typedefs
//
typedef i32 char2Idx;
typedef i32 char3Idx;
typedef struct pageinfo_s pageinfo_t;

// ascii char in 2D space
typedef struct ascii2_s {
    f32 xpos;       // local x position of char
    f32 ypos;       // local y position of char
    u32 color;
    u16 visible;
    u16 charID;
} ascii2_t;

// ascii char in 3D space
typedef struct ascii3_s {
    f32 xpos;
    f32 ypos;
    f32 zpos;
    u32 color;
    u32 visible;    // each char has its own visibility flag so renderbuf can be processed directly
    u32 charID;
} ascii3_t;

// info needed to allocate 2D char
typedef struct ascii2info_s {
    u32     charID;
    u32     color;
    vec2f_t pos;
} ascii2info_t;

// info needed to allocate 3D char
typedef struct ascii3info_s {
    u32     charID;
    u32     color;
    vec3f_t pos;
} ascii3info_t;

// struct to group chars into objects
typedef struct asciiobj_s {
    u32 len;
    u32 type;
    u16 visible;
    u16 paged;

    f32 xpos;       // global x offset of object
    f32 ypos;       // global y offset of object
    f32 zpos;       // global z offset of object (only for 3D)

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
void initAscii(u32 renderMode);
void cleanupAscii(void);
void asciiResetAll(void);
void asciiChangeMode(u32 renderMode);

void asciiRenderAll(u32 backgroundColor, u16 clearScr, u16 preserveRenderBuf);
void asciiRender2D(u32 backgroundColor, u16 clearScr, u16 preserveRenderBuf);
void asciiRender3D(u32 backgroundColor, u16 clearScr, u16 preserveRenderBuf);

void renderAsciiObject2D(asciiobj_t* object);
void renderAsciiObject3D(asciiobj_t* object);
void renderAsciiObjectDirect2D(asciiobj_t* object);
void renderAsciiObjectDirect3D(asciiobj_t* object);

asciiobj_t* asciiObject2D(const vec2f_t* positions, const u32* colors, const u32* chars, u64 len);
asciiobj_t* asciiObject3D(const vec3f_t* positions, const u32* colors, const u32* chars, u64 len);
asciiobj_t* asciiObject2DIStruct(const ascii2info_t* info, u64 len);
asciiobj_t* asciiObject3DIStruct(const ascii3info_t* info, u64 len);

asciiobj_t* getAsciiObj(u32 type, u32 len);
bool getAsciiObjMem(void** dst, u32 type, u32 len);
void renderBuf2D(const ascii2_t* buf, u64 len, f32 dx, f32 dy);
void renderBuf3D(const ascii3_t* buf, u32 len, f32 dx, f32 dy, f32 dz);

void memPage(void* ptr, u32 type, u32 len);
void removeMemPage(void* ptr);
void freePages(void);

/*
void initAscii(u32 renderMode);
void asciiCleanup(void);
void asciiResetAll(void);
void asciiChangeMode(u32 renderMode);

void renderAscii(u32 clearScr, u32 backgroundColor);
void renderAsciiPartial(asciiobj_t* object);
void renderAsciiPartialOffset2D(asciiobj_t* object, f32 dx, f32 dy);
void renderAsciiPartialOffset3D(asciiobj_t* object, f32 dx, f32 dy, f32 dz);

char2Idx asciiChar2D(f32 xpos, f32 ypos, u32 color, u32 charID);
char3Idx asciiChar3D(f32 xpos, f32 ypos, f32 zpos, u32 color, u64 charID);

asciiobj_t* asciiObject2D(const vec2f_t* positions, const u32* colors, const u32* chars, u64 len);
asciiobj_t* asciiObject3D(const vec3f_t* positions, const u32* colors, const u32* chars, u64 len);

void moveAsciiChar2D(char2Idx idx, f32 dx, f32 dy);
void moveAsciiChar3D(char3Idx idx, f32 dx, f32 dy, f32 dz);
void moveAsciiObject2D(asciiobj_t* object, f32 dx, f32 dy);
void moveAsciiObject3D(asciiobj_t* object, f32 dx, f32 dy, f32 dz);
void setPosAsciiObject2D(asciiobj_t* object, vec2f_t* local, f32 xglobal, f32 yglobal);
void setPosAsciiObject3D(asciiobj_t* object, vec3f_t* local, f32 xglobal, f32 yglobal, f32 zglobal);

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
void        removePage(void* ptr);
void        freePaged(void);
*/
#endif