#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <switch.h>
#include <errno.h>

#include "utils.h"

const char *download_error = NULL;

int download_init(void) {
  socketInitializeDefault();
  return curl_global_init(CURL_GLOBAL_DEFAULT);
}

void download_die(void) {
  curl_global_cleanup();
  socketExit();
}

static size_t download_writefunc(void *ptr, size_t sz, size_t nmemb, FILE *f) {
  return fwrite(ptr, sz, nmemb, f);
}

int download_file(const char *url, const char *to) {
  static const char *ftp_login = "anonymous";
  static const char *ftp_passwd = "anonymous";

  FILE *fout = fopen(to, "wb");
  if (!fout) {
    download_error = strerror(errno);
    return -1;
  }

  CURL *curl = curl_easy_init();
  if (!curl) {
    download_error = "could not init CURL";
    fclose(fout);
    return -2;
  }

  char urlbuf[MAX_PATH + 1];
  strncpy(urlbuf, url, MAX_PATH);

  char *rurl = urlbuf;

  if (!strncmp(rurl, "https", 5)) {
    // HACK: attempt HTTP access since HTTPS is borked
    rurl++;
    memcpy(rurl, "http", 4);
  }

  curl_easy_setopt(curl, CURLOPT_URL, rurl);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  // set it up to write to a file using our functions
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fout);

  // who cares about security
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  // if it's an ftp, we probably need to set login and pass
  curl_easy_setopt(curl, CURLOPT_USERNAME, ftp_login);
  curl_easy_setopt(curl, CURLOPT_PASSWORD, ftp_passwd);

  int res = curl_easy_perform(curl);
  fclose(fout);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    download_error = curl_easy_strerror(res);
    return -3;
  }

  return 0;
}
