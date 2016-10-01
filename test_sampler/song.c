#include "lib/sampler.h"
// file  satie_gnossienne1.mid
// 2 tracks, 256 ticks/beat (PPQ)
// Time signature : 4/4
// (meta) ??? (89) : '\xfc\x00'
// Set Tempo : 100 bpm (600000 mpqn)
// Set Tempo : 100 bpm (600000 mpqn)
struct NoteEvent track_piano[] = {
    {.tick=   0, .note=0x29, .vel=0x39}, // ch 0 : note=41 F4 velocity=57
    {.tick=  24, .note=0x48, .vel=0x34}, // ch 0 : note=72 C7 velocity=52
    {.tick=   0, .note=0x38, .vel=0x39}, // ch 0 : note=56 G#5 velocity=57
    {.tick=   0, .note=0x3c, .vel=0x39}, // ch 0 : note=60 C6 velocity=57
    {.tick=   0, .note=0x41, .vel=0x39}, // ch 0 : note=65 F6 velocity=57
    {.tick=  12, .note=0x4b, .vel=0x3d}, // ch 0 : note=75 D#7 velocity=61
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x38}, // ch 0 : note=74 D7 velocity=56
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x35}, // ch 0 : note=72 C7 velocity=53
    {.tick=   0, .note=0x38, .vel=0x3d}, // ch 0 : note=56 G#5 velocity=61
    {.tick=   0, .note=0x3c, .vel=0x3d}, // ch 0 : note=60 C6 velocity=61
    {.tick=   0, .note=0x41, .vel=0x3d}, // ch 0 : note=65 F6 velocity=61
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x39}, // ch 0 : note=72 C7 velocity=57
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x39}, // ch 0 : note=71 B6 velocity=57
    {.tick=   0, .note=0x29, .vel=0x38}, // ch 0 : note=41 F4 velocity=56
    {.tick=  24, .note=0x38, .vel=0x33}, // ch 0 : note=56 G#5 velocity=51
    {.tick=   0, .note=0x3c, .vel=0x33}, // ch 0 : note=60 C6 velocity=51
    {.tick=   0, .note=0x41, .vel=0x33}, // ch 0 : note=65 F6 velocity=51
    {.tick=  22, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3d}, // ch 0 : note=72 C7 velocity=61
    {.tick=   2, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x3d}, // ch 0 : note=71 B6 velocity=61
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3d}, // ch 0 : note=56 G#5 velocity=61
    {.tick=   0, .note=0x3c, .vel=0x3d}, // ch 0 : note=60 C6 velocity=61
    {.tick=   0, .note=0x41, .vel=0x3d}, // ch 0 : note=65 F6 velocity=61
    {.tick=  24, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x48, .vel=0x36}, // ch 0 : note=72 C7 velocity=54
    {.tick=   0, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  12, .note=0x4b, .vel=0x40}, // ch 0 : note=75 D#7 velocity=64
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x34}, // ch 0 : note=74 D7 velocity=52
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x32}, // ch 0 : note=72 C7 velocity=50
    {.tick=   0, .note=0x38, .vel=0x40}, // ch 0 : note=56 G#5 velocity=64
    {.tick=   0, .note=0x3c, .vel=0x40}, // ch 0 : note=60 C6 velocity=64
    {.tick=   0, .note=0x41, .vel=0x40}, // ch 0 : note=65 F6 velocity=64
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  22, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x43}, // ch 0 : note=76 E7 velocity=67
    {.tick=   2, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x43}, // ch 0 : note=77 F7 velocity=67
    {.tick=   0, .note=0x29, .vel=0x3b}, // ch 0 : note=41 F4 velocity=59
    {.tick=  24, .note=0x38, .vel=0x39}, // ch 0 : note=56 G#5 velocity=57
    {.tick=   0, .note=0x3c, .vel=0x39}, // ch 0 : note=60 C6 velocity=57
    {.tick=   0, .note=0x41, .vel=0x39}, // ch 0 : note=65 F6 velocity=57
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x3d}, // ch 0 : note=76 E7 velocity=61
    {.tick=   2, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3d}, // ch 0 : note=77 F7 velocity=61
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3d}, // ch 0 : note=56 G#5 velocity=61
    {.tick=   0, .note=0x3c, .vel=0x3d}, // ch 0 : note=60 C6 velocity=61
    {.tick=   0, .note=0x41, .vel=0x3d}, // ch 0 : note=65 F6 velocity=61
    {.tick=  24, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x41}, // ch 0 : note=41 F4 velocity=65
    {.tick=  24, .note=0x48, .vel=0x3a}, // ch 0 : note=72 C7 velocity=58
    {.tick=   0, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  12, .note=0x4b, .vel=0x3c}, // ch 0 : note=75 D#7 velocity=60
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x40}, // ch 0 : note=74 D7 velocity=64
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x38}, // ch 0 : note=72 C7 velocity=56
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x37}, // ch 0 : note=72 C7 velocity=55
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x37}, // ch 0 : note=71 B6 velocity=55
    {.tick=   0, .note=0x29, .vel=0x3b}, // ch 0 : note=41 F4 velocity=59
    {.tick=  24, .note=0x38, .vel=0x31}, // ch 0 : note=56 G#5 velocity=49
    {.tick=   0, .note=0x3c, .vel=0x31}, // ch 0 : note=60 C6 velocity=49
    {.tick=   0, .note=0x41, .vel=0x31}, // ch 0 : note=65 F6 velocity=49
    {.tick=  22, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3b}, // ch 0 : note=68 G#6 velocity=59
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3b}, // ch 0 : note=67 G6 velocity=59
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3e}, // ch 0 : note=67 G6 velocity=62
    {.tick=   0, .note=0x30, .vel=0x48}, // ch 0 : note=48 C5 velocity=72
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x41}, // ch 0 : note=65 F6 velocity=65
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x41}, // ch 0 : note=67 G6 velocity=65
    {.tick=   0, .note=0x33, .vel=0x3b}, // ch 0 : note=51 D#5 velocity=59
    {.tick=   0, .note=0x37, .vel=0x3b}, // ch 0 : note=55 G5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=  48, .note=0x33, .vel=0}, // note off 
    {.tick=   0, .note=0x37, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x33, .vel=0x3b}, // ch 0 : note=51 D#5 velocity=59
    {.tick=   0, .note=0x37, .vel=0x3b}, // ch 0 : note=55 G5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=  12, .note=0x43, .vel=0}, // note off 
    {.tick=  10, .note=0x44, .vel=0x37}, // ch 0 : note=68 G#6 velocity=55
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x30, .vel=0}, // note off 
    {.tick=   0, .note=0x33, .vel=0}, // note off 
    {.tick=   0, .note=0x37, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x62}, // ch 0 : note=67 G6 velocity=98
    {.tick=   0, .note=0x29, .vel=0x5e}, // ch 0 : note=41 F4 velocity=94
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x5b}, // ch 0 : note=67 G6 velocity=91
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x5b}, // ch 0 : note=65 F6 velocity=91
    {.tick=   0, .note=0x38, .vel=0x58}, // ch 0 : note=56 G#5 velocity=88
    {.tick=   0, .note=0x3c, .vel=0x58}, // ch 0 : note=60 C6 velocity=88
    {.tick=   0, .note=0x41, .vel=0x58}, // ch 0 : note=65 F6 velocity=88
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x60}, // ch 0 : note=56 G#5 velocity=96
    {.tick=   0, .note=0x3c, .vel=0x60}, // ch 0 : note=60 C6 velocity=96
    {.tick=   0, .note=0x41, .vel=0x60}, // ch 0 : note=65 F6 velocity=96
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x62}, // ch 0 : note=41 F4 velocity=98
    {.tick=  24, .note=0x38, .vel=0x57}, // ch 0 : note=56 G#5 velocity=87
    {.tick=   0, .note=0x3c, .vel=0x57}, // ch 0 : note=60 C6 velocity=87
    {.tick=   0, .note=0x41, .vel=0x57}, // ch 0 : note=65 F6 velocity=87
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x64}, // ch 0 : note=56 G#5 velocity=100
    {.tick=   0, .note=0x3c, .vel=0x64}, // ch 0 : note=60 C6 velocity=100
    {.tick=   0, .note=0x41, .vel=0x64}, // ch 0 : note=65 F6 velocity=100
    {.tick=  24, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3c}, // ch 0 : note=41 F4 velocity=60
    {.tick=  24, .note=0x48, .vel=0x3c}, // ch 0 : note=72 C7 velocity=60
    {.tick=   0, .note=0x38, .vel=0x36}, // ch 0 : note=56 G#5 velocity=54
    {.tick=   0, .note=0x3c, .vel=0x36}, // ch 0 : note=60 C6 velocity=54
    {.tick=   0, .note=0x41, .vel=0x36}, // ch 0 : note=65 F6 velocity=54
    {.tick=  12, .note=0x4b, .vel=0x43}, // ch 0 : note=75 D#7 velocity=67
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x3a}, // ch 0 : note=74 D7 velocity=58
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x33}, // ch 0 : note=72 C7 velocity=51
    {.tick=   0, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3e}, // ch 0 : note=72 C7 velocity=62
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x3e}, // ch 0 : note=71 B6 velocity=62
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x38, .vel=0x39}, // ch 0 : note=56 G#5 velocity=57
    {.tick=   0, .note=0x3c, .vel=0x39}, // ch 0 : note=60 C6 velocity=57
    {.tick=   0, .note=0x41, .vel=0x39}, // ch 0 : note=65 F6 velocity=57
    {.tick=  22, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3a}, // ch 0 : note=72 C7 velocity=58
    {.tick=   2, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x3a}, // ch 0 : note=71 B6 velocity=58
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  24, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3c}, // ch 0 : note=41 F4 velocity=60
    {.tick=  24, .note=0x48, .vel=0x39}, // ch 0 : note=72 C7 velocity=57
    {.tick=   0, .note=0x38, .vel=0x34}, // ch 0 : note=56 G#5 velocity=52
    {.tick=   0, .note=0x3c, .vel=0x34}, // ch 0 : note=60 C6 velocity=52
    {.tick=   0, .note=0x41, .vel=0x34}, // ch 0 : note=65 F6 velocity=52
    {.tick=  12, .note=0x4b, .vel=0x3a}, // ch 0 : note=75 D#7 velocity=58
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x3a}, // ch 0 : note=74 D7 velocity=58
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x35}, // ch 0 : note=72 C7 velocity=53
    {.tick=   0, .note=0x38, .vel=0x3f}, // ch 0 : note=56 G#5 velocity=63
    {.tick=   0, .note=0x3c, .vel=0x3f}, // ch 0 : note=60 C6 velocity=63
    {.tick=   0, .note=0x41, .vel=0x3f}, // ch 0 : note=65 F6 velocity=63
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  22, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x3d}, // ch 0 : note=76 E7 velocity=61
    {.tick=   2, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3d}, // ch 0 : note=77 F7 velocity=61
    {.tick=   0, .note=0x29, .vel=0x41}, // ch 0 : note=41 F4 velocity=65
    {.tick=  24, .note=0x38, .vel=0x34}, // ch 0 : note=56 G#5 velocity=52
    {.tick=   0, .note=0x3c, .vel=0x34}, // ch 0 : note=60 C6 velocity=52
    {.tick=   0, .note=0x41, .vel=0x34}, // ch 0 : note=65 F6 velocity=52
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x3b}, // ch 0 : note=76 E7 velocity=59
    {.tick=   2, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3b}, // ch 0 : note=77 F7 velocity=59
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  24, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3f}, // ch 0 : note=41 F4 velocity=63
    {.tick=  24, .note=0x48, .vel=0x37}, // ch 0 : note=72 C7 velocity=55
    {.tick=   0, .note=0x38, .vel=0x38}, // ch 0 : note=56 G#5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=  12, .note=0x4b, .vel=0x3d}, // ch 0 : note=75 D#7 velocity=61
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x3f}, // ch 0 : note=74 D7 velocity=63
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3b}, // ch 0 : note=72 C7 velocity=59
    {.tick=   0, .note=0x38, .vel=0x42}, // ch 0 : note=56 G#5 velocity=66
    {.tick=   0, .note=0x3c, .vel=0x42}, // ch 0 : note=60 C6 velocity=66
    {.tick=   0, .note=0x41, .vel=0x42}, // ch 0 : note=65 F6 velocity=66
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x35}, // ch 0 : note=72 C7 velocity=53
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x35}, // ch 0 : note=71 B6 velocity=53
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x38, .vel=0x38}, // ch 0 : note=56 G#5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=  22, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x38}, // ch 0 : note=68 G#6 velocity=56
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x38}, // ch 0 : note=67 G6 velocity=56
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x43}, // ch 0 : note=65 F6 velocity=67
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x43}, // ch 0 : note=67 G6 velocity=67
    {.tick=   0, .note=0x30, .vel=0x42}, // ch 0 : note=48 C5 velocity=66
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3b}, // ch 0 : note=67 G6 velocity=59
    {.tick=   0, .note=0x33, .vel=0x38}, // ch 0 : note=51 D#5 velocity=56
    {.tick=   0, .note=0x37, .vel=0x38}, // ch 0 : note=55 G5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=  48, .note=0x33, .vel=0}, // note off 
    {.tick=   0, .note=0x37, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x33, .vel=0x39}, // ch 0 : note=51 D#5 velocity=57
    {.tick=   0, .note=0x37, .vel=0x39}, // ch 0 : note=55 G5 velocity=57
    {.tick=   0, .note=0x3c, .vel=0x39}, // ch 0 : note=60 C6 velocity=57
    {.tick=  12, .note=0x43, .vel=0}, // note off 
    {.tick=  10, .note=0x44, .vel=0x37}, // ch 0 : note=68 G#6 velocity=55
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x30, .vel=0}, // note off 
    {.tick=   0, .note=0x33, .vel=0}, // note off 
    {.tick=   0, .note=0x37, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x62}, // ch 0 : note=67 G6 velocity=98
    {.tick=   0, .note=0x29, .vel=0x5f}, // ch 0 : note=41 F4 velocity=95
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x60}, // ch 0 : note=67 G6 velocity=96
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x60}, // ch 0 : note=65 F6 velocity=96
    {.tick=   0, .note=0x38, .vel=0x60}, // ch 0 : note=56 G#5 velocity=96
    {.tick=   0, .note=0x3c, .vel=0x60}, // ch 0 : note=60 C6 velocity=96
    {.tick=   0, .note=0x41, .vel=0x60}, // ch 0 : note=65 F6 velocity=96
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x5f}, // ch 0 : note=56 G#5 velocity=95
    {.tick=   0, .note=0x3c, .vel=0x5f}, // ch 0 : note=60 C6 velocity=95
    {.tick=   0, .note=0x41, .vel=0x5f}, // ch 0 : note=65 F6 velocity=95
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x5d}, // ch 0 : note=41 F4 velocity=93
    {.tick=  24, .note=0x38, .vel=0x56}, // ch 0 : note=56 G#5 velocity=86
    {.tick=   0, .note=0x3c, .vel=0x56}, // ch 0 : note=60 C6 velocity=86
    {.tick=   0, .note=0x41, .vel=0x56}, // ch 0 : note=65 F6 velocity=86
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x5d}, // ch 0 : note=56 G#5 velocity=93
    {.tick=   0, .note=0x3c, .vel=0x5d}, // ch 0 : note=60 C6 velocity=93
    {.tick=   0, .note=0x41, .vel=0x5d}, // ch 0 : note=65 F6 velocity=93
    {.tick=  22, .note=0x44, .vel=0x65}, // ch 0 : note=68 G#6 velocity=101
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x62}, // ch 0 : note=70 A#6 velocity=98
    {.tick=   0, .note=0x22, .vel=0x5a}, // ch 0 : note=34 A#3 velocity=90
    {.tick=  24, .note=0x35, .vel=0x59}, // ch 0 : note=53 F5 velocity=89
    {.tick=   0, .note=0x3a, .vel=0x59}, // ch 0 : note=58 A#5 velocity=89
    {.tick=   0, .note=0x3d, .vel=0x59}, // ch 0 : note=61 C#6 velocity=89
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x5d}, // ch 0 : note=68 G#6 velocity=93
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x5d}, // ch 0 : note=67 G6 velocity=93
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x5f}, // ch 0 : note=53 F5 velocity=95
    {.tick=   0, .note=0x3a, .vel=0x5f}, // ch 0 : note=58 A#5 velocity=95
    {.tick=   0, .note=0x3d, .vel=0x5f}, // ch 0 : note=61 C#6 velocity=95
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x64}, // ch 0 : note=68 G#6 velocity=100
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x64}, // ch 0 : note=70 A#6 velocity=100
    {.tick=   0, .note=0x22, .vel=0x64}, // ch 0 : note=34 A#3 velocity=100
    {.tick=  24, .note=0x35, .vel=0x56}, // ch 0 : note=53 F5 velocity=86
    {.tick=   0, .note=0x3a, .vel=0x56}, // ch 0 : note=58 A#5 velocity=86
    {.tick=   0, .note=0x3d, .vel=0x56}, // ch 0 : note=61 C#6 velocity=86
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x5b}, // ch 0 : note=68 G#6 velocity=91
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x5b}, // ch 0 : note=67 G6 velocity=91
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x5d}, // ch 0 : note=53 F5 velocity=93
    {.tick=   0, .note=0x3a, .vel=0x5d}, // ch 0 : note=58 A#5 velocity=93
    {.tick=   0, .note=0x3d, .vel=0x5d}, // ch 0 : note=61 C#6 velocity=93
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x5d}, // ch 0 : note=68 G#6 velocity=93
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x5d}, // ch 0 : note=67 G6 velocity=93
    {.tick=   0, .note=0x29, .vel=0x69}, // ch 0 : note=41 F4 velocity=105
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x60}, // ch 0 : note=67 G6 velocity=96
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x60}, // ch 0 : note=65 F6 velocity=96
    {.tick=   0, .note=0x38, .vel=0x57}, // ch 0 : note=56 G#5 velocity=87
    {.tick=   0, .note=0x3c, .vel=0x57}, // ch 0 : note=60 C6 velocity=87
    {.tick=   0, .note=0x41, .vel=0x57}, // ch 0 : note=65 F6 velocity=87
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x65}, // ch 0 : note=56 G#5 velocity=101
    {.tick=   0, .note=0x3c, .vel=0x65}, // ch 0 : note=60 C6 velocity=101
    {.tick=   0, .note=0x41, .vel=0x65}, // ch 0 : note=65 F6 velocity=101
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x5e}, // ch 0 : note=41 F4 velocity=94
    {.tick=  24, .note=0x38, .vel=0x59}, // ch 0 : note=56 G#5 velocity=89
    {.tick=   0, .note=0x3c, .vel=0x59}, // ch 0 : note=60 C6 velocity=89
    {.tick=   0, .note=0x41, .vel=0x59}, // ch 0 : note=65 F6 velocity=89
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x61}, // ch 0 : note=56 G#5 velocity=97
    {.tick=   0, .note=0x3c, .vel=0x61}, // ch 0 : note=60 C6 velocity=97
    {.tick=   0, .note=0x41, .vel=0x61}, // ch 0 : note=65 F6 velocity=97
    {.tick=  22, .note=0x44, .vel=0x67}, // ch 0 : note=68 G#6 velocity=103
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x62}, // ch 0 : note=70 A#6 velocity=98
    {.tick=   0, .note=0x22, .vel=0x5b}, // ch 0 : note=34 A#3 velocity=91
    {.tick=  24, .note=0x35, .vel=0x57}, // ch 0 : note=53 F5 velocity=87
    {.tick=   0, .note=0x3a, .vel=0x57}, // ch 0 : note=58 A#5 velocity=87
    {.tick=   0, .note=0x3d, .vel=0x57}, // ch 0 : note=61 C#6 velocity=87
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x5a}, // ch 0 : note=68 G#6 velocity=90
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x5a}, // ch 0 : note=67 G6 velocity=90
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x60}, // ch 0 : note=53 F5 velocity=96
    {.tick=   0, .note=0x3a, .vel=0x60}, // ch 0 : note=58 A#5 velocity=96
    {.tick=   0, .note=0x3d, .vel=0x60}, // ch 0 : note=61 C#6 velocity=96
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x68}, // ch 0 : note=68 G#6 velocity=104
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x68}, // ch 0 : note=70 A#6 velocity=104
    {.tick=   0, .note=0x22, .vel=0x63}, // ch 0 : note=34 A#3 velocity=99
    {.tick=  24, .note=0x35, .vel=0x55}, // ch 0 : note=53 F5 velocity=85
    {.tick=   0, .note=0x3a, .vel=0x55}, // ch 0 : note=58 A#5 velocity=85
    {.tick=   0, .note=0x3d, .vel=0x55}, // ch 0 : note=61 C#6 velocity=85
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x59}, // ch 0 : note=68 G#6 velocity=89
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x59}, // ch 0 : note=67 G6 velocity=89
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x5e}, // ch 0 : note=53 F5 velocity=94
    {.tick=   0, .note=0x3a, .vel=0x5e}, // ch 0 : note=58 A#5 velocity=94
    {.tick=   0, .note=0x3d, .vel=0x5e}, // ch 0 : note=61 C#6 velocity=94
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x62}, // ch 0 : note=68 G#6 velocity=98
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x62}, // ch 0 : note=67 G6 velocity=98
    {.tick=   0, .note=0x29, .vel=0x68}, // ch 0 : note=41 F4 velocity=104
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x57}, // ch 0 : note=67 G6 velocity=87
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x57}, // ch 0 : note=65 F6 velocity=87
    {.tick=   0, .note=0x38, .vel=0x5a}, // ch 0 : note=56 G#5 velocity=90
    {.tick=   0, .note=0x3c, .vel=0x5a}, // ch 0 : note=60 C6 velocity=90
    {.tick=   0, .note=0x41, .vel=0x5a}, // ch 0 : note=65 F6 velocity=90
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x60}, // ch 0 : note=56 G#5 velocity=96
    {.tick=   0, .note=0x3c, .vel=0x60}, // ch 0 : note=60 C6 velocity=96
    {.tick=   0, .note=0x41, .vel=0x60}, // ch 0 : note=65 F6 velocity=96
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x60}, // ch 0 : note=41 F4 velocity=96
    {.tick=  24, .note=0x38, .vel=0x57}, // ch 0 : note=56 G#5 velocity=87
    {.tick=   0, .note=0x3c, .vel=0x57}, // ch 0 : note=60 C6 velocity=87
    {.tick=   0, .note=0x41, .vel=0x57}, // ch 0 : note=65 F6 velocity=87
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x61}, // ch 0 : note=56 G#5 velocity=97
    {.tick=   0, .note=0x3c, .vel=0x61}, // ch 0 : note=60 C6 velocity=97
    {.tick=   0, .note=0x41, .vel=0x61}, // ch 0 : note=65 F6 velocity=97
    {.tick=  24, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x61}, // ch 0 : note=41 F4 velocity=97
    {.tick=  12, .note=0x48, .vel=0x38}, // ch 0 : note=72 C7 velocity=56
    {.tick=  12, .note=0x4a, .vel=0x38}, // ch 0 : note=74 D7 velocity=56
    {.tick=   0, .note=0x38, .vel=0x39}, // ch 0 : note=56 G#5 velocity=57
    {.tick=   0, .note=0x3c, .vel=0x39}, // ch 0 : note=60 C6 velocity=57
    {.tick=   0, .note=0x41, .vel=0x39}, // ch 0 : note=65 F6 velocity=57
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4c, .vel=0x43}, // ch 0 : note=76 E7 velocity=67
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  12, .note=0x4d, .vel=0x47}, // ch 0 : note=77 F7 velocity=71
    {.tick=   0, .note=0x4c, .vel=0}, // note off 
    {.tick=  12, .note=0x4f, .vel=0x40}, // ch 0 : note=79 G7 velocity=64
    {.tick=   0, .note=0x4d, .vel=0}, // note off 
    {.tick=  12, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x53, .vel=0x45}, // ch 0 : note=83 B7 velocity=69
    {.tick=   0, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=   0, .note=0x4f, .vel=0}, // note off 
    {.tick=  12, .note=0x4f, .vel=0x38}, // ch 0 : note=79 G7 velocity=56
    {.tick=   0, .note=0x53, .vel=0}, // note off 
    {.tick=  12, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x32}, // ch 0 : note=77 F7 velocity=50
    {.tick=   0, .note=0x29, .vel=0x41}, // ch 0 : note=41 F4 velocity=65
    {.tick=   0, .note=0x4f, .vel=0}, // note off 
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4f, .vel=0x38}, // ch 0 : note=79 G7 velocity=56
    {.tick=   2, .note=0x4f, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x38}, // ch 0 : note=77 F7 velocity=56
    {.tick=   0, .note=0x38, .vel=0x36}, // ch 0 : note=56 G#5 velocity=54
    {.tick=   0, .note=0x3c, .vel=0x36}, // ch 0 : note=60 C6 velocity=54
    {.tick=   0, .note=0x41, .vel=0x36}, // ch 0 : note=65 F6 velocity=54
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3f}, // ch 0 : note=56 G#5 velocity=63
    {.tick=   0, .note=0x3c, .vel=0x3f}, // ch 0 : note=60 C6 velocity=63
    {.tick=   0, .note=0x41, .vel=0x3f}, // ch 0 : note=65 F6 velocity=63
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4f, .vel=0x3a}, // ch 0 : note=79 G7 velocity=58
    {.tick=   2, .note=0x4f, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3a}, // ch 0 : note=77 F7 velocity=58
    {.tick=   0, .note=0x29, .vel=0x3f}, // ch 0 : note=41 F4 velocity=63
    {.tick=  24, .note=0x38, .vel=0x39}, // ch 0 : note=56 G#5 velocity=57
    {.tick=   0, .note=0x3c, .vel=0x39}, // ch 0 : note=60 C6 velocity=57
    {.tick=   0, .note=0x41, .vel=0x39}, // ch 0 : note=65 F6 velocity=57
    {.tick=  21, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3f}, // ch 0 : note=77 F7 velocity=63
    {.tick=   3, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x3f}, // ch 0 : note=76 E7 velocity=63
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  22, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x49, .vel=0x37}, // ch 0 : note=73 C#7 velocity=55
    {.tick=   2, .note=0x49, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x37}, // ch 0 : note=72 C7 velocity=55
    {.tick=   0, .note=0x29, .vel=0x3f}, // ch 0 : note=41 F4 velocity=63
    {.tick=  24, .note=0x38, .vel=0x36}, // ch 0 : note=56 G#5 velocity=54
    {.tick=   0, .note=0x3c, .vel=0x36}, // ch 0 : note=60 C6 velocity=54
    {.tick=   0, .note=0x41, .vel=0x36}, // ch 0 : note=65 F6 velocity=54
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x38}, // ch 0 : note=72 C7 velocity=56
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x38}, // ch 0 : note=71 B6 velocity=56
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3d}, // ch 0 : note=56 G#5 velocity=61
    {.tick=   0, .note=0x3c, .vel=0x3d}, // ch 0 : note=60 C6 velocity=61
    {.tick=   0, .note=0x41, .vel=0x3d}, // ch 0 : note=65 F6 velocity=61
    {.tick=  22, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3a}, // ch 0 : note=68 G#6 velocity=58
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3a}, // ch 0 : note=67 G6 velocity=58
    {.tick=   0, .note=0x29, .vel=0x3a}, // ch 0 : note=41 F4 velocity=58
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x38}, // ch 0 : note=67 G6 velocity=56
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=   0, .note=0x38, .vel=0x35}, // ch 0 : note=56 G#5 velocity=53
    {.tick=   0, .note=0x3c, .vel=0x35}, // ch 0 : note=60 C6 velocity=53
    {.tick=   0, .note=0x41, .vel=0x35}, // ch 0 : note=65 F6 velocity=53
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3c}, // ch 0 : note=41 F4 velocity=60
    {.tick=  24, .note=0x38, .vel=0x32}, // ch 0 : note=56 G#5 velocity=50
    {.tick=   0, .note=0x3c, .vel=0x32}, // ch 0 : note=60 C6 velocity=50
    {.tick=   0, .note=0x41, .vel=0x32}, // ch 0 : note=65 F6 velocity=50
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=  24, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3e}, // ch 0 : note=41 F4 velocity=62
    {.tick=  12, .note=0x48, .vel=0x39}, // ch 0 : note=72 C7 velocity=57
    {.tick=  12, .note=0x4a, .vel=0x39}, // ch 0 : note=74 D7 velocity=57
    {.tick=   0, .note=0x38, .vel=0x38}, // ch 0 : note=56 G#5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4c, .vel=0x3f}, // ch 0 : note=76 E7 velocity=63
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  12, .note=0x4d, .vel=0x46}, // ch 0 : note=77 F7 velocity=70
    {.tick=   0, .note=0x4c, .vel=0}, // note off 
    {.tick=  12, .note=0x4f, .vel=0x3a}, // ch 0 : note=79 G7 velocity=58
    {.tick=   0, .note=0x4d, .vel=0}, // note off 
    {.tick=  12, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x53, .vel=0x42}, // ch 0 : note=83 B7 velocity=66
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=   0, .note=0x4f, .vel=0}, // note off 
    {.tick=  12, .note=0x4f, .vel=0x38}, // ch 0 : note=79 G7 velocity=56
    {.tick=   0, .note=0x53, .vel=0}, // note off 
    {.tick=  12, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x35}, // ch 0 : note=77 F7 velocity=53
    {.tick=   0, .note=0x29, .vel=0x3b}, // ch 0 : note=41 F4 velocity=59
    {.tick=   0, .note=0x4f, .vel=0}, // note off 
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4f, .vel=0x36}, // ch 0 : note=79 G7 velocity=54
    {.tick=   2, .note=0x4f, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x36}, // ch 0 : note=77 F7 velocity=54
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4f, .vel=0x39}, // ch 0 : note=79 G7 velocity=57
    {.tick=   2, .note=0x4f, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x39}, // ch 0 : note=77 F7 velocity=57
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  21, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x38}, // ch 0 : note=77 F7 velocity=56
    {.tick=   3, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x38}, // ch 0 : note=76 E7 velocity=56
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  22, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x49, .vel=0x38}, // ch 0 : note=73 C#7 velocity=56
    {.tick=   2, .note=0x49, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x38}, // ch 0 : note=72 C7 velocity=56
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3b}, // ch 0 : note=72 C7 velocity=59
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x3b}, // ch 0 : note=71 B6 velocity=59
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  22, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3c}, // ch 0 : note=68 G#6 velocity=60
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3c}, // ch 0 : note=67 G6 velocity=60
    {.tick=   0, .note=0x29, .vel=0x3a}, // ch 0 : note=41 F4 velocity=58
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3f}, // ch 0 : note=67 G6 velocity=63
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x3f}, // ch 0 : note=65 F6 velocity=63
    {.tick=   0, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3f}, // ch 0 : note=56 G#5 velocity=63
    {.tick=   0, .note=0x3c, .vel=0x3f}, // ch 0 : note=60 C6 velocity=63
    {.tick=   0, .note=0x41, .vel=0x3f}, // ch 0 : note=65 F6 velocity=63
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  22, .note=0x44, .vel=0x42}, // ch 0 : note=68 G#6 velocity=66
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x3d}, // ch 0 : note=70 A#6 velocity=61
    {.tick=   0, .note=0x22, .vel=0x3b}, // ch 0 : note=34 A#3 velocity=59
    {.tick=  24, .note=0x35, .vel=0x37}, // ch 0 : note=53 F5 velocity=55
    {.tick=   0, .note=0x3a, .vel=0x37}, // ch 0 : note=58 A#5 velocity=55
    {.tick=   0, .note=0x3d, .vel=0x37}, // ch 0 : note=61 C#6 velocity=55
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3b}, // ch 0 : note=68 G#6 velocity=59
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3b}, // ch 0 : note=67 G6 velocity=59
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x40}, // ch 0 : note=53 F5 velocity=64
    {.tick=   0, .note=0x3a, .vel=0x40}, // ch 0 : note=58 A#5 velocity=64
    {.tick=   0, .note=0x3d, .vel=0x40}, // ch 0 : note=61 C#6 velocity=64
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x42}, // ch 0 : note=68 G#6 velocity=66
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x42}, // ch 0 : note=70 A#6 velocity=66
    {.tick=   0, .note=0x22, .vel=0x3b}, // ch 0 : note=34 A#3 velocity=59
    {.tick=  24, .note=0x35, .vel=0x3b}, // ch 0 : note=53 F5 velocity=59
    {.tick=   0, .note=0x3a, .vel=0x3b}, // ch 0 : note=58 A#5 velocity=59
    {.tick=   0, .note=0x3d, .vel=0x3b}, // ch 0 : note=61 C#6 velocity=59
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3f}, // ch 0 : note=68 G#6 velocity=63
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3f}, // ch 0 : note=67 G6 velocity=63
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x3d}, // ch 0 : note=53 F5 velocity=61
    {.tick=   0, .note=0x3a, .vel=0x3d}, // ch 0 : note=58 A#5 velocity=61
    {.tick=   0, .note=0x3d, .vel=0x3d}, // ch 0 : note=61 C#6 velocity=61
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x37}, // ch 0 : note=68 G#6 velocity=55
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x37}, // ch 0 : note=67 G6 velocity=55
    {.tick=   0, .note=0x29, .vel=0x40}, // ch 0 : note=41 F4 velocity=64
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x37}, // ch 0 : note=67 G6 velocity=55
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=   0, .note=0x38, .vel=0x35}, // ch 0 : note=56 G#5 velocity=53
    {.tick=   0, .note=0x3c, .vel=0x35}, // ch 0 : note=60 C6 velocity=53
    {.tick=   0, .note=0x41, .vel=0x35}, // ch 0 : note=65 F6 velocity=53
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x39}, // ch 0 : note=41 F4 velocity=57
    {.tick=  24, .note=0x38, .vel=0x36}, // ch 0 : note=56 G#5 velocity=54
    {.tick=   0, .note=0x3c, .vel=0x36}, // ch 0 : note=60 C6 velocity=54
    {.tick=   0, .note=0x41, .vel=0x36}, // ch 0 : note=65 F6 velocity=54
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=  22, .note=0x44, .vel=0x39}, // ch 0 : note=68 G#6 velocity=57
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x3d}, // ch 0 : note=70 A#6 velocity=61
    {.tick=   0, .note=0x22, .vel=0x3b}, // ch 0 : note=34 A#3 velocity=59
    {.tick=  24, .note=0x35, .vel=0x32}, // ch 0 : note=53 F5 velocity=50
    {.tick=   0, .note=0x3a, .vel=0x32}, // ch 0 : note=58 A#5 velocity=50
    {.tick=   0, .note=0x3d, .vel=0x32}, // ch 0 : note=61 C#6 velocity=50
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x37}, // ch 0 : note=68 G#6 velocity=55
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x37}, // ch 0 : note=67 G6 velocity=55
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x3f}, // ch 0 : note=53 F5 velocity=63
    {.tick=   0, .note=0x3a, .vel=0x3f}, // ch 0 : note=58 A#5 velocity=63
    {.tick=   0, .note=0x3d, .vel=0x3f}, // ch 0 : note=61 C#6 velocity=63
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x42}, // ch 0 : note=68 G#6 velocity=66
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x42}, // ch 0 : note=70 A#6 velocity=66
    {.tick=   0, .note=0x22, .vel=0x37}, // ch 0 : note=34 A#3 velocity=55
    {.tick=  24, .note=0x35, .vel=0x36}, // ch 0 : note=53 F5 velocity=54
    {.tick=   0, .note=0x3a, .vel=0x36}, // ch 0 : note=58 A#5 velocity=54
    {.tick=   0, .note=0x3d, .vel=0x36}, // ch 0 : note=61 C#6 velocity=54
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x38}, // ch 0 : note=68 G#6 velocity=56
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x38}, // ch 0 : note=67 G6 velocity=56
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x3f}, // ch 0 : note=53 F5 velocity=63
    {.tick=   0, .note=0x3a, .vel=0x3f}, // ch 0 : note=58 A#5 velocity=63
    {.tick=   0, .note=0x3d, .vel=0x3f}, // ch 0 : note=61 C#6 velocity=63
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3b}, // ch 0 : note=68 G#6 velocity=59
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3b}, // ch 0 : note=67 G6 velocity=59
    {.tick=   0, .note=0x29, .vel=0x43}, // ch 0 : note=41 F4 velocity=67
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x38}, // ch 0 : note=67 G6 velocity=56
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=   0, .note=0x38, .vel=0x31}, // ch 0 : note=56 G#5 velocity=49
    {.tick=   0, .note=0x3c, .vel=0x31}, // ch 0 : note=60 C6 velocity=49
    {.tick=   0, .note=0x41, .vel=0x31}, // ch 0 : note=65 F6 velocity=49
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3b}, // ch 0 : note=41 F4 velocity=59
    {.tick=  24, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x38}, // ch 0 : note=56 G#5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=  24, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3b}, // ch 0 : note=41 F4 velocity=59
    {.tick=  24, .note=0x48, .vel=0x38}, // ch 0 : note=72 C7 velocity=56
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  12, .note=0x4b, .vel=0x3a}, // ch 0 : note=75 D#7 velocity=58
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x3a}, // ch 0 : note=74 D7 velocity=58
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x37}, // ch 0 : note=72 C7 velocity=55
    {.tick=   0, .note=0x38, .vel=0x41}, // ch 0 : note=56 G#5 velocity=65
    {.tick=   0, .note=0x3c, .vel=0x41}, // ch 0 : note=60 C6 velocity=65
    {.tick=   0, .note=0x41, .vel=0x41}, // ch 0 : note=65 F6 velocity=65
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3a}, // ch 0 : note=72 C7 velocity=58
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x3a}, // ch 0 : note=71 B6 velocity=58
    {.tick=   0, .note=0x29, .vel=0x3c}, // ch 0 : note=41 F4 velocity=60
    {.tick=  24, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  24, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x40}, // ch 0 : note=41 F4 velocity=64
    {.tick=  24, .note=0x48, .vel=0x36}, // ch 0 : note=72 C7 velocity=54
    {.tick=   0, .note=0x38, .vel=0x35}, // ch 0 : note=56 G#5 velocity=53
    {.tick=   0, .note=0x3c, .vel=0x35}, // ch 0 : note=60 C6 velocity=53
    {.tick=   0, .note=0x41, .vel=0x35}, // ch 0 : note=65 F6 velocity=53
    {.tick=  12, .note=0x4b, .vel=0x39}, // ch 0 : note=75 D#7 velocity=57
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x39}, // ch 0 : note=74 D7 velocity=57
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3c}, // ch 0 : note=72 C7 velocity=60
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  22, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x42}, // ch 0 : note=76 E7 velocity=66
    {.tick=   2, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x42}, // ch 0 : note=77 F7 velocity=66
    {.tick=   0, .note=0x29, .vel=0x39}, // ch 0 : note=41 F4 velocity=57
    {.tick=  24, .note=0x38, .vel=0x3d}, // ch 0 : note=56 G#5 velocity=61
    {.tick=   0, .note=0x3c, .vel=0x3d}, // ch 0 : note=60 C6 velocity=61
    {.tick=   0, .note=0x41, .vel=0x3d}, // ch 0 : note=65 F6 velocity=61
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=  24, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3b}, // ch 0 : note=41 F4 velocity=59
    {.tick=  24, .note=0x48, .vel=0x39}, // ch 0 : note=72 C7 velocity=57
    {.tick=   0, .note=0x38, .vel=0x35}, // ch 0 : note=56 G#5 velocity=53
    {.tick=   0, .note=0x3c, .vel=0x35}, // ch 0 : note=60 C6 velocity=53
    {.tick=   0, .note=0x41, .vel=0x35}, // ch 0 : note=65 F6 velocity=53
    {.tick=  12, .note=0x4b, .vel=0x41}, // ch 0 : note=75 D#7 velocity=65
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x3f}, // ch 0 : note=74 D7 velocity=63
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x38}, // ch 0 : note=72 C7 velocity=56
    {.tick=   0, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3a}, // ch 0 : note=72 C7 velocity=58
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x3a}, // ch 0 : note=71 B6 velocity=58
    {.tick=   0, .note=0x29, .vel=0x40}, // ch 0 : note=41 F4 velocity=64
    {.tick=  24, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3f}, // ch 0 : note=56 G#5 velocity=63
    {.tick=   0, .note=0x3c, .vel=0x3f}, // ch 0 : note=60 C6 velocity=63
    {.tick=   0, .note=0x41, .vel=0x3f}, // ch 0 : note=65 F6 velocity=63
    {.tick=  24, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x48, .vel=0x33}, // ch 0 : note=72 C7 velocity=51
    {.tick=   0, .note=0x38, .vel=0x35}, // ch 0 : note=56 G#5 velocity=53
    {.tick=   0, .note=0x3c, .vel=0x35}, // ch 0 : note=60 C6 velocity=53
    {.tick=   0, .note=0x41, .vel=0x35}, // ch 0 : note=65 F6 velocity=53
    {.tick=  12, .note=0x4b, .vel=0x38}, // ch 0 : note=75 D#7 velocity=56
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4a, .vel=0x3b}, // ch 0 : note=74 D7 velocity=59
    {.tick=   0, .note=0x4b, .vel=0}, // note off 
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x33}, // ch 0 : note=72 C7 velocity=51
    {.tick=   0, .note=0x38, .vel=0x38}, // ch 0 : note=56 G#5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  22, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x3f}, // ch 0 : note=76 E7 velocity=63
    {.tick=   2, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3f}, // ch 0 : note=77 F7 velocity=63
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x38, .vel=0x34}, // ch 0 : note=56 G#5 velocity=52
    {.tick=   0, .note=0x3c, .vel=0x34}, // ch 0 : note=60 C6 velocity=52
    {.tick=   0, .note=0x41, .vel=0x34}, // ch 0 : note=65 F6 velocity=52
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3f}, // ch 0 : note=68 G#6 velocity=63
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x3f}, // ch 0 : note=70 A#6 velocity=63
    {.tick=   0, .note=0x22, .vel=0x3a}, // ch 0 : note=34 A#3 velocity=58
    {.tick=  24, .note=0x35, .vel=0x35}, // ch 0 : note=53 F5 velocity=53
    {.tick=   0, .note=0x3a, .vel=0x35}, // ch 0 : note=58 A#5 velocity=53
    {.tick=   0, .note=0x3d, .vel=0x35}, // ch 0 : note=61 C#6 velocity=53
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x37}, // ch 0 : note=68 G#6 velocity=55
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x37}, // ch 0 : note=67 G6 velocity=55
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x3d}, // ch 0 : note=53 F5 velocity=61
    {.tick=   0, .note=0x3a, .vel=0x3d}, // ch 0 : note=58 A#5 velocity=61
    {.tick=   0, .note=0x3d, .vel=0x3d}, // ch 0 : note=61 C#6 velocity=61
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x40}, // ch 0 : note=68 G#6 velocity=64
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x40}, // ch 0 : note=70 A#6 velocity=64
    {.tick=   0, .note=0x22, .vel=0x3b}, // ch 0 : note=34 A#3 velocity=59
    {.tick=  24, .note=0x35, .vel=0x35}, // ch 0 : note=53 F5 velocity=53
    {.tick=   0, .note=0x3a, .vel=0x35}, // ch 0 : note=58 A#5 velocity=53
    {.tick=   0, .note=0x3d, .vel=0x35}, // ch 0 : note=61 C#6 velocity=53
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x36}, // ch 0 : note=68 G#6 velocity=54
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x36}, // ch 0 : note=67 G6 velocity=54
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x3e}, // ch 0 : note=53 F5 velocity=62
    {.tick=   0, .note=0x3a, .vel=0x3e}, // ch 0 : note=58 A#5 velocity=62
    {.tick=   0, .note=0x3d, .vel=0x3e}, // ch 0 : note=61 C#6 velocity=62
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3b}, // ch 0 : note=68 G#6 velocity=59
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3b}, // ch 0 : note=67 G6 velocity=59
    {.tick=   0, .note=0x29, .vel=0x46}, // ch 0 : note=41 F4 velocity=70
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x35}, // ch 0 : note=67 G6 velocity=53
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x35}, // ch 0 : note=65 F6 velocity=53
    {.tick=   0, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x42}, // ch 0 : note=56 G#5 velocity=66
    {.tick=   0, .note=0x3c, .vel=0x42}, // ch 0 : note=60 C6 velocity=66
    {.tick=   0, .note=0x41, .vel=0x42}, // ch 0 : note=65 F6 velocity=66
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x38}, // ch 0 : note=41 F4 velocity=56
    {.tick=  24, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  22, .note=0x44, .vel=0x3a}, // ch 0 : note=68 G#6 velocity=58
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x3d}, // ch 0 : note=70 A#6 velocity=61
    {.tick=   0, .note=0x22, .vel=0x36}, // ch 0 : note=34 A#3 velocity=54
    {.tick=  24, .note=0x35, .vel=0x34}, // ch 0 : note=53 F5 velocity=52
    {.tick=   0, .note=0x3a, .vel=0x34}, // ch 0 : note=58 A#5 velocity=52
    {.tick=   0, .note=0x3d, .vel=0x34}, // ch 0 : note=61 C#6 velocity=52
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3b}, // ch 0 : note=68 G#6 velocity=59
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3b}, // ch 0 : note=67 G6 velocity=59
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x3a}, // ch 0 : note=53 F5 velocity=58
    {.tick=   0, .note=0x3a, .vel=0x3a}, // ch 0 : note=58 A#5 velocity=58
    {.tick=   0, .note=0x3d, .vel=0x3a}, // ch 0 : note=61 C#6 velocity=58
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x42}, // ch 0 : note=68 G#6 velocity=66
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x42}, // ch 0 : note=70 A#6 velocity=66
    {.tick=   0, .note=0x22, .vel=0x3d}, // ch 0 : note=34 A#3 velocity=61
    {.tick=  24, .note=0x35, .vel=0x33}, // ch 0 : note=53 F5 velocity=51
    {.tick=   0, .note=0x3a, .vel=0x33}, // ch 0 : note=58 A#5 velocity=51
    {.tick=   0, .note=0x3d, .vel=0x33}, // ch 0 : note=61 C#6 velocity=51
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3b}, // ch 0 : note=68 G#6 velocity=59
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3b}, // ch 0 : note=67 G6 velocity=59
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x3d}, // ch 0 : note=53 F5 velocity=61
    {.tick=   0, .note=0x3a, .vel=0x3d}, // ch 0 : note=58 A#5 velocity=61
    {.tick=   0, .note=0x3d, .vel=0x3d}, // ch 0 : note=61 C#6 velocity=61
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3b}, // ch 0 : note=68 G#6 velocity=59
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3b}, // ch 0 : note=67 G6 velocity=59
    {.tick=   0, .note=0x29, .vel=0x41}, // ch 0 : note=41 F4 velocity=65
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x37}, // ch 0 : note=67 G6 velocity=55
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=   0, .note=0x38, .vel=0x37}, // ch 0 : note=56 G#5 velocity=55
    {.tick=   0, .note=0x3c, .vel=0x37}, // ch 0 : note=60 C6 velocity=55
    {.tick=   0, .note=0x41, .vel=0x37}, // ch 0 : note=65 F6 velocity=55
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x38}, // ch 0 : note=41 F4 velocity=56
    {.tick=  24, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=  24, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3e}, // ch 0 : note=41 F4 velocity=62
    {.tick=  12, .note=0x48, .vel=0x36}, // ch 0 : note=72 C7 velocity=54
    {.tick=  12, .note=0x4a, .vel=0x3c}, // ch 0 : note=74 D7 velocity=60
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4c, .vel=0x45}, // ch 0 : note=76 E7 velocity=69
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  12, .note=0x4d, .vel=0x4c}, // ch 0 : note=77 F7 velocity=76
    {.tick=   0, .note=0x4c, .vel=0}, // note off 
    {.tick=  12, .note=0x4f, .vel=0x43}, // ch 0 : note=79 G7 velocity=67
    {.tick=   0, .note=0x4d, .vel=0}, // note off 
    {.tick=  12, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x53, .vel=0x49}, // ch 0 : note=83 B7 velocity=73
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=   0, .note=0x4f, .vel=0}, // note off 
    {.tick=  12, .note=0x4f, .vel=0x3b}, // ch 0 : note=79 G7 velocity=59
    {.tick=   0, .note=0x53, .vel=0}, // note off 
    {.tick=  12, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x37}, // ch 0 : note=77 F7 velocity=55
    {.tick=   0, .note=0x29, .vel=0x40}, // ch 0 : note=41 F4 velocity=64
    {.tick=   0, .note=0x4f, .vel=0}, // note off 
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4f, .vel=0x39}, // ch 0 : note=79 G7 velocity=57
    {.tick=   2, .note=0x4f, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x39}, // ch 0 : note=77 F7 velocity=57
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4f, .vel=0x3d}, // ch 0 : note=79 G7 velocity=61
    {.tick=   2, .note=0x4f, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3d}, // ch 0 : note=77 F7 velocity=61
    {.tick=   0, .note=0x29, .vel=0x3c}, // ch 0 : note=41 F4 velocity=60
    {.tick=  24, .note=0x38, .vel=0x38}, // ch 0 : note=56 G#5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=  21, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x39}, // ch 0 : note=77 F7 velocity=57
    {.tick=   3, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x39}, // ch 0 : note=76 E7 velocity=57
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  22, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x49, .vel=0x3e}, // ch 0 : note=73 C#7 velocity=62
    {.tick=   2, .note=0x49, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x3e}, // ch 0 : note=72 C7 velocity=62
    {.tick=   0, .note=0x29, .vel=0x3e}, // ch 0 : note=41 F4 velocity=62
    {.tick=  24, .note=0x38, .vel=0x31}, // ch 0 : note=56 G#5 velocity=49
    {.tick=   0, .note=0x3c, .vel=0x31}, // ch 0 : note=60 C6 velocity=49
    {.tick=   0, .note=0x41, .vel=0x31}, // ch 0 : note=65 F6 velocity=49
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x35}, // ch 0 : note=72 C7 velocity=53
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x35}, // ch 0 : note=71 B6 velocity=53
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  22, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x3e}, // ch 0 : note=68 G#6 velocity=62
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x3e}, // ch 0 : note=67 G6 velocity=62
    {.tick=   0, .note=0x29, .vel=0x40}, // ch 0 : note=41 F4 velocity=64
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x38}, // ch 0 : note=67 G6 velocity=56
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=   0, .note=0x38, .vel=0x36}, // ch 0 : note=56 G#5 velocity=54
    {.tick=   0, .note=0x3c, .vel=0x36}, // ch 0 : note=60 C6 velocity=54
    {.tick=   0, .note=0x41, .vel=0x36}, // ch 0 : note=65 F6 velocity=54
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x42}, // ch 0 : note=56 G#5 velocity=66
    {.tick=   0, .note=0x3c, .vel=0x42}, // ch 0 : note=60 C6 velocity=66
    {.tick=   0, .note=0x41, .vel=0x42}, // ch 0 : note=65 F6 velocity=66
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3e}, // ch 0 : note=41 F4 velocity=62
    {.tick=  24, .note=0x38, .vel=0x36}, // ch 0 : note=56 G#5 velocity=54
    {.tick=   0, .note=0x3c, .vel=0x36}, // ch 0 : note=60 C6 velocity=54
    {.tick=   0, .note=0x41, .vel=0x36}, // ch 0 : note=65 F6 velocity=54
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=  24, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3e}, // ch 0 : note=41 F4 velocity=62
    {.tick=  12, .note=0x48, .vel=0x3a}, // ch 0 : note=72 C7 velocity=58
    {.tick=  12, .note=0x4a, .vel=0x3d}, // ch 0 : note=74 D7 velocity=61
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=   0, .note=0x48, .vel=0}, // note off 
    {.tick=  12, .note=0x4c, .vel=0x3e}, // ch 0 : note=76 E7 velocity=62
    {.tick=   0, .note=0x4a, .vel=0}, // note off 
    {.tick=  12, .note=0x4d, .vel=0x42}, // ch 0 : note=77 F7 velocity=66
    {.tick=   0, .note=0x4c, .vel=0}, // note off 
    {.tick=  12, .note=0x4f, .vel=0x40}, // ch 0 : note=79 G7 velocity=64
    {.tick=   0, .note=0x4d, .vel=0}, // note off 
    {.tick=  12, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x53, .vel=0x49}, // ch 0 : note=83 B7 velocity=73
    {.tick=   0, .note=0x38, .vel=0x3c}, // ch 0 : note=56 G#5 velocity=60
    {.tick=   0, .note=0x3c, .vel=0x3c}, // ch 0 : note=60 C6 velocity=60
    {.tick=   0, .note=0x41, .vel=0x3c}, // ch 0 : note=65 F6 velocity=60
    {.tick=   0, .note=0x4f, .vel=0}, // note off 
    {.tick=  12, .note=0x4f, .vel=0x34}, // ch 0 : note=79 G7 velocity=52
    {.tick=   0, .note=0x53, .vel=0}, // note off 
    {.tick=  12, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x2d}, // ch 0 : note=77 F7 velocity=45
    {.tick=   0, .note=0x29, .vel=0x3c}, // ch 0 : note=41 F4 velocity=60
    {.tick=   0, .note=0x4f, .vel=0}, // note off 
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4f, .vel=0x36}, // ch 0 : note=79 G7 velocity=54
    {.tick=   2, .note=0x4f, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x36}, // ch 0 : note=77 F7 velocity=54
    {.tick=   0, .note=0x38, .vel=0x38}, // ch 0 : note=56 G#5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  22, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4f, .vel=0x3b}, // ch 0 : note=79 G7 velocity=59
    {.tick=   2, .note=0x4f, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3b}, // ch 0 : note=77 F7 velocity=59
    {.tick=   0, .note=0x29, .vel=0x3a}, // ch 0 : note=41 F4 velocity=58
    {.tick=  24, .note=0x38, .vel=0x38}, // ch 0 : note=56 G#5 velocity=56
    {.tick=   0, .note=0x3c, .vel=0x38}, // ch 0 : note=60 C6 velocity=56
    {.tick=   0, .note=0x41, .vel=0x38}, // ch 0 : note=65 F6 velocity=56
    {.tick=  21, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4d, .vel=0x3c}, // ch 0 : note=77 F7 velocity=60
    {.tick=   3, .note=0x4d, .vel=0}, // note off 
    {.tick=   0, .note=0x4c, .vel=0x3c}, // ch 0 : note=76 E7 velocity=60
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  22, .note=0x4c, .vel=0}, // note off 
    {.tick=   0, .note=0x49, .vel=0x36}, // ch 0 : note=73 C#7 velocity=54
    {.tick=   2, .note=0x49, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x36}, // ch 0 : note=72 C7 velocity=54
    {.tick=   0, .note=0x29, .vel=0x3d}, // ch 0 : note=41 F4 velocity=61
    {.tick=  24, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  21, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x48, .vel=0x36}, // ch 0 : note=72 C7 velocity=54
    {.tick=   3, .note=0x48, .vel=0}, // note off 
    {.tick=   0, .note=0x47, .vel=0x36}, // ch 0 : note=71 B6 velocity=54
    {.tick=  24, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x41}, // ch 0 : note=56 G#5 velocity=65
    {.tick=   0, .note=0x3c, .vel=0x41}, // ch 0 : note=60 C6 velocity=65
    {.tick=   0, .note=0x41, .vel=0x41}, // ch 0 : note=65 F6 velocity=65
    {.tick=  22, .note=0x47, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x39}, // ch 0 : note=68 G#6 velocity=57
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x39}, // ch 0 : note=67 G6 velocity=57
    {.tick=   0, .note=0x29, .vel=0x3c}, // ch 0 : note=41 F4 velocity=60
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x39}, // ch 0 : note=67 G6 velocity=57
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x39}, // ch 0 : note=65 F6 velocity=57
    {.tick=   0, .note=0x38, .vel=0x3b}, // ch 0 : note=56 G#5 velocity=59
    {.tick=   0, .note=0x3c, .vel=0x3b}, // ch 0 : note=60 C6 velocity=59
    {.tick=   0, .note=0x41, .vel=0x3b}, // ch 0 : note=65 F6 velocity=59
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3e}, // ch 0 : note=56 G#5 velocity=62
    {.tick=   0, .note=0x3c, .vel=0x3e}, // ch 0 : note=60 C6 velocity=62
    {.tick=   0, .note=0x41, .vel=0x3e}, // ch 0 : note=65 F6 velocity=62
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x3f}, // ch 0 : note=41 F4 velocity=63
    {.tick=  24, .note=0x38, .vel=0x36}, // ch 0 : note=56 G#5 velocity=54
    {.tick=   0, .note=0x3c, .vel=0x36}, // ch 0 : note=60 C6 velocity=54
    {.tick=   0, .note=0x41, .vel=0x36}, // ch 0 : note=65 F6 velocity=54
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x3a}, // ch 0 : note=56 G#5 velocity=58
    {.tick=   0, .note=0x3c, .vel=0x3a}, // ch 0 : note=60 C6 velocity=58
    {.tick=   0, .note=0x41, .vel=0x3a}, // ch 0 : note=65 F6 velocity=58
    {.tick=  22, .note=0x44, .vel=0x3a}, // ch 0 : note=68 G#6 velocity=58
    {.tick=   2, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x62}, // ch 0 : note=70 A#6 velocity=98
    {.tick=   0, .note=0x22, .vel=0x56}, // ch 0 : note=34 A#3 velocity=86
    {.tick=  24, .note=0x35, .vel=0x61}, // ch 0 : note=53 F5 velocity=97
    {.tick=   0, .note=0x3a, .vel=0x61}, // ch 0 : note=58 A#5 velocity=97
    {.tick=   0, .note=0x3d, .vel=0x61}, // ch 0 : note=61 C#6 velocity=97
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x5d}, // ch 0 : note=68 G#6 velocity=93
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x5d}, // ch 0 : note=67 G6 velocity=93
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x5e}, // ch 0 : note=53 F5 velocity=94
    {.tick=   0, .note=0x3a, .vel=0x5e}, // ch 0 : note=58 A#5 velocity=94
    {.tick=   0, .note=0x3d, .vel=0x5e}, // ch 0 : note=61 C#6 velocity=94
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x6c}, // ch 0 : note=68 G#6 velocity=108
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x46, .vel=0x6c}, // ch 0 : note=70 A#6 velocity=108
    {.tick=   0, .note=0x22, .vel=0x65}, // ch 0 : note=34 A#3 velocity=101
    {.tick=  24, .note=0x35, .vel=0x57}, // ch 0 : note=53 F5 velocity=87
    {.tick=   0, .note=0x3a, .vel=0x57}, // ch 0 : note=58 A#5 velocity=87
    {.tick=   0, .note=0x3d, .vel=0x57}, // ch 0 : note=61 C#6 velocity=87
    {.tick=  22, .note=0x46, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x5f}, // ch 0 : note=68 G#6 velocity=95
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x5f}, // ch 0 : note=67 G6 velocity=95
    {.tick=  24, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0x61}, // ch 0 : note=53 F5 velocity=97
    {.tick=   0, .note=0x3a, .vel=0x61}, // ch 0 : note=58 A#5 velocity=97
    {.tick=   0, .note=0x3d, .vel=0x61}, // ch 0 : note=61 C#6 velocity=97
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x44, .vel=0x5f}, // ch 0 : note=68 G#6 velocity=95
    {.tick=   2, .note=0x44, .vel=0}, // note off 
    {.tick=   0, .note=0x22, .vel=0}, // note off 
    {.tick=   0, .note=0x35, .vel=0}, // note off 
    {.tick=   0, .note=0x3a, .vel=0}, // note off 
    {.tick=   0, .note=0x3d, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x5f}, // ch 0 : note=67 G6 velocity=95
    {.tick=   0, .note=0x29, .vel=0x6f}, // ch 0 : note=41 F4 velocity=111
    {.tick=  22, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x43, .vel=0x58}, // ch 0 : note=67 G6 velocity=88
    {.tick=   2, .note=0x43, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0x58}, // ch 0 : note=65 F6 velocity=88
    {.tick=   0, .note=0x38, .vel=0x5c}, // ch 0 : note=56 G#5 velocity=92
    {.tick=   0, .note=0x3c, .vel=0x5c}, // ch 0 : note=60 C6 velocity=92
    {.tick=   0, .note=0x41, .vel=0x5c}, // ch 0 : note=65 F6 velocity=92
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x60}, // ch 0 : note=56 G#5 velocity=96
    {.tick=   0, .note=0x3c, .vel=0x60}, // ch 0 : note=60 C6 velocity=96
    {.tick=   0, .note=0x41, .vel=0x60}, // ch 0 : note=65 F6 velocity=96
    {.tick=  24, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0x62}, // ch 0 : note=41 F4 velocity=98
    {.tick=  24, .note=0x38, .vel=0x59}, // ch 0 : note=56 G#5 velocity=89
    {.tick=   0, .note=0x3c, .vel=0x59}, // ch 0 : note=60 C6 velocity=89
    {.tick=   0, .note=0x41, .vel=0x59}, // ch 0 : note=65 F6 velocity=89
    {.tick=  48, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0x63}, // ch 0 : note=56 G#5 velocity=99
    {.tick=   0, .note=0x3c, .vel=0x63}, // ch 0 : note=60 C6 velocity=99
    {.tick=   0, .note=0x41, .vel=0x63}, // ch 0 : note=65 F6 velocity=99
    {.tick=  24, .note=0x41, .vel=0}, // note off 
    {.tick=   0, .note=0x29, .vel=0}, // note off 
    {.tick=   0, .note=0x38, .vel=0}, // note off 
    {.tick=   0, .note=0x3c, .vel=0}, // note off 
    {.tick=   0, .note=0x41, .vel=0}, // note off 
};
const int track_piano_len = 1623; // 6 kb
// format 1,ntracks 2,resolution 256, 1623 events
