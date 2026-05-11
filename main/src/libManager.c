#include "libManager.h"

static bool ismp3(const char *filename)
{
    int len = strlen(filename);
    if (len < 5)
        return false;
    return strcasecmp(filename + len - 4, ".mp3") == 0;
}

int totalLines(char *path)
{
    FILE *f = fopen(path, "r");
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

void loadBuffer(char *path, uint32_t *bufout, int size, int offset)
{
    FILE *file = fopen(*path, "rb");
    int index = 0;
    uint32_t buffer;

    for (int i = 0; i < size; i++)
        bufout[i] = 0xFFFFFFFF;

    fseek(file, offset * sizeof(uint32_t), SEEK_SET);

    fread(bufout, sizeof(uint32_t), size, file);

    fclose(file);
}

bool idxExists(char *path, uint32_t search)
{
    FILE *file = fopen(path, "rb");
    if (!file) = return false;

    uint32_t buffer;

    while(fread(&buffer, sizeof(uint32_t), 1, file) == 1)
        if(buffer == search)
        {
            fclose(file);
            return true;
        }

    return false;
}

void buildLib(void *params)
{
    FILE *file = fopen(CSV_PATH, "w");
    FILE *artists = fopen(ARTIST_PATH, "wb");
    FILE *albums = fopen(ALBUM_PATH, "wb");
    FILE *genres = fopen(GENRE_PATH, "wb");

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

    while ((dir = readdir(d)) != NULL)
    {
        char path[128];

        snprintf(path, sizeof(path), "%s", dir->d_name);

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

                int id_art = 0, id_alb = 0, id_gen = 0;

                int written = fprintf(file, "%04d;%04d;%04d;%s;%s;%s;%s;%s", id_art, id_alb, id_gen, artist, album, genre, title, path);

                int padding = 255 - written;

                while(padding--)
                    fputc('\0', file);

                fputc('\n', file);

                if(idxExists(id_art, ARTIST_PATH))
                    fprintf(artists, "%s\n", artist);
                if(idxExists(id_alb, ALBUMS_PATH))
                    fprintf(albums, "%s\n", album);
                if(idxExists(id_art, GENRES_PATH))
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
