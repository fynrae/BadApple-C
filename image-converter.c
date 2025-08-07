#define USE_MULTICORE 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

#if USE_MULTICORE == 1
#include <omp.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_PATH_LENGTH 4096

void rgb_to_hex(char hex_str[7], unsigned char r, unsigned char g, unsigned char b) {
    snprintf(hex_str, 7, "%02x%02x%02x", r, g, b);
}

int is_number(const char* s) {
    if (s == NULL || *s == '\0') return 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_folder> <output_folder>\n", argv[0]);
        return 1;
    }

    const char* input_dir_path = argv[1];
    const char* output_dir_path = argv[2];
    
    mkdir(output_dir_path, 0755);

#if USE_MULTICORE == 1
    
    printf("Running in multi-core mode.\n");

    DIR* input_dir = opendir(input_dir_path);
    if (input_dir == NULL) {
        fprintf(stderr, "Error: Could not open input directory '%s'\n", input_dir_path);
        return 1;
    }

    char** file_list = NULL;
    int file_count = 0;
    struct dirent* entry;

    while ((entry = readdir(input_dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            file_count++;
            file_list = realloc(file_list, file_count * sizeof(char*));
            if (file_list == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory for file list.\n");
                return 1;
            }
            file_list[file_count - 1] = strdup(entry->d_name);
        }
    }
    closedir(input_dir);

    #pragma omp parallel for
    for (int i = 0; i < file_count; i++) {
        char* d_name = file_list[i];

        char filename_stem[256];
        strncpy(filename_stem, d_name, sizeof(filename_stem) - 1);
        filename_stem[sizeof(filename_stem) - 1] = '\0';
        
        char* dot = strrchr(filename_stem, '.');
        if (dot != NULL) *dot = '\0';

        if (!is_number(filename_stem)) {
            continue;
        }

        char input_filepath[MAX_PATH_LENGTH];
        char output_filepath[MAX_PATH_LENGTH];
        snprintf(input_filepath, sizeof(input_filepath), "%s/%s", input_dir_path, d_name);
        snprintf(output_filepath, sizeof(output_filepath), "%s/%s", output_dir_path, filename_stem);

        int width, height, channels;
        unsigned char* data = stbi_load(input_filepath, &width, &height, &channels, 3);
        if (data) {
            FILE* out_file = fopen(output_filepath, "w");
            if (out_file) {
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        int idx = (y * width + x) * 3;
                        char hex_str[7];
                        rgb_to_hex(hex_str, data[idx], data[idx + 1], data[idx + 2]);
                        fprintf(out_file, "%s%c", hex_str, (x == width - 1) ? '\0' : ' ');
                    }
                    fprintf(out_file, "\n");
                }
                fclose(out_file);
                printf("Processed: %s on thread %d\n", d_name, omp_get_thread_num());
            }
            stbi_image_free(data);
        }
    }

    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }
    free(file_list);

#else

    printf("Running in single-core mode.\n");

    DIR* input_dir = opendir(input_dir_path);
    if (input_dir == NULL) {
        fprintf(stderr, "Error: Could not open input directory '%s'\n", input_dir_path);
        return 1;
    }

    struct dirent* entry;
    while ((entry = readdir(input_dir)) != NULL) {
        if (entry->d_type != DT_REG) continue;

        char filename_stem[256];
        strncpy(filename_stem, entry->d_name, sizeof(filename_stem) -1);
        filename_stem[sizeof(filename_stem) - 1] = '\0';

        char* dot = strrchr(filename_stem, '.');
        if (dot != NULL) *dot = '\0';
        
        if (!is_number(filename_stem)) continue;

        char input_filepath[MAX_PATH_LENGTH];
        char output_filepath[MAX_PATH_LENGTH];
        snprintf(input_filepath, sizeof(input_filepath), "%s/%s", input_dir_path, entry->d_name);
        snprintf(output_filepath, sizeof(output_filepath), "%s/%s", output_dir_path, filename_stem);

        int width, height, channels;
        unsigned char* data = stbi_load(input_filepath, &width, &height, &channels, 3);
        if (data) {
            FILE* out_file = fopen(output_filepath, "w");
            if (out_file) {
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        int idx = (y * width + x) * 3;
                        char hex_str[7];
                        rgb_to_hex(hex_str, data[idx], data[idx + 1], data[idx + 2]);
                        fprintf(out_file, "%s%c", hex_str, (x == width - 1) ? '\0' : ' ');
                    }
                    fprintf(out_file, "\n");
                }
                fclose(out_file);
                printf("Processed: %s\n", entry->d_name);
            }
            stbi_image_free(data);
        }
    }
    closedir(input_dir);

#endif

    printf("All images converted.\n");
    return 0;
}