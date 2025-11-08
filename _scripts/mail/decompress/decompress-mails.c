#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zstd.h>

#define MAGIC_SIZE 4
const unsigned char ZSTD_MAGIC[MAGIC_SIZE] = {0x28, 0xB5, 0x2F, 0xFD};

int is_zstd(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return 0;

    unsigned char magic[MAGIC_SIZE];
    size_t n = fread(magic, 1, MAGIC_SIZE, f);
    fclose(f);

    if (n != MAGIC_SIZE) return 0;
    return memcmp(magic, ZSTD_MAGIC, MAGIC_SIZE) == 0;
}

int decompress_file_stream(const char* filepath) {
    // Open origineel bestand
    FILE* fin = fopen(filepath, "rb");
    if (!fin) {
        fprintf(stderr, "[FOUT] Kan bestand niet openen: %s\n", filepath);
        return 0;
    }

    // Tijdelijk bestand
    char tmp_path[4096];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", filepath);
    FILE* fout = fopen(tmp_path, "wb");
    if (!fout) {
        fprintf(stderr, "[FOUT] Kan tijdelijk bestand niet openen: %s\n", tmp_path);
        fclose(fin);
        return 0;
    }

    // Streaming decompressie
    ZSTD_DStream* dstream = ZSTD_createDStream();
    if (!dstream) {
        fprintf(stderr, "[FOUT] Kan ZSTD stream niet aanmaken\n");
        fclose(fin);
        fclose(fout);
        return 0;
    }
    if (ZSTD_isError(ZSTD_initDStream(dstream))) {
        fprintf(stderr, "[FOUT] Kan ZSTD stream niet initialiseren\n");
        ZSTD_freeDStream(dstream);
        fclose(fin);
        fclose(fout);
        return 0;
    }

    size_t inSize = ZSTD_DStreamInSize();
    size_t outSize = ZSTD_DStreamOutSize();
    void* inBuffer = malloc(inSize);
    void* outBuffer = malloc(outSize);
    if (!inBuffer || !outBuffer) {
        fprintf(stderr, "[FOUT] Niet genoeg geheugen\n");
        free(inBuffer); free(outBuffer);
        ZSTD_freeDStream(dstream);
        fclose(fin); fclose(fout);
        return 0;
    }

    size_t readBytes;
    while ((readBytes = fread(inBuffer, 1, inSize, fin)) > 0) {
        ZSTD_inBuffer input = { inBuffer, readBytes, 0 };
        while (input.pos < input.size) {
            ZSTD_outBuffer output = { outBuffer, outSize, 0 };
            size_t ret = ZSTD_decompressStream(dstream, &output, &input);
            if (ZSTD_isError(ret)) {
                fprintf(stderr, "[FOUT] %s kon niet uitgepakt worden: %s\n", filepath, ZSTD_getErrorName(ret));
                free(inBuffer); free(outBuffer);
                ZSTD_freeDStream(dstream);
                fclose(fin); fclose(fout);
                remove(tmp_path);
                return 0;
            }
            if (output.pos > 0) {
                fwrite(outBuffer, 1, output.pos, fout);
            }
        }
    }

    free(inBuffer);
    free(outBuffer);
    ZSTD_freeDStream(dstream);
    fclose(fin);
    fclose(fout);

    // Overschrijf origineel
    if (rename(tmp_path, filepath) != 0) {
        fprintf(stderr, "[FOUT] Kan origineel bestand niet vervangen: %s\n", filepath);
        remove(tmp_path);
        return 0;
    }

    printf("[OK] Uitgepakt: %s\n", filepath);
    return 1;
}

void process_maildir_cur(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            if (strcmp(entry->d_name, "cur") == 0) {
                // verwerk bestanden in cur
                DIR* curdir = opendir(fullpath);
                if (!curdir) continue;
                struct dirent* mail;
                while ((mail = readdir(curdir)) != NULL) {
                    if (strcmp(mail->d_name, ".") == 0 || strcmp(mail->d_name, "..") == 0)
                        continue;

                    char mailpath[4096];
                    snprintf(mailpath, sizeof(mailpath), "%s/%s", fullpath, mail->d_name);

                    struct stat mst;
                    if (stat(mailpath, &mst) == -1 || !S_ISREG(mst.st_mode))
                        continue;

                    if (!is_zstd(mailpath)) {
                        printf("[SKIP] Geen zstd-bestand: %s\n", mailpath);
                        continue;
                    }
                    decompress_file_stream(mailpath);
                }
                closedir(curdir);
            } else {
                // recursief
                process_maildir_cur(fullpath);
            }
        }
    }
    closedir(dir);
}

int main() {
    const char* maildir = "./"; // pas aan
    process_maildir_cur(maildir);
    return 0;
}

