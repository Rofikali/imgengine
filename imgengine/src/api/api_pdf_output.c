// ./src/api/api_pdf_output.c
// src/api/api_pdf_output.c

#include <string.h>

int is_pdf_output(const char *path) {
    if (!path)
        return 0;

    const char *dot = strrchr(path, '.');
    if (!dot)
        return 0;
    return (dot[1] == 'p' || dot[1] == 'P') && (dot[2] == 'd' || dot[2] == 'D') &&
           (dot[3] == 'f' || dot[3] == 'F') && dot[4] == '\0';
}
