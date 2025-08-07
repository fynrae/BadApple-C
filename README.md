
# Bad Apple!! C
Bad Apple, but it is written from scratch in C!

## Build
### Dependencies

To build this project, you will need:
- [SDL2](https://github.com/libsdl-org/SDL)
- [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer)

Optional:

- [OpenMP](https://www.openmp.org) - for multi-threading support when converting images

### Enabling Multi-threading
To enable multi-threading for the image converter, edit `image-converter.c`:

Change

```C
#define USE_MULTICORE 0
```

to

```C
#define USE_MULTICORE 1
```

### To build
#### main.c
`main.c` is the main player for this project, the instruction to build it is below:

- If you have `sdl2-config` available (comes with `libsdl2-dev`):

```bash
gcc main.c -o main $(sdl2-config --cflags --libs) -lSDL2_mixer
```

- If `sdl2-config` isn’t installed, you can explicitly link:

```bash
gcc main.c -o main -lSDL2 -lSDL2_mixer
```

#### image-converter.c
`image-converter.c` is the image converter used to convert image file into text file that the main player can read and display, the instruction to build it is below:

- Multi-core build (OpenMP enabled)

```bash
gcc image-converter.c -o image-converter -fopenmp -lm
```

- Single-core build (OpenMP disabled)
```bash
gcc image-converter.c -o image-converter -lm
```

## Running
### Converter
To convert images, you can execute the following command:
```bash
./image-converter <input_folder> <output_folder>
``` 

- `<input_folder>` is the folder containing your source images (numbered sequentially, e.g., `1.png`, `2.png`, `3.png`, …)
- `<output_folder>` is the folder where all converted frame files will be stored

### Player
To run the player, you have to put the `output` folder in the same directory as `main` (The executable player file), then excute this command:

```bash
./main
```

The player will run and enjoy!
