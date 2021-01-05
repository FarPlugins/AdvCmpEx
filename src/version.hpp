#ifdef _WIN64
#define PLATFORM " x64"
#elif defined _WIN32
#define PLATFORM " x86"
#else
#define PLATFORM ""
#endif

#define PLUGIN_VER_MAJOR 3
#define PLUGIN_VER_MINOR 1
#define PLUGIN_VER_PATCH 0
#define PLUGIN_DESC L"Advanced compare files for Far Manager 3" PLATFORM
#define PLUGIN_NAME L"AdvCmpEx"
#define PLUGIN_FILENAME L"AdvCmpEx.dll"
#define PLUGIN_COPYRIGHT L"Copyright © 2006 Alexey Samlyukov"

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define PLUGIN_VERSION STRINGIZE(PLUGIN_VER_MAJOR) "." STRINGIZE(PLUGIN_VER_MINOR) "." STRINGIZE(PLUGIN_VER_PATCH)
