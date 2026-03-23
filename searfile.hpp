// searfile.hpp
#pragma once

#include "atfile.hpp"

#include <cstddef>
#include <string>
#include <string_view>

class searfile : public atfile {
public:
    struct search_result {
        bool        found        = false;
        bool        cut          = false;
        std::size_t s            = 0;
        std::size_t e            = 0;
        std::size_t next_offset  = 0;
        std::string tail_fragment;

        bool no_match_at_all() const noexcept {
            return !found && !cut;
        }
    };

public:
    explicit searfile(std::string_view path);

    search_result search(std::string_view word) const;
};
