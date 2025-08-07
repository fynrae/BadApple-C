#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_mixer.h>
#include <string.h>

#define WIDTH 480
#define HEIGHT 360
#define FRAME_FOLDER "./output/"

#define TARGET_FPS 30.00f
#define FRAME_TIME (1000.0f / TARGET_FPS)

uint32_t hex_to_color(const char* hex) {
    uint32_t color = 0;
    sscanf(hex, "%x", &color);
    return 0xFF000000 | color;
}

int load_frame(const char* filename, uint32_t* pixels) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    char line[8192];
    int y = 0;
    while (fgets(line, sizeof(line), f) && y < HEIGHT) {
        int x = 0;
        char* token = strtok(line, " \t\n\r");
        while (token && x < WIDTH) {
            pixels[y * WIDTH + x] = hex_to_color(token);
            token = strtok(NULL, " \t\n\r");
            x++;
        }
        y++;
    }

    fclose(f);
    return 1;
}

Uint32 get_simulated_music_position_ms(Uint32 start_time) {
    return SDL_GetTicks() - start_time;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* win = SDL_CreateWindow("Bad Apple, but it's in OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    uint32_t* pixels = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    if (!pixels) {
        fprintf(stderr, "Failed to allocate memory for pixels!\n");
        return 1;
    }

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Music *music = Mix_LoadMUS("./badapple.mp3");
    Mix_PlayMusic(music, -1);

    int running = 1;
    SDL_Event e;
    int frame_number = 1;
    char filepath[256];

    Uint32 program_start_ticks = SDL_GetTicks();
    double video_time_ms = 0.0;
    double audio_time_ms = 0.0;

    SDL_GL_SetSwapInterval(0);

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
        }

        audio_time_ms = (double)get_simulated_music_position_ms(program_start_ticks);
        
        video_time_ms = (frame_number - 1) * FRAME_TIME;

        if (video_time_ms > audio_time_ms) {
            double delay_ms = video_time_ms - audio_time_ms;
            SDL_Delay((Uint32)delay_ms);
        }

        snprintf(filepath, sizeof(filepath), FRAME_FOLDER"%d", frame_number);
        if (!load_frame(filepath, pixels)) {
            printf("Could not load frame %d. Looping back.\n", frame_number);
            frame_number = 1;
            program_start_ticks = SDL_GetTicks(); 
            continue;
        }

        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frame_number++;
    }

    free(pixels);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
