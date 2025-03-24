#include <objects.h>

#define BLACK   0x00000000
#define WHITE   0xffffff00
#define GOLD    0xfcd30300
#define RED     0xff000000
#define LGREEN  0x40d80000
#define MGREEN  0x0d700000
#define DGREEN  0x00300000

const ascii2info_t o_asciiBird[16] = {
    {'\\', WHITE, {  2, 33}}, {'-', WHITE, {14, -5}}, {'-', WHITE, {31, -5}}, {'\\', WHITE, {47,  2}},
    { '(', WHITE, {-12,  9}}, {'@',  GOLD, { 3, 10}}, {'O', WHITE, {32,  6}}, { '>',   RED, {55, 18}},
    {'\'', BLACK, { 33, 10}}, {'_', WHITE, {18, 32}}, {'_', WHITE, {33, 32}}, { '/', WHITE, {49, 32}},
    { '>',   RED, { 46, 18}}, {')', WHITE, {17, 11}}, {'B',  GOLD, {17, 28}}, { 'D',  GOLD, {34, 28}}
};

const ascii2info_t o_asciiPipeHeadTop[37] = {
    {'_', LGREEN, {  0, -14}}, {'_', LGREEN, {16, -14}}, {'_', LGREEN, { 32, -14}}, { '_', LGREEN, {48, -14}},
    {'_', LGREEN, { 64, -14}},
    {'#', DGREEN, { 42,  12}}, {'E', DGREEN, {58,  12}}, {'#', DGREEN, { 42,  28}}, { 'E', DGREEN, {58,  28}},
    {'_', DGREEN, {  1,  -2}}, {']', DGREEN, {16,  12}}, {']', DGREEN, { 16,  28}},
    {'!', MGREEN, { 28,  12}}, {'!', MGREEN, {28,  28}}, {'#', MGREEN, {  2,  28}}, {'\"', MGREEN, { 2,  17}},
    {'=', MGREEN, {  3,   0}}, {'=', MGREEN, {22,   0}}, {'=', MGREEN, { 42,   0}}, { '=', MGREEN, {61,   0}},
    {'`', MGREEN, { 66,  16}}, {'`', MGREEN, {66,  25}}, {'`', MGREEN, { 66,  34}},
    {'|', LGREEN, {-10,   1}}, {'|', LGREEN, {72,   1}}, {'|', LGREEN, {-10,  17}}, { '|', LGREEN, {72,  17}},
    {'|', LGREEN, {-10,  33}}, {'|', LGREEN, {72,  33}},
    {'T', LGREEN, {  0,  45}}, {'=', LGREEN, {16,  41}}, {'=', LGREEN, { 32,  41}}, { '=', LGREEN, {48,  41}},
    {'T', LGREEN, { 62,  45}},
    {'_', MGREEN, { 14,  41}}, {'_', MGREEN, {32,  41}}, {'_', MGREEN, { 50,  41}}
};

const ascii2info_t o_asciiPipeHeadBot[39] = {
    {'_', MGREEN, { 14, -23}}, {'_', MGREEN, {32, -23}}, {'_', MGREEN, { 50, -23}}, 
    {'-', LGREEN, {  0,  -5}}, {'=', LGREEN, {16,  -7}}, {'=', LGREEN, { 32,  -7}}, { '=', LGREEN, {48, -7}},
    {'-', LGREEN, { 62,  -5}}, 
    {'|', LGREEN, {-10,  33}}, {'|', LGREEN, {72,  33}}, {'|', LGREEN, {-10,  17}}, { '|', LGREEN, {72, 17}},
    {'|', LGREEN, {-10,   1}}, {'|', LGREEN, {72,   1}}, 
    {'`', MGREEN, { 66,  10}}, {'`', MGREEN, {66,  20}}, {'`', MGREEN, { 66,  30}}, 
    {'=', MGREEN, {  3,  34}}, {'=', MGREEN, {22,  34}}, {'=', MGREEN, { 42,  34}}, { '=', MGREEN, {61, 34}}, 
    {'!', MGREEN, { 28,  22}}, {'!', MGREEN, {28,   6}}, {'#', MGREEN, {  2,   6}}, {'\"', MGREEN, { 2, 25}}, 
    {'_', DGREEN, {  1,  20}}, {']', DGREEN, {16,  22}}, {']', DGREEN, { 16,   6}}, 
    {'#', DGREEN, { 42,  22}}, {'E', DGREEN, {58,  22}}, {'#', DGREEN, { 42,   6}}, { 'E', DGREEN, {58,  6}}, 
    {'_', LGREEN, {  0,  32}}, {'_', LGREEN, {16,  32}}, {'_', LGREEN, { 32,  32}}, { '_', LGREEN, {48, 32}},
    {'_', LGREEN, { 64,  32}}, {'|', LGREEN, { 0, -12}}, {'|', LGREEN, { 62, -12}}
};

const ascii2info_t o_asciiPipeSection[9] = {
    { '|', LGREEN, { 0, 61}}, {'-', DGREEN, {10, 55}}, {']', DGREEN, {24, 61}}, 
    { '!', MGREEN, {34, 61}}, {'E', DGREEN, {47, 61}}, {'|', LGREEN, {62, 61}}, 
    {'\"', MGREEN, {12, 67}}, {'`', MGREEN, {56, 63}}, {'`', MGREEN, {56, 69}}
};