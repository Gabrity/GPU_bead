#pragma once

#include <string>
#include <vector>

struct Heightmap {
    int width = 0;
    int height = 0;
    std::vector<float> h;  // row-major, h[y * width + x], in meters
    float minValue = 0.0f;
    float maxValue = 0.0f;
};

// Text format: values separated by whitespace, rows separated by line breaks.
// Empty lines are skipped, so both single and double line breaks between rows work.
// Returns false and fills `error` on I/O or format problems (e.g. ragged rows).
bool loadHeightmap(const std::string& path, Heightmap& out, std::string& error);
