#include "aaa_first.h"

#ifdef _MSC_VER
//segfaults on MinGW
#include <rpnew.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

enki::TaskScheduler g_TS;
