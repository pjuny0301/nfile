// change_file.cpp
#include "change_file.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

change_file::change_file(std::string_view path)
    : searfile(path), path_(path) {}

std::size_t change_file::change_all(std::string_view from, std::string_view to) {
    if (from.empty()) {
        return 0;
    }

    const std::size_t total = size();

    std::string out;
    out.reserve(total);

    std::size_t cursor = 0;
    std::size_t count = 0;

    while (cursor < total) {
        buffed(cursor, total - cursor);

        std::string buf;
        *this >> buf;

        if (buf.empty()) {
            break;
        }

        auto st = search(from);

        if (st.no_match_at_all()) {
            out += buf;
            break;
        }

        if (st.cut) {
            out.push_back(buf.front());
            ++cursor;
            continue;
        }

        out.append(buf, 0, st.s);
        out.append(to.data(), to.size());

        cursor += st.e;
        ++count;
    }

    if (count == 0) {
        return 0;
    }

    std::ofstream out_file(path_, std::ios::binary | std::ios::trunc);
    if (!out_file) {
        throw std::runtime_error("failed to open file for rewrite: " + path_);
    }

    out_file.write(out.data(), static_cast<std::streamsize>(out.size()));
    if (!out_file) {
        throw std::runtime_error("failed to rewrite file: " + path_);
    }

    buffed();
    return count;
}
