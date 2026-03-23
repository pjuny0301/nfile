// atfile.hpp
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace fsize_literals {
    constexpr std::size_t operator"" _bytes(unsigned long long v) {
        return static_cast<std::size_t>(v);
    }
}

class atfile {
public:
    explicit atfile(std::string_view path);

    atfile& buffed();
    atfile& buffed(std::size_t offset, std::size_t size);
    atfile& buffed(std::size_t more_bytes);
    void    flush();

    atfile&       operator<<(std::string_view content); // buffer에만 append
    const atfile& operator>>(std::string& out) const;   // buffer에서만 읽기
    atfile&       reflect();                            // buffer 내용을 파일에 반영

    const std::string& buffer() const noexcept { return buffer_; }
    std::size_t buffer_offset() const noexcept { return buffer_offset_; }
    std::size_t next_offset() const noexcept { return buffer_offset_ + buffer_.size(); }
    std::size_t size() const noexcept;

private:
    void        refresh_file_size_();
    std::string read_range_(std::size_t offset, std::size_t size) const;

private:
    std::string path_;
    std::size_t file_size_         = 0; // 실제 파일 크기
    std::size_t buffer_offset_     = 0; // 버퍼가 대응하는 파일 시작 위치
    std::size_t loaded_end_offset_ = 0; // 원래 파일에서 읽어온 구간의 끝
    std::string buffer_;
};

namespace fcommand {
    void create(std::string_view path);
    void remove(std::string_view path);
    void rename(std::string_view path, std::string_view new_name);
    void move(std::string_view path, std::string_view dest_dir);
    void copy(std::string_view src, std::string_view dest);
    bool exists(std::string_view path);
}
