#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define xutoss(t, n) ((t*) malloc((n) * sizeof(t)))

#define xuretoss(ptr, t, n) (ptr = (t*) realloc((ptr), (n) * sizeof(t)))

#define xuzero(a) (memset(&(a), 0, sizeof(a)))

#define xumax(a, b) (((a) > (b)) ? (a) : (b))

#define xumin(a, b) (((a) < (b)) ? (a) : (b))

int xulns(FILE* const file);

char* xureadln(FILE* const file);

uint32_t xushade(const float brightness);

void xubomb(const char* const message, ...);
