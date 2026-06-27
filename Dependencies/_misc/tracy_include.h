#pragma once

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#else
#define ZoneScoped
#define FrameMark
#define FrameMarkStart(x)
#define FrameMarkEnd(x)
#endif
