#pragma once

extern const char *download_error;

int download_init(void);
void download_die(void);
int download_file(const char *url, const char *to);
