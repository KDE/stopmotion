#include "src/config.h"

#ifndef HTML_PATH
#define HTML_PATH "/usr/local/share/doc/stopmotion/html"
#endif

#ifndef TRANSLATIONS_PATH
#define TRANSLATIONS_PATH "/usr/local/share/stopmotion/translations"
#endif

const char *const stopmotion::htmlDirectory = HTML_PATH;
const char *const stopmotion::translationsDirectory = TRANSLATIONS_PATH;
