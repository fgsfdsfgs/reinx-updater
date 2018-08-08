#include <switch.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "version.h"

version_t *version_parse(const char *verfile) {
  char line[MAX_VERLINE + 1];
  version_t *ver = NULL;

  FILE *f = fopen(verfile, "r");
  if (!f) return NULL;

  size_t fsz = fsize(f);
  if (!fsz) goto _failure;

  ver = calloc(1, sizeof(version_t));
  if (!ver) goto _failure;

  // read version triplet
  if (!fgets(line, MAX_VERLINE, f)) goto _failure;
  line[strcspn(line, "\r\n")] = '\0';
  sscanf(line, "%hhu.%hhu.%hhu", ver->triplet, ver->triplet + 1, ver->triplet + 2);
  // read hash
  if (!fgets(line, MAX_VERLINE, f)) goto _failure;
  line[strcspn(line, "\r\n")] = '\0';
  hex2bytes(ver->hash, line);
  // read DL link
  if (!fgets(ver->link, MAX_VERLINE, f)) goto _failure;
  ver->link[strcspn(ver->link, "\r\n")] = '\0';
  // read comments
  s64 bufsz = fsz - ftell(f);
  if (!bufsz) goto _failure;
  ver->changelog = calloc(1, bufsz + 1);
  if (!ver->changelog) goto _failure;
  fread(ver->changelog, bufsz, 1, f);

  fclose(f);
  return ver;

_failure:
  if (ver) version_free(ver);
  if (f) fclose(f);
  return NULL;
}

void version_free(version_t *ver) {
  if (!ver) return;
  if (ver->changelog) free(ver->changelog);
  free(ver);
}
