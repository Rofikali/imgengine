// /* cmd/imgengine/main.c */

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "api/v1/img_api.h"
#include "core/buffer.h"

int main(int argc, char **argv)
{
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
        return 1;

    size_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // 🔥 ZERO COPY FILE LOAD
    uint8_t *input = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (input == MAP_FAILED)
        return 1;

    img_engine_t *engine = img_api_init(4);

    uint8_t *output = NULL;
    size_t out_size = 0;

    img_api_process_fast(
        engine,
        input,
        size,
        &output,
        &out_size);

    int out_fd = open(argv[2], O_WRONLY | O_CREAT, 0644);
    write(out_fd, output, out_size);

    close(out_fd);
    munmap(input, size);

    img_api_shutdown(engine);

    return 0;
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>

// #include "api/v1/img_api.h"

// int main(int argc, char **argv)
// {
//     if (argc < 3)
//     {
//         printf("Usage: %s input.jpg output.jpg\n", argv[0]);
//         return 1;
//     }

//     // =====================================
//     // READ FILE
//     // =====================================
//     FILE *f = fopen(argv[1], "rb");
//     if (!f)
//         return 1;

//     fseek(f, 0, SEEK_END);
//     size_t size = ftell(f);
//     fseek(f, 0, SEEK_SET);

//     uint8_t *input = malloc(size);
//     fread(input, 1, size, f);
//     fclose(f);

//     // =====================================
//     // ENGINE
//     // =====================================
//     img_engine_t *engine = img_api_init();

//     uint8_t *output = NULL;
//     size_t out_size = 0;

//     if (img_api_process_fast(
//             engine,
//             input,
//             size,
//             &output,
//             &out_size) != 0)
//     {
//         printf("Processing failed\n");
//         return 1;
//     }

//     // =====================================
//     // WRITE FILE
//     // =====================================
//     FILE *out = fopen(argv[2], "wb");
//     fwrite(output, 1, out_size, out);
//     fclose(out);

//     // =====================================
//     // CLEANUP
//     // =====================================
//     free(input);
//     free(output);

//     img_api_shutdown(engine);

//     printf("Done.\n");
//     return 0;
// }
