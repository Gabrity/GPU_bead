#include "heightmap.h"

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <memory>

namespace {

bool readFile(const std::string& path, std::string& data, std::string& error)
{
    std::unique_ptr<FILE, int (*)(FILE*)> f(std::fopen(path.c_str(), "rb"), std::fclose);
    if (!f) {
        error = "cannot open file: " + path;
        return false;
    }
    std::fseek(f.get(), 0, SEEK_END);
    long size = std::ftell(f.get());
    std::fseek(f.get(), 0, SEEK_SET);
    if (size < 0) {
        error = "cannot determine file size: " + path;
        return false;
    }
    data.resize(static_cast<size_t>(size));
    if (std::fread(data.data(), 1, data.size(), f.get()) != data.size()) {
        error = "read error: " + path;
        return false;
    }
    return true;
}

inline bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

}  // namespace

bool loadHeightmap(const std::string& path, Heightmap& out, std::string& error)
{
    std::string data;
    if (!readFile(path, data, error)) return false;

    out = Heightmap();
    const char* p = data.data();
    const char* end = p + data.size();
    int rows = 0;
    int cols = 0;

    while (p < end) {
        const char* lineEnd = static_cast<const char*>(std::memchr(p, '\n', end - p));
        if (!lineEnd) lineEnd = end;

        int count = 0;
        const char* q = p;
        while (q < lineEnd) {
            while (q < lineEnd && isSpace(*q)) ++q;
            if (q == lineEnd) break;

            float v;
            auto res = std::from_chars(q, lineEnd, v);
            if (res.ec != std::errc()) {
                error = "invalid number in row " + std::to_string(rows + 1);
                return false;
            }
            out.h.push_back(v);
            q = res.ptr;
            ++count;
        }

        if (count > 0) {
            if (rows == 0) {
                cols = count;
            } else if (count != cols) {
                error = "row " + std::to_string(rows + 1) + " has " + std::to_string(count) +
                        " values, expected " + std::to_string(cols);
                return false;
            }
            ++rows;
        }
        p = lineEnd + 1;
    }

    if (rows == 0) {
        error = "file contains no data: " + path;
        return false;
    }

    out.width = cols;
    out.height = rows;
    auto mm = std::minmax_element(out.h.begin(), out.h.end());
    out.minValue = *mm.first;
    out.maxValue = *mm.second;
    return true;
}
