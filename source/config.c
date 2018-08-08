#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "config.h"

#define MAX_CFGLINE (MAX_PATH * 2)

// version file URL (FTP and HTTP supported, no HTTPS)
char cfg_meta_url[MAX_PATH + 1] = "http://dootnode.org/reinx/meta.txt";
// 0 - always install nogc file from stash
// 1 - remove nogc file if it wasn't present before update
// 2 - ask user
int cfg_nogc_behavior = 2;
// 0 - always install fs_mitm.kip from stash
// 1 - remove fs_mitm.kip if it wasn't present before update
// 2 - ask user
int cfg_mitm_behavior = 2;
// 0 - never install anything except the SD Files foldeer
// 1 - install stuff from e.g. Meteos Recommends
// 2 - ask user
int cfg_install_extras = 2;

typedef struct cfgvar_s {
  const char *name; // var name, used in cfg file
  int type;         // var type (0 = string, 1 = int)
  void *value;      // pointer to actual var
} cfgvar_t;

static cfgvar_t cfgvars[] = {
  { "version_file_url", 0, cfg_meta_url },
  { "nogc_behavior", 1, &cfg_nogc_behavior },
  { "layeredfs_behavior", 1, &cfg_mitm_behavior },
  { "install_extras", 1, &cfg_install_extras },
  { NULL, 0, NULL },
};

static char *strstrip(char *str) {
  if (!str) return NULL;

  char *s = str;

  // ltrim
  while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;

  // rtrim
  int len = strlen(s);
  if (!len) return s;
  char *e = s + len - 1;
  while (e >= str && (*e == ' ' || *e == '\t' || *e == '\n' || *e == '\r'))
    *(e--) = '\0';

  return s;
}

static void config_set(char *name, char *value) {
  for (cfgvar_t *var = cfgvars; var->name; ++var) {
    if (strncmp(name, var->name, MAX_CFGLINE))
      continue;
    if (var->type == 0) {
      strcpy((char *)(var->value), value);
    } else if (var->type == 1) {
      int x = strtol(value, NULL, 0);
      *(int *)(var->value) = x;
    }
    break;
  }
}

int config_load(const char *fname) {
  static char line[MAX_CFGLINE + 1];
  static char varname[MAX_CFGLINE + 1];
  static char varvalue[MAX_CFGLINE + 1];

  FILE *f = fopen(fname, "r");
  if (!f) return -1;

  int ret = 0;

  while (fgets(line, MAX_CFGLINE, f)) {
    char *tline = strstrip(line);
    int tlen = strlen(tline);
    // comment or empty line
    if (!tlen || !strncmp(tline, "//", 2)) continue;
    // separate into varname and value
    char *eqptr = strchr(tline, '=');
    if (!eqptr || eqptr == tline || eqptr >= tline + tlen - 1) {
      ret = -2;
      break;
    }
    strncpy(varvalue, eqptr + 1, MAX_CFGLINE);
    memcpy(varname, tline, eqptr - tline);
    varname[eqptr - tline] = '\0';
    char *tvarvalue = strstrip(varvalue);
    char *tvarname = strstrip(varname);
    config_set(tvarname, tvarvalue);
  }

  fclose(f);
  return ret;
}

int config_save(const char *fname) {
  FILE *f = fopen(fname, "w");
  if (!f) return -1;

  for (cfgvar_t *var = cfgvars; var->name; ++var) {
    if (var->type == 0)
      fprintf(f, "%s = %s\n", var->name, (char *)(var->value));
    else if (var->type == 1)
      fprintf(f, "%s = %d\n", var->name, *(int *)(var->value));
  }

  fclose(f);
}
