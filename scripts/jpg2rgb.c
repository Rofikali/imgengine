// Simple utility to decode a JPEG into raw RGB24 bytes using libturbojpeg.
// Usage: ./jpg2rgb in.jpg out.rgb

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <turbojpeg.h>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s in.jpg out.rgb\n", argv[0]);
        return 1;
    }

    const char *inpath = argv[1];
    const char *outpath = argv[2];

    FILE *f = fopen(inpath, "rb");
    if (!f)
    {
        perror("open input");
        return 1;
    }

    if (fseek(f, 0, SEEK_END) != 0)
    {
        perror("fseek");
        fclose(f);
        return 1;
    }

    long sz = ftell(f);
    if (sz <= 0)
    {
        fprintf(stderr, "invalid input size\n");
        fclose(f);
        return 1;
    }
    rewind(f);

    unsigned char *inbuf = malloc((size_t)sz);
    if (!inbuf)
    {
        perror("malloc");
        fclose(f);
        return 1;
    }

    if (fread(inbuf, 1, (size_t)sz, f) != (size_t)sz)
    {
        perror("fread");
        free(inbuf);
        fclose(f);
        return 1;
    }
    fclose(f);

    tjhandle tj = tjInitDecompress();
    if (!tj)
    {
        fprintf(stderr, "tjInitDecompress failed\n");
        free(inbuf);
        return 1;
    }

    int width = 0, height = 0, subsamp = 0, colorspace = 0;
    if (tjDecompressHeader3(tj, inbuf, (unsigned long)sz, &width, &height, &subsamp, &colorspace) != 0)
    {
        fprintf(stderr, "tjDecompressHeader3 failed: %s\n", tjGetErrorStr());
        tjDestroy(tj);
        free(inbuf);
        return 1;
    }

    int stride = width * 3;
    size_t out_size = (size_t)stride * (size_t)height;
    unsigned char *outbuf = malloc(out_size);
    if (!outbuf)
    {
        perror("malloc outbuf");
        tjDestroy(tj);
        free(inbuf);
        return 1;
    }

    if (tjDecompress2(tj, inbuf, (unsigned long)sz, outbuf, width, stride, height, TJPF_RGB, 0) != 0)
    {
        fprintf(stderr, "tjDecompress2 failed: %s\n", tjGetErrorStr());
        tjDestroy(tj);
        free(inbuf);
        free(outbuf);
        return 1;
    }

    tjDestroy(tj);
    free(inbuf);

    FILE *out = fopen(outpath, "wb");
    if (!out)
    {
        perror("open out");
        free(outbuf);
        return 1;
    }

    size_t wrote = fwrite(outbuf, 1, out_size, out);
    fclose(out);
    if (wrote != out_size)
    {
        fprintf(stderr, "short write\n");
        free(outbuf);
        return 1;
    }

    free(outbuf);

    // Print width height stride for convenience
    printf("%d %d %d\n", width, height, stride);
    return 0;
}
