#include "aaa_first.h"

#include <rpmalloc.h>
#include <rpnew.h> //only include once! (there's no include guard, despite the readme saying it's okay)

//compiling rpmalloc with malloc/free replacement *and automatic thread initialization* (ENABLE_OVERRIDE=1):

//current version (1.4.5):
//Windows: C/C++ > Code Generation > Runtime Library to static (the non-DLL one), use glfw3_mt.lib, add ENABLE_PRELOAD=1
//Linux: remove the rpnew.h include, add _GNU_SOURCE (even if you're using Clang!), add ENABLE_PRELOAD=1
//Windows continued: it's not possible to use enkiTS and rpmalloc together this way (Linux unaffected)

//latest develop branch (which will presumably become 2.0.0) (ENABLE_OVERRIDE is on by default):
//Windows: just C17 and /experimental:c11atomics (other changes are not necessary)
//Linux: remove the rpnew.h include (a debugger shows it's hitting the rpmalloc functions on a new() call despite rpnew.h not being included, so I guess rpnew.h is Windows-only (__CRTDECL is Microsoft-exclusive, after all))
//Linux continued: malloc.c says to include rpnew.h on Windows, and Linux does not compile if rpnew.h is included (redefining new/delete/etc), even though the readme implies it's supposed to be included despite the platform

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

FirstLoadedObject::FirstLoadedObject() {
	//needs to be initialized before main(), because anything using the heap (like std::string or std::vector) will fail otherwise
	//also, this is *not* a HACK, because this is literally the only way to solve this problem in C/C++ (as far as I'm aware)
	rpmalloc_initialize();
}

FirstLoadedObject first_loaded;

enki::TaskScheduler g_TS;
