#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <gmime/gmime.h>

#define PATH "./"

// heel simpele MIME herkenning: kijk of header From: of Date: voorkomt
int looks_like_mime(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) return 0;

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '\n' || line[0] == '\r') break;  // einde headers
        if (strncmp(line, "From:", 5) == 0 ||
            strncmp(line, "Date:", 5) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void process_file(const char *filepath) {
    if (!looks_like_mime(filepath))
        return;

    GMimeStream *fstream = g_mime_stream_file_open(filepath, "r", NULL);
    if (!fstream)
        return;

    GMimeParser *parser = g_mime_parser_new_with_stream(fstream);

    /* Belangrijk: alleen headers parsen */
    // g_mime_parser_set_header_only(parser, TRUE);

    GMimeMessage *msg = g_mime_parser_construct_message(parser, g_mime_parser_options_get_default());
    if (msg) {
        const char *from = g_mime_object_get_header((GMimeObject *) msg, "From");
        const char *date = g_mime_object_get_header((GMimeObject *) msg, "Date");

        printf("FILE: %s\n", filepath);
        if (from) printf("  From: %s\n", from);
        if (date) printf("  Date: %s\n", date);

        g_object_unref(msg);
    }

    g_object_unref(parser);
    g_object_unref(fstream);
}

void walk(const char *directory) {
    DIR *dir = opendir(directory);
    if (!dir) return;

    struct dirent *entry;
    char pathbuf[4096];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(pathbuf, sizeof(pathbuf), "%s/%s", directory, entry->d_name);

        struct stat st;
        if (stat(pathbuf, &st) != 0)
            continue;

        if (S_ISDIR(st.st_mode)) {
            walk(pathbuf);
        }
        else if (S_ISREG(st.st_mode)) {
            process_file(pathbuf);
        }
    }

    closedir(dir);
}

int main(int argc, char **argv) {
    g_mime_init();

    walk(PATH);

    g_mime_shutdown();
    return 0;
}

// gcc -o rename-mails rename-mails.c $(pkg-config --cflags --libs gmime-3.0) -O2