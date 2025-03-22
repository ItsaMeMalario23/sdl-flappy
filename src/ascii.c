#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

#include <ascii.h>
#include <render.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

void* g_asciibuf = NULL;
asciiobj_t g_asciiobjects[RENDER_MAX_OBJECTS];

u16 g_bufidx = 0;
u16 g_objectsidx = 0;

u16 g_asciimode = RENDER_MODE_UNINIT;

bool g_memfragmented = 0;
bool g_objfragmented = 0;

pageinfo_t* g_pagedmem = NULL;

//
//  Public functions
//
void asciiInit(u32 renderMode)
{
    if (g_asciibuf || g_asciimode != RENDER_MODE_UNINIT) {
        SDL_Log("ASCII init failed, already initialized");
        return;
    }

    rAssert(renderMode == RENDER_MODE_2D || renderMode == RENDER_MODE_3D);

    // allocate with sizeof bigger struct so main buf only needs to be allocated once
    g_asciibuf = (void*) memAlloc(RENDER_MAX_CHARS * sizeof(ascii3_t));
    g_asciimode = renderMode;
    g_bufidx = 0;
    g_objectsidx = 0;
    g_memfragmented = 0;
    g_objfragmented = 0;

    memset(g_asciibuf, 0, RENDER_MAX_CHARS * sizeof(ascii3_t));
    memset(g_asciiobjects, 0, sizeof(g_asciiobjects));
}

void asciiCleanup(void)
{
    if (!g_asciibuf || g_asciimode == RENDER_MODE_UNINIT) {
        SDL_Log("ASCII cleanup failed, not initialized");
        return;
    }

    memFree(g_asciibuf);

    freePaged();

    g_asciibuf = NULL;
    g_bufidx = 0;
    g_objectsidx = 0;
    g_memfragmented = 0;
    g_objfragmented = 0;
    g_asciimode = RENDER_MODE_UNINIT;
}

// reset buffers
void asciiResetAll(void)
{
    if (!g_asciibuf || g_asciimode == RENDER_MODE_UNINIT) {
        SDL_Log("ASCII reset failed, not initialized");
        return;
    }

    memset(g_asciibuf, 0, RENDER_MAX_CHARS * sizeof(ascii3_t));
    memset(g_asciiobjects, 0, sizeof(g_asciiobjects));

    freePaged();

    g_bufidx = 0;
    g_objectsidx = 0;
    g_memfragmented = 0;
    g_objfragmented = 0;
}

// change between 2D and 3D mode
void asciiChangeMode(u32 renderMode)
{
    if (!g_asciibuf || g_asciimode == RENDER_MODE_UNINIT) {
        SDL_Log("ASCII ch mode failed, not initialized");
        return;
    }

    rAssert(renderMode == RENDER_MODE_2D || renderMode == RENDER_MODE_3D);

    memset(g_asciibuf, 0, RENDER_MAX_CHARS * sizeof(ascii3_t));
    memset(g_asciiobjects, 0, sizeof(g_asciiobjects));

    g_asciimode = renderMode;
    g_bufidx = 0;
    g_objectsidx = 0;
    g_memfragmented = 0;
    g_objfragmented = 0;
}

// render all ascii chars and objects
void renderAscii(u32 clearScr, u32 backgroundColor)
{
    if (!g_asciibuf) {
        SDL_Log("Could not render ascii, not initialized");
        return;
    }

    if (clearScr)
        clearScreen(backgroundColor);

    u32 bound = g_memfragmented ? RENDER_MAX_CHARS : g_bufidx;

    if (g_asciimode == RENDER_MODE_2D) {
        ascii2_t* buf = (ascii2_t*) g_asciibuf;

        for (u32 i = 0; i < bound; i++) {
            if (buf[i].charID < 32)
                continue;

            renderCharColor((i16) roundf(buf[i].xpos), (i16) roundf(buf[i].ypos), 0.25f, buf[i].color, buf[i].charID);
        }
    } else if (g_asciimode == RENDER_MODE_3D) {
        ascii3_t* buf = (ascii3_t*) g_asciibuf;

        for (u32 i = 0; i < bound; i++) {
            if (buf[i].charID < 32)
                continue;

            renderCharColor((i16) roundf(buf[i].xpos), (i16) roundf(buf[i].ypos), 0.25f, buf[i].color, buf[i].charID);
        }
    }

    if (g_pagedmem)
        renderPaged();
}

// add 2D ascii char
char2Idx asciiChar2D(f32 xpos, f32 ypos, u32 color, u32 charID)
{
    rAssert(g_asciibuf);
    rAssert(charID > 32);
    rAssert(g_asciimode == RENDER_MODE_2D);

    if (g_bufidx >= RENDER_MAX_CHARS) {
        SDL_Log("Failed to add 2D ascii char, buffer full");
        return -1;
    }

    ascii2_t* tmp = (ascii2_t*) g_asciibuf + g_bufidx;

    tmp->xpos = xpos;
    tmp->ypos = ypos;
    tmp->color = color;
    tmp->charID = charID;

    return g_bufidx++;
}

// add 3D ascii char
char3Idx asciiChar3D(f32 xpos, f32 ypos, f32 zpos, u32 color, u64 charID)
{
    rAssert(g_asciibuf);
    rAssert(charID > 32);
    rAssert(g_asciimode == RENDER_MODE_3D);

    if (g_bufidx >= RENDER_MAX_CHARS) {
        SDL_Log("Failed to add 3D ascii char, buffer full");
        return -1;
    }

    ascii3_t* tmp = (ascii3_t*) g_asciibuf + g_bufidx;

    tmp->xpos = xpos;
    tmp->ypos = ypos;
    tmp->zpos = zpos;
    tmp->color = color;
    tmp->charID = charID;

    return g_bufidx++;
}

// add 2D ascii object, default to white if colors is null
asciiobj_t* asciiObject2D(const vec2f_t* positions, const u32* colors, const u32* chars, u64 len)
{
    rAssert(chars);
    rAssert(positions);
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_2D);

    asciiobj_t* tmp = getAsciiObj();

    if (!tmp)
        return NULL;

    tmp->len = len;
    tmp->type = OBJ_TYPE_2D;
    tmp->paged = getAscii2Mem(&tmp->data.ascii2, len);

    ascii2_t* ch;

    for (u32 i = 0; i < len; i++) {
        ch = tmp->data.ascii2 + i;

        ch->xpos = positions[i].x;
        ch->ypos = positions[i].y;
        ch->charID = chars[i];

        rAssert(ch->charID > 32);

        if (colors)
            ch->color = colors[i];
        else
            ch->color = COLOR_WHITE;
    }

    return tmp;
}

// add 3D ascii object, default to white if colors is null
asciiobj_t* asciiObject3D(const vec3f_t* positions, const u32* colors, const u32* chars, u64 len)
{
    rAssert(chars);
    rAssert(positions);
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_3D);

    asciiobj_t* tmp = getAsciiObj();

    if (!tmp)
        return NULL;

    tmp->len = len;
    tmp->type = OBJ_TYPE_3D;
    tmp->paged = getAscii3Mem(&tmp->data.ascii3, len);

    ascii3_t* ch;

    for (u32 i = 0; i < len; i++) {
        ch = tmp->data.ascii3 + i;

        ch->xpos = positions[i].x;
        ch->ypos = positions[i].y;
        ch->zpos = positions[i].z;
        ch->charID =  chars[i];

        rAssert(ch->charID > 32);

        if (colors)
            ch->color = colors[i];
        else
            ch->color = COLOR_WHITE;
    }

    return tmp;
}

// move 2D ascii char
void moveAsciiChar2D(char2Idx idx, f32 dx, f32 dy)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_2D);
    rAssert(idx >= 0 && idx < RENDER_MAX_CHARS);

    ((ascii2_t*) g_asciibuf)[idx].xpos += dx;
    ((ascii2_t*) g_asciibuf)[idx].ypos += dy;
}

// move 3D ascii char
void moveAsciiChar3D(char3Idx idx, f32 dx, f32 dy, f32 dz)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_3D);
    rAssert(idx >= 0 && idx < RENDER_MAX_CHARS);

    ((ascii3_t*) g_asciibuf)[idx].xpos += dx;
    ((ascii3_t*) g_asciibuf)[idx].ypos += dy;
    ((ascii3_t*) g_asciibuf)[idx].zpos += dz;
}

// move 2D ascii object
void moveAsciiObject2D(asciiobj_t* object, f32 dx, f32 dy)
{
    rAssert(object);
    rAssert(object->data.ascii2);
    rAssert(object->type == OBJ_TYPE_2D);
    rAssert(g_asciimode == RENDER_MODE_2D);

    for (u32 i = 0; i < object->len; i++) {
        object->data.ascii2[i].xpos += dx;
        object->data.ascii2[i].ypos += dy;
    }
}

// move 3D ascii object
void moveAsciiObject3D(asciiobj_t* object, f32 dx, f32 dy, f32 dz)
{
    rAssert(object);
    rAssert(object->data.ascii3);
    rAssert(object->type == OBJ_TYPE_3D);
    rAssert(g_asciimode == RENDER_MODE_3D);

    for (u32 i = 0; i < object->len; i++) {
        object->data.ascii3[i].xpos += dx;
        object->data.ascii3[i].ypos += dy;
        object->data.ascii3[i].zpos += dz;
    }
}

// set position of 2D ascii object
void setPosAsciiObject2D(asciiobj_t* object, vec2f_t* local, f32 xglobal, f32 yglobal, u64 len)
{
    rAssert(object);
    rAssert(object->data.ascii2);
    rAssert(object->type == OBJ_TYPE_2D);
    rAssert(g_asciimode == RENDER_MODE_2D);

    for (u32 i = 0; i < object->len; i++) {
        object->data.ascii2[i].xpos = xglobal + local[i].x;
        object->data.ascii2[i].ypos = yglobal + local[i].y;
    }
}

// set position of 3D ascii object
void setPosAsciiObject3D(asciiobj_t* object, vec3f_t* local, f32 xglobal, f32 yglobal, f32 zglobal, u32 len)
{
    rAssert(object);
    rAssert(object->data.ascii3);
    rAssert(object->type == OBJ_TYPE_3D);
    rAssert(g_asciimode == RENDER_MODE_3D);

    for (u32 i = 0; i < object->len; i++) {
        object->data.ascii3[i].xpos = xglobal + local[i].x;
        object->data.ascii3[i].ypos = yglobal + local[i].y;
        object->data.ascii3[i].zpos = zglobal + local[i].z;
    }
}

// change color of 2D ascii char
void changeAsciiCharColor2D(char2Idx idx, u32 color)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_2D);
    rAssert(idx >= 0 && idx < RENDER_MAX_CHARS);

    ((ascii2_t*) g_asciibuf)[idx].color = color;
}

// change color of 3D ascii char
void changeAsciiCharColor3D(char3Idx idx, u32 color)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_3D);
    rAssert(idx >= 0 && idx < RENDER_MAX_CHARS);

    ((ascii3_t*) g_asciibuf)[idx].color = color;
}

// remove 2D char at idx
void removeAsciiChar2D(char2Idx idx)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_2D);
    rAssert(idx >= 0);
    rAssert(idx < RENDER_MAX_CHARS);

    memset((ascii2_t*) g_asciibuf + idx, 0, sizeof(ascii2_t));

    if (!g_memfragmented && idx + 1 == g_bufidx)
        g_bufidx--;
    else
        g_memfragmented = 1;

    rWarning(g_memfragmented || idx < g_bufidx);
}

// remove 3D char at idx
void removeAsciiChar3D(char3Idx idx)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_3D);
    rAssert(idx >= 0);
    rAssert(idx < RENDER_MAX_CHARS);

    memset((ascii3_t*) g_asciibuf + idx, 0, sizeof(ascii3_t));

    if (!g_memfragmented && idx + 1 == g_bufidx)
        g_bufidx--;
    else
        g_memfragmented = 1;

    rWarning(g_memfragmented || idx < g_bufidx);
}

// remove multiple 2D chars starting at startIdx
void removeAsciiChars2D(ascii2_t* chars, u32 numChars)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_2D);

    // make sure pointer is inside asciibuf
    rAssert(chars >= (ascii2_t*) g_asciibuf && chars < (ascii2_t*) g_asciibuf + RENDER_MAX_CHARS);

    memset(chars, 0, numChars * sizeof(ascii2_t));

    if (!g_memfragmented && chars + numChars + 1 == (ascii2_t*) g_asciibuf + g_bufidx)
        g_bufidx -= numChars;
    else
        g_memfragmented = 1;

    rWarning(g_memfragmented || chars + numChars < (ascii2_t*) g_asciibuf + g_bufidx);
}

// remove multiple 3D chars starting at chars
void removeAsciiChars3D(ascii3_t* chars, u32 numChars)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_3D);

    // make sure pointer is inside asciibuf
    rAssert(chars >= (ascii3_t*) g_asciibuf && chars < (ascii3_t*) g_asciibuf + RENDER_MAX_CHARS);

    memset(chars, 0, numChars * sizeof(ascii3_t));

    if (!g_memfragmented && chars + numChars + 1 == (ascii3_t*) g_asciibuf + g_bufidx)
        g_bufidx -= numChars;
    else
        g_memfragmented;

    rWarning(g_memfragmented || chars + numChars < (ascii3_t*) g_asciibuf + g_bufidx);
}

// remove ascii object
void removeAsciiObject(asciiobj_t* object)
{
    rAssert(object);
    rAssert(object->len);
    rAssert(object->type == OBJ_TYPE_2D || object->type == OBJ_TYPE_3D);
    
    // make sure object ptr is inside objects buf
    rAssert(object >= g_asciiobjects && object < g_asciiobjects + RENDER_MAX_OBJECTS);

    if (object->paged)
        removePaged(object->data.ascii2);

    if (object->type == OBJ_TYPE_2D)
        removeAsciiChars2D(object->data.ascii2, object->len);
    else
        removeAsciiChars3D(object->data.ascii3, object->len);

    memset(object, 0, sizeof(asciiobj_t));

    if (!g_objfragmented && object + 1 == g_asciiobjects + g_objectsidx)
        g_objectsidx--;
    else
        g_objfragmented;
}

//
//  Local functions
//

// find free memory for object, return null on failure
asciiobj_t* getAsciiObj(void)
{
    if (!g_objfragmented) {
        if (g_objectsidx >= RENDER_MAX_OBJECTS)
            goto fail;

        return g_asciiobjects + g_objectsidx++;
    }

    for (u32 i = 0; i < RENDER_MAX_OBJECTS; i++) {
        if (!g_asciiobjects[i].len)
            return g_asciiobjects + i;
    }

    fail:

    SDL_Log("Failed to add ascii object, object buffer full");
    return NULL;
}

// TODO increase bufidx?
// find memory for 2D ascii object, page if necessary
bool getAscii2Mem(ascii2_t** dst, u64 len)
{
    rAssert(g_asciimode == RENDER_MODE_2D);

    u16 count = 0;

    for (u16 i = 0; i < RENDER_MAX_CHARS; i++) {
        if (!((ascii2_t*) g_asciibuf + i)->charID)
            count++;
        else
            count = 0;

        if (count >= len) {
            *dst = (ascii2_t*) g_asciibuf + i + 1 - len;
            
            //
            g_bufidx += len;
            //

            return 0;
        }
    }

    // main buf out of memory, page
    ascii2_t* ptr = (ascii2_t*) memAlloc(len * sizeof(ascii2_t));

    addPage(ptr, len, OBJ_TYPE_2D);

    SDL_Log("Ascii buffer full, adding memory page for 2D ascii object, size %lld", len);

    return ptr;
}

// TODO increase bufidx?
// find memory for 3D ascii object, page if necessary
bool getAscii3Mem(ascii3_t** dst, u64 len)
{
    rAssert(g_asciimode == RENDER_MODE_3D);

    u16 count = 0;

    for (u16 i = 0; i < g_bufidx; i++) {
        if (!((ascii3_t*) g_asciibuf + i)->charID)
            count++;
        else
            count = 0;

        if (count >= len) {
            *dst = (ascii3_t*) g_asciibuf + i + 1 - len;
            return 0;
        }
    }

    // main buf out of memory, page
    ascii3_t* ptr = (ascii3_t*) memAlloc(len * sizeof(ascii3_t));

    addPage(ptr, len, OBJ_TYPE_3D);

    *dst = ptr;

    SDL_Log("Ascii buffer full, adding memory page for 3D ascii object, size %lld", len);

    return 1;
}

// find free index for 2D char, return -1 on failure
char2Idx getChar2(void)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_2D);

    if (!g_memfragmented) {
        if (g_bufidx >= RENDER_MAX_CHARS)
            goto fail;

        return g_bufidx++;
    }

    for (u32 i = 0; i < RENDER_MAX_CHARS; i++) {
        if (!((ascii2_t*) g_asciibuf + i)->charID)
            return i;
    }

    fail:

    // do not page for single char
    SDL_Log("Failed to add 2D ascii char, buffer full");
    return -1;
}

// find free index for 3D char, return -1 on failure
char3Idx getChar3(void)
{
    rAssert(g_asciibuf);
    rAssert(g_asciimode == RENDER_MODE_3D);

    if (!g_memfragmented) {
        if (g_bufidx >= RENDER_MAX_CHARS)
            goto fail;

        return g_bufidx++;
    }

    for (u32 i = 0; i < RENDER_MAX_CHARS; i++) {
        if (!((ascii3_t*) g_asciibuf + i)->charID)
            return i;
    }

    fail:

    // do not page for single char
    SDL_Log("Failed to add 3D ascii char, buffer full");
    return -1;
}

// render chars stored in paged memory
void renderPaged(void)
{
    if (!g_pagedmem)
        return;

    for (pageinfo_t* i = g_pagedmem; i; i = i->next) {
        if (i->type != g_asciimode)
            continue;

        if (g_asciimode == RENDER_MODE_2D) {
            for (u32 k = 0; k < i->len; k++) {
                if (((ascii2_t*) i->ptr)[k].charID < 32)
                    continue;

                renderCharColor
                (
                    (i16) roundf(((ascii2_t*) i->ptr)[k].xpos),
                    (i16) roundf(((ascii2_t*) i->ptr)[k].ypos),
                    0.25f,
                    ((ascii2_t*) i->ptr)[k].color,
                    ((ascii2_t*) i->ptr)[k].charID
                );
            }
        } else {
            for (u32 k = 0; k < i->len; k++) {
                if (((ascii3_t*) i->ptr)[k].charID < 32)
                    continue;

                renderCharColor
                (
                    (i16) roundf(((ascii3_t*) i->ptr)[k].xpos),
                    (i16) roundf(((ascii3_t*) i->ptr)[k].ypos),
                    0.25f,
                    ((ascii3_t*) i->ptr)[k].color,
                    ((ascii3_t*) i->ptr)[k].charID
                );
            }
        }
    }
}

// add bookkeeping info about paged memory
void addPage(void* ptr, u32 len, u32 type)
{
    pageinfo_t* tmp = g_pagedmem;

    if (!g_pagedmem) {
        g_pagedmem = (pageinfo_t*) memAlloc(sizeof(pageinfo_t));
        tmp = g_pagedmem;
    } else {
        tmp = g_pagedmem;

        while (tmp->next)
            tmp = tmp->next;

        rAssert(!tmp->next);

        tmp->next = (pageinfo_t*) memAlloc(sizeof(pageinfo_t));

        tmp = tmp->next;
    }

    rAssert(tmp);

    tmp->ptr = ptr;
    tmp->len = len;
    tmp->type = type;
    tmp->next = NULL;
}

// free paged memory block
void removePaged(void* ptr)
{
    rAssert(ptr);
    rAssert(g_pagedmem);

    pageinfo_t* tmp;

    if (g_pagedmem->ptr == ptr) {
        tmp = g_pagedmem->next;

        memFree(g_pagedmem->ptr);
        memFree(g_pagedmem);

        g_pagedmem = tmp;

        return;
    }

    for (pageinfo_t* i = g_pagedmem; i; i = i->next) {
        if (i->next && i->next->ptr == ptr) {
            tmp = i->next->next;

            memFree(i->next->ptr);
            memFree(i->next);

            i->next = tmp;

            return;
        }
    }
}

// free all paged memory
void freePaged(void)
{
    if (!g_pagedmem)
        return;
    
    pageinfo_t* tmp = NULL;

    for (pageinfo_t* i = g_pagedmem; i; i = i->next) {
        memFree(i->ptr);
        memFree(tmp);

        tmp = i;
    }

    memFree(tmp);

    g_pagedmem = NULL;
}