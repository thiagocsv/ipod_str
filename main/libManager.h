#ifndef LIBMANAGER_H
#define LIBMANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <taglib/tag_c.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

#define CSV_PATH "/sdcard/data/tags_library.csv"
#define ARTIST_PATH "/sdcard/data/index/artists.idx"
#define ALBUM_PATH "/sdcard/data/index/album.idx"
#define GENRE_PATH "/sdcard/data/index/genre.idx"
#define MUSIC_DIR "/sdcard/music/"

typedef struct
{
    char artist[32];
    char album[32];
    char title[64];
    char genre[32];
    char path[128];
} song_t;

int totalSongs(void);

bool getSong(int index, song_t target);

void buildLib(void *params);

#endif
