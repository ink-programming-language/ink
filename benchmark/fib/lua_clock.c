#include <lua.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#define BENCHMARK_EXPORT __declspec(dllexport)
static LARGE_INTEGER Frequency;
#else
#include <time.h>
#define BENCHMARK_EXPORT
#endif

static int now(lua_State* State)
{
#if defined(_WIN32)
    LARGE_INTEGER Counter;
    if (!QueryPerformanceCounter(&Counter))
    {
        return 0;
    }
    lua_pushnumber(State, (lua_Number)Counter.QuadPart / (lua_Number)Frequency.QuadPart);
#else
    struct timespec Counter;
    if (clock_gettime(CLOCK_MONOTONIC, &Counter) != 0)
    {
        return 0;
    }
    lua_pushnumber(State, (lua_Number)Counter.tv_sec + (lua_Number)Counter.tv_nsec / 1000000000.0);
#endif
    return 1;
}

BENCHMARK_EXPORT int luaopen_benchmark_clock(lua_State* State)
{
#if defined(_WIN32)
    if (!QueryPerformanceFrequency(&Frequency))
    {
        return 0;
    }
#endif
    lua_pushcfunction(State, now);
    return 1;
}
