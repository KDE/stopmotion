#include "src/config.h"

#ifndef PREFIX_PATH
#define PREFIX_PATH "/usr/local"
#endif

const char *const stopmotion::htmlDirectory
		= PREFIX_PATH "/share/doc/stopmotion/html";
const char *const stopmotion::translationsDirectory
		= PREFIX_PATH "/share/stopmotion/translations";
