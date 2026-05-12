#include "libManager.h"

bool loadSong(uint32_t index, song_t song, char *out_path)
{
    FILE *file = fopen(INDEX_PATH, "rb");
    if(!file)
        return false;

    long offset = 4 + (index * sizeof(song));

    if(fseek(file, offset, SEEK_SET) != 0)
    {
        fclose(file);
        return false;
    }

    size_t block_read = fread(song, 1, sizeof(song), f);
    fclose(file);

    if(block_read != sizeof(song))
        return false;

    sprintf(out_path, "/sdcard/music/F%02d/%.3s.mp3", song->folder_id, song->filename);

    return true;
}

uint32_t totalSongs()
{
    FILE *file = fopen(INDEX_PATH, "rb");
    if(!file)
        return 0;

    uint32_t total = 0;
    fread(&total, 4, 1, file);
    return total;
}


char **getData(char **out_list)
{

}
