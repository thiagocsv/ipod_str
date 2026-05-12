#ifndef LIBMANAGER_H
#define LIBMANAGER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MUSIC_PATH "/sdcard/music/"
#define INDEX_PATH "/sdcard/music/index.bin"
#define PATH_SIZE 26

#pragma pack(push, 1)
typedef struct
{
    char title[46];
    char artist[40];
    char album[36];
    uint8_t track_num;
    uint8_t genre_id;
    uint8_t folder_id;
    char filename[3];
} song_t;
#pragma (pop)

bool loadSong(uint32_t index, song_t song, char *out_path);

uint32_t totalSongs();

#endif
