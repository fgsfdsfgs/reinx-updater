#pragma once

#include "utils.h"

extern char cfg_meta_url[MAX_PATH + 1];
extern int cfg_nogc_behavior;
extern int cfg_mitm_behavior;
extern int cfg_install_extras;

int config_load(const char *fname);
int config_save(const char *fname);
