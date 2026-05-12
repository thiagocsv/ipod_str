#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <tag_c.h>

#define TOTAL_PASTAS 50
#define BUFFER_SIZE 8192

// Estrutura perfeitamente alinhada para 128 bytes (4 músicas por setor de 512 bytes do SD)
#pragma pack(push, 1)
typedef struct {
    char title[46];
    char artist[40];
    char album[36];
    uint8_t track_number;
    uint8_t genre_id;
    uint8_t folder_id;
    char filename[3];
} TrackRecord;
#pragma pack(pop)

void gen_name(char *saida) {
    for (int i = 0; i < 3; i++) {
        saida[i] = 'A' + (rand() % 26);
    }
}

void preparar_pastas(const char *base_path) {
    char path[512];

    snprintf(path, sizeof(path), "%s/music", base_path);
    mkdir(path, 0777);

    // Cria as subpastas F00 a F49
    for (int i = 0; i < TOTAL_PASTAS; i++) {
        snprintf(path, sizeof(path), "%s/music/F%02d", base_path, i);
        mkdir(path, 0777);
    }
}

// Copia o arquivo em blocos de 8KB
int copy_file(const char *origem, const char *destino) {
    FILE *in = fopen(origem, "rb");
    if (!in) return 0;
    FILE *out = fopen(destino, "wb");
    if (!out)
    {
        fclose(in);
        return 0;
    }

    uint8_t buffer[BUFFER_SIZE];
    size_t bytes_lidos;

    while ((bytes_lidos = fread(buffer, 1, sizeof(buffer), in)) > 0)
        fwrite(buffer, 1, bytes_lidos, out);

    fclose(in);
    fclose(out);
    return 1;
}

// Extrai as tags do MP3 e joga na Struct
void extract_metadata(const char *filepath, TrackRecord *track) {
    memset(track, 0, sizeof(TrackRecord)); // Zera toda a struct com \0

    TagLib_File *file = taglib_file_new(filepath);
    if (file != NULL && taglib_file_is_valid(file))
    {
        TagLib_Tag *tag = taglib_file_tag(file);

        if (tag != NULL)
        {
            char *title = taglib_tag_title(tag);
            char *artist = taglib_tag_artist(tag);
            char *album = taglib_tag_album(tag);

            if (title && strlen(title) > 0) strncpy(track->title, title, 45);
            else {
                // Fallback: Nome do arquivo
                strncpy(track->title, filepath, 45);
                char *dot = strrchr(track->title, '.');
                if (dot) *dot = '\0';
            }

            if (artist && strlen(artist) > 0) strncpy(track->artist, artist, 39);
            else strncpy(track->artist, "Unknown", 39);

            if (album && strlen(album) > 0) strncpy(track->album, album, 35);
            else strncpy(track->album, "Unknown", 35);

            track->track_number = (uint8_t)taglib_tag_track(tag);

            // Mapeamento simples de genero (255 = Unknown)
            char *genre = taglib_tag_genre(tag);
            if (genre && strstr(genre, "Rock")) track->genre_id = 17;
            else if (genre && strstr(genre, "Pop")) track->genre_id = 13;
            else track->genre_id = 255;

            taglib_tag_free_strings();
        }
        taglib_file_free(file);
    } else {
        // Fallback critico
        strncpy(track->title, filepath, 45);
        strncpy(track->artist, "Unknown", 39);
        strncpy(track->album, "Unknown", 35);
    }
}

// Verifica duplicatas comparando Artista e Titulo
int musica_ja_existe(TrackRecord *db, uint32_t total, const char *artist, const char *title)
{
    for (uint32_t i = 0; i < total; i++)
        if (strncmp(db[i].artist, artist, 40) == 0 && strncmp(db[i].title, title, 46) == 0)
            return 1;

    return 0;
}

// Verifica se o nome de 3 letras ja existe na pasta especifica
int nome_ja_usado(TrackRecord *db, uint32_t total, uint8_t folder_id, const char *nome3)
{
    for (uint32_t i = 0; i < total; i++)
        if (db[i].folder_id == folder_id && strncmp(db[i].filename, nome3, 3) == 0)
            return 1;

    return 0;
}

void ordenate(FILE *index)
{

}

int main()
{
    srand(time(NULL));

    char dest_path[256];
    printf("Digite o caminho do cartao SD (Ex: /run/media/usuario/SD ou ./sdcard):\n> ");

    if (fgets(dest_path, sizeof(dest_path), stdin) == NULL)
    {
        printf("Erro ao ler entrada.\n");
        return 1;
    }

    // Remove a quebra de linha do fgets
    dest_path[strcspn(dest_path, "\n")] = '\0';

    preparar_pastas(dest_path);

    TrackRecord *banco_dados = NULL;
    uint32_t total_antigo = 0;

    char db_filepath[512];
    snprintf(db_filepath, sizeof(db_filepath), "%s/music/index.bin", dest_path);

    FILE *db_file = fopen(db_filepath, "rb");

    if (db_file)
    {
        if (fread(&total_antigo, sizeof(uint32_t), 1, db_file) == 1)
        {
            banco_dados = malloc(total_antigo * sizeof(TrackRecord));
            fread(banco_dados, sizeof(TrackRecord), total_antigo, db_file);
            printf("Banco carregado! %u musicas encontradas em %s\n", total_antigo, dest_path);
        }
        fclose(db_file);
    }

    DIR *dir = opendir(".");

    if (!dir)
    {
        printf("Erro ao ler o diretorio atual.\n");
        return 1;
    }

    struct dirent *entrada;
    uint32_t novas_adicionadas = 0;

    printf("\nVarrendo musicas da pasta atual...\n");

    // 2. Processa cada arquivo MP3
    while ((entrada = readdir(dir)) != NULL)
    {
        char *ext = strrchr(entrada->d_name, '.');
        if (ext && (strcmp(ext, ".mp3") == 0 || strcmp(ext, ".MP3") == 0)) {

            TrackRecord tmp_track;
            extract_metadata(entrada->d_name, &tmp_track);

            if (musica_ja_existe(banco_dados, total_antigo + novas_adicionadas, tmp_track.artist, tmp_track.title))
            {
                printf("Ignorado (ja existe): [%.15s - %.15s]\n", tmp_track.artist, tmp_track.title);
                continue;
            }

            banco_dados = realloc(banco_dados, (total_antigo + novas_adicionadas + 1) * sizeof(TrackRecord));
            TrackRecord *nova_track = &banco_dados[total_antigo + novas_adicionadas];

            // Copia os metadados extraidos para o ponteiro real
            memcpy(nova_track, &tmp_track, sizeof(TrackRecord));

            // Define a pasta (Round-Robin)
            nova_track->folder_id = (total_antigo + novas_adicionadas) % TOTAL_PASTAS;

            // Gera o nome de 3 letras evitando colisoes
            char caminho_destino[512];
            FILE *teste_colisao = NULL;

            do
            {
                gen_name(nova_track->filename);
                snprintf(caminho_destino, sizeof(caminho_destino), "%s/music/F%02d/%.3s.mp3",
                         dest_path, nova_track->folder_id, nova_track->filename);

                teste_colisao = fopen(caminho_destino, "r");
                if (teste_colisao) fclose(teste_colisao);

            } while (teste_colisao != NULL || nome_ja_usado(banco_dados, total_antigo + novas_adicionadas, nova_track->folder_id, nova_track->filename));

                // Copia fisicamente o arquivo para o SD
                printf("Adicionado: [%.15s - %.15s] -> F%02d/%.4s.mp3\n",
                       nova_track->artist, nova_track->title, nova_track->folder_id, nova_track->filename);
                copy_file(entrada->d_name, caminho_destino);

                novas_adicionadas++;
        }
    }
    closedir(dir);

    // 3. Sobrescreve o index.bin no SD com os dados novos
    if (novas_adicionadas > 0)
    {
        uint32_t total_final = total_antigo + novas_adicionadas;
        db_file = fopen(db_filepath, "wb");

        if (db_file)
        {
            fwrite(&total_final, sizeof(uint32_t), 1, db_file);
            fwrite(banco_dados, sizeof(TrackRecord), total_final, db_file);

            ordenate(db_file);

            fclose(db_file);
            printf("\nSucesso! %u musicas adicionadas. Total no aparelho: %u\n", novas_adicionadas, total_final);
        }
    }
    else
        printf("\nNenhuma musica nova encontrada para copiar.\n");

    if (banco_dados)
        free(banco_dados);

    return 0;
}
