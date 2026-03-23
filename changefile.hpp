// change_file.hpp
#pragma once

#include "searfile.hpp"

#include <cstddef>
#include <string>
#include <string_view>

class change_file : public searfile {
public:
    explicit change_file(std::string_view path);

    std::size_t change_all(std::string_view from, std::string_view to);

private:
    std::string path_;
};
