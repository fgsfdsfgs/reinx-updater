#pragma once

#include <switch.h>
#include "utils.h"

#define MAX_VERLINE 1024

typedef struct version_s {
  u8 triplet[3];
  u8 hash[0x20];
  char link[MAX_VERLINE + 1];
  char *changelog;
} version_t;

version_t *version_parse(const char *verfile);
void version_free(version_t *ver);
