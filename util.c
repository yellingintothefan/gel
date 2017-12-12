#include "util.h"

#include <stdarg.h>
#include <stdlib.h>

uint32_t xushade(const float brightness)
{
    return
    (uint32_t) (0xFF * brightness) << 0x10 |
    (uint32_t) (0xFF * brightness) << 0x08 |
    (uint32_t) (0xFF * brightness) << 0x00;
}

int xulns(FILE* const file)
{
    int ch = EOF;
    int lines = 0;
    int pc = '\n';
    while((ch = getc(file)) != EOF)
    {
        if(ch == '\n') lines++;
        pc = ch;
    }
    if(pc != '\n') lines++;
    rewind(file);
    return lines;
}

char* xureadln(FILE* const file)
{
    int ch = EOF;
    int reads = 0;
    int size = 128;
    char* line = xutoss(char, size);;
    while((ch = getc(file)) != '\n' && ch != EOF)
    {
        line[reads++] = ch;
        if(reads + 1 == size) xuretoss(line, char, size *= 2);
    }
    line[reads] = '\0';
    return line;
}

void xubomb(const char* const message, ...)
{
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    exit(1);
}
