#include <stdio.h>
#include <stdlib.h>
#include <switch.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "utils.h"
#include "unzip_utils.h"
#include "version.h"
#include "download.h"

static const char *meta_url = "http://dootnode.org/reinx/meta.txt";

static void errormsg(const char *msg) {
  printf("ERROR: %s!\n", msg);
  printf("press B to exit\n");
  wait_for_button(KEY_B);
}

static void init(void) {
  gfxInitDefault();
  consoleInit(NULL);
  if (download_init()) {
    errormsg("could not init CURL");
    gfxExit();
    exit(1);
  }
}

static void die(void) {
  remove("./temp/update.zip");
  remove("./temp/new.ver");
  download_die();
  gfxExit();
}

static version_t *default_version(void) {
  version_t *ver = calloc(1, sizeof(version_t));
  return ver;
}

int main(int argc, char **argv) {
  init();
  atexit(die);

  mkpath("./temp/update/", 0777);

  version_t *ver_cur = version_parse("./current.ver");
  if (!ver_cur) ver_cur = default_version();
  if (!ver_cur) return 2;

  printf("ReiNX sdfiles updater\n");
  printf("=======================\n\n");
  printf("current version: %hhu.%hhu.%hhu\n\n",
     ver_cur->triplet[0],  ver_cur->triplet[1],  ver_cur->triplet[2]
  );
  printf("press A to check for updates\n");
  printf("press any other button to quit\n");

  u32 keys = wait_for_input();
  if (!(keys & KEY_A)) {
    version_free(ver_cur);
    return 3;
  }

  consoleClear();
  printf("downloading version file...\n");

  if (download_file(meta_url, "./temp/new.ver")) {
    errormsg(download_error);
    return 4;
  }

  version_t *ver_new = version_parse("./temp/new.ver");
  if (!ver_new) {
    errormsg("could not parse new.ver");
    return 5;
  }

  consoleClear();

  printf("current version: %hhu.%hhu.%hhu\n",
     ver_cur->triplet[0],  ver_cur->triplet[1],  ver_cur->triplet[2]
  );
  printf("latest version: %hhu.%hhu.%hhu\n",
     ver_new->triplet[0],  ver_new->triplet[1],  ver_new->triplet[2]
  );

  if (!memcmp(ver_cur->triplet, ver_new->triplet, 3)) {
    printf("\nyou already have the latest version\n");
    printf("press any button to quit\n");
    wait_for_input();
    return 0;
  }

  printf("\nchangelog:\n%s\n\n", ver_new->changelog);

  printf("press A to download update\n");
  printf("press any other button to quit\n");

  keys = wait_for_input();
  if (!(keys & KEY_A)) {
    version_free(ver_cur);
    return 6;
  }

  consoleClear();
  printf("downloading update zip from\n%s\n", ver_new->link);

  if (download_file(ver_new->link, "./temp/update.zip")) {
    errormsg(download_error);
    return 7;
  }

  printf("checking hash...\n");

  u8 ziphash[0x20] = { 0 };

  if (fhash("./temp/update.zip", ziphash)) {
    errormsg("failed to compute hash for update.zip");
    return 8;
  }

  if (memcmp(ziphash, ver_new->hash, 0x20)) {
    char strziphash[0x41] = { 0 };
    char strnewhash[0x41] = { 0 };
    bytes2hex(strziphash, ziphash, 0x20);
    bytes2hex(strnewhash, ver_new->hash, 0x20);
    printf("expected: %s\n", strnewhash);
    printf("got:      %s\n", strziphash);
    errormsg("hash mismatch");
    return 9;
  }

  printf("unzipping update.zip...\n");

  unzFile *zip = zip_open("./temp/update.zip");
  if (!zip) {
    errormsg("failed to open update.zip");
    return 10;
  }

  zip_extract_all(zip, "./temp/update");
  zip_close(zip);

  printf("clearing temp files...\n");

  remove("./current.ver");
  rename("./temp/new.ver", "./current.ver");

  printf("done\npress any button to quit\n");
  wait_for_input();

  return 0;
}
