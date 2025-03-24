#ifndef OBJECTS_H
#define OBJECTS_H

#include <main.h>
#include <ascii.h>

#define O_ASCII_BIRD_LEN        16
#define O_PIPE_HEAD_TOP_LEN     37
#define O_PIPE_HEAD_BOT_LEN     39
#define O_PIPE_SECTION_LEN      9

extern const ascii2info_t o_asciiBird[O_ASCII_BIRD_LEN];
extern const ascii2info_t o_asciiPipeHeadTop[O_PIPE_HEAD_TOP_LEN];
extern const ascii2info_t o_asciiPipeHeadBot[O_PIPE_HEAD_BOT_LEN];
extern const ascii2info_t o_asciiPipeSection[O_PIPE_SECTION_LEN];

#endif