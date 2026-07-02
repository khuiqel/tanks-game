#include "aaa_first.h"

#ifdef _WIN32
#include <rpnew.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

enki::TaskScheduler g_TS;
