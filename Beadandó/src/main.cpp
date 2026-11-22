#include <chrono>
#include <cstdio>
#include <string>

#include "heightmap.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <heightmap.txt>\n", argv[0]);
        return 1;
    }

    Heightmap map;
    std::string error;
    auto t0 = std::chrono::steady_clock::now();
    if (!loadHeightmap(argv[1], map, error)) {
        std::fprintf(stderr, "error: %s\n", error.c_str());
        return 1;
    }
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();

    std::printf("%s: %d x %d, min %.3f m, max %.3f m, loaded in %.1f ms\n", argv[1], map.width,
                map.height, map.minValue, map.maxValue, ms);
    return 0;
}
