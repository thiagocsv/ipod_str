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

    int count = -1;
    char c;
    while ((c = fgetc(f)) != EOF)
        if (c == '\n')
            count++;

    fclose(f);
    return count < 0 ? 0 : count;
}

void getSong(int target, song_t *song)
{
    FILE *file = fopen (CSV_PATH, "r");
    if(!file)
        return false;

    char line[300];
    int current = -1;

    while(fgets(line, sizeof(line), file) != NULL)
    {
        if(current == target)
        {
            char *str = strtok(line, ";");

            strncpy(out_song->artist, token, sizeof(out_song->artist) - 1);
            token = strtok(NULL, ";");
            strncpy(out_song->artist, token, sizeof(out_song->album) - 1);
            token = strtok(NULL, ";");
            strncpy(out_song->artist, token, sizeof(out_song->title) - 1);
            token = strtok(NULL, ";");
            strncpy(out_song->artist, token, sizeof(out_song->genre) - 1);
            token = strtok(NULL, ";");

            snprintf(out_song->path, sizeof(out_song->path), "%s%s", MUSIC_DIR, token);

            fclose(file);
            return true;
        }
        current++;
    }
    fclose(file);
    return false;
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

    if (d == NULL || file == NULL || artists == NULL || albums == NULL || genres == NULL) {
        if(d) closedir(d);
        if(file) fclose(file);
        if(artists) fclose(artists);
        if(albums) fclose(albums);
        if(genres) fclose(genres);
        return;
    }

    fprintf(file, "Artist; Album; Title; Genre; Path\n");

    while ((dir = readdir(d)) != NULL)
    {
        char path[128];
        snprintf(path, sizeof(path), "/sdcard/music/%s", dir->d_name);

        if (ismp3(path))
        {
            TagLib_File *f = taglib_file_new(path);
            if (f != NULL)
            {
                TagLib_Tag *tag = taglib_file_tag(f);

                const char *artist = taglib_tag_artist(tag) ? taglib_tag_artist(tag) : "Unknown";
                const char *album  = taglib_tag_album(tag)  ? taglib_tag_album(tag)  : "Unknown";
                const char *title  = taglib_tag_title(tag)  ? taglib_tag_title(tag)  : dir->d_name;
                const char *genre  = taglib_tag_genre(tag)  ? taglib_tag_genre(tag)  : "Unknown";

                fprintf(file, "%s;%s;%s;%s;%s;\n", artist, album, title, genre, dir->d_name);
                fprintf(artists, "%s\n", artist);
                fprintf(albums, "%s\n", album);
                fprintf(genres, "%s\n", genre);
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
