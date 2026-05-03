#include "libManager.h"

static bool ismp3(const char *filename)
{
    int len = strlen(filename);
    if (len < 5)
        return false;
    return strcasecmp(filename + len - 4, ".mp3") == 0;
}

int totalSongs(void)
{
    FILE *f = fopen(CSV_PATH, "r");
    if (!f)
        return -1;

    int count = -1; // começa em -1 para descontar o header
    char c;
    while ((c = fgetc(f)) != EOF)
        if (c == '\n')
            count++;

    fclose(f);
    return count < 0 ? 0 : count;
}

void getSongs(char *param)
{
    FILE *file = fopen(CSV_PATH, "r");

    while(file != NULL)
    {

    }
}

void buildLib(void *params)
{
    FILE *file = fopen(CSV_PATH, "w");
    FILE *artists = fopen(ARTIST_PATH, "w");
    FILE *albums = fopen(ALBUM_PATH, "w");
    FILE *genres = fopen(GENRE_PATH, "w");

    DIR *d;
    struct dirent *dir;

    d = opendir("/sdcard/music/");

    fprintf(file, "Artist; Album; Title; Genre; Path\n");

    while ((dir = readdir(d)) != NULL)
    {
        char path[] = "/sdcard/music/" + dir->d_name;
        if (ismp3(path))
        {
            TagLib_File *f = taglib_file_new(path);
            if (f != NULL)
            {
                TagLib_Tag *tag = taglib_file_tag(f);

                fprintf(file, "%s;%s;%s;%s;%s;\n", taglib_tag_artist(tag), taglib_tag_album(tag), taglib_tag_title(tag), taglib_tag_genre(tag), dir->d_name);
                fprintf(artists, "%s\n", taglib_tag_artist(tag));
                fprintf(albums, "%s\n", taglib_tag_album(tag));
                fprintf(genres, "%s\n", taglib_tag_genre(tag));
            }

            taglib_tag_free_strings();
            taglib_file_free(f);
        }
    }

    closedir(d);
    fclose(file);
    fclose(artists);
    fclose(albums);
    fclose(genres);
}