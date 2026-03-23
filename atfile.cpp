// atfile.cpp
#include "atfile.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

atfile::atfile(std::string_view path)
    : path_(path) {
    refresh_file_size_();
}

std::size_t atfile::size() const noexcept {
    const std::size_t replaced =
        (loaded_end_offset_ >= buffer_offset_)
        ? (loaded_end_offset_ - buffer_offset_)
        : 0;

    return file_size_ - std::min(file_size_, replaced) + buffer_.size();
}

void atfile::refresh_file_size_() {
    std::error_code ec;

    if (!fs::exists(path_, ec) || !fs::is_regular_file(path_, ec)) {
        throw std::runtime_error("file not found: " + path_);
    }

    file_size_ = static_cast<std::size_t>(fs::file_size(path_, ec));
    if (ec) {
        throw std::runtime_error("failed to get file size: " + path_);
    }
}

std::string atfile::read_range_(std::size_t offset, std::size_t size) const {
    if (offset >= file_size_ || size == 0) {
        return {};
    }

    const std::size_t readable = std::min(size, file_size_ - offset);

    std::ifstream in(path_, std::ios::binary);
    if (!in) {
        throw std::runtime_error("failed to open file: " + path_);
    }

    in.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!in) {
        throw std::runtime_error("failed to seek file: " + path_);
    }

    std::string out(readable, '\0');
    in.read(out.data(), static_cast<std::streamsize>(readable));
    out.resize(static_cast<std::size_t>(in.gcount()));
    return out;
}

atfile& atfile::buffed() {
    refresh_file_size_();
    buffer_ = read_range_(0, file_size_);
    buffer_offset_ = 0;
    loaded_end_offset_ = buffer_.size();
    return *this;
}

atfile& atfile::buffed(std::size_t offset, std::size_t size) {
    refresh_file_size_();
    buffer_ = read_range_(offset, size);
    buffer_offset_ = offset;
    loaded_end_offset_ = offset + buffer_.size();
    return *this;
}

atfile& atfile::buffed(std::size_t more_bytes) {
    refresh_file_size_();

    if (more_bytes == 0) {
        return *this;
    }

    const std::size_t append_from = loaded_end_offset_;

    if (buffer_.empty()) {
        buffer_offset_ = append_from;
    }

    std::string more = read_range_(append_from, more_bytes);
    buffer_ += more;
    loaded_end_offset_ = append_from + more.size();
    return *this;
}

void atfile::flush() {
    buffer_.clear();
    buffer_offset_ = 0;
    loaded_end_offset_ = 0;
}

atfile& atfile::operator<<(std::string_view content) {
    buffer_.append(content.data(), content.size());
    return *this;
}

const atfile& atfile::operator>>(std::string& out) const {
    out = buffer_;
    return *this;
}

atfile& atfile::reflect() {
    refresh_file_size_();

    const std::string prefix = read_range_(0, buffer_offset_);
    const std::string suffix =
        (loaded_end_offset_ < file_size_)
        ? read_range_(loaded_end_offset_, file_size_ - loaded_end_offset_)
        : std::string{};

    std::ofstream out(path_, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("failed to open file for reflect: " + path_);
    }

    out.write(prefix.data(), static_cast<std::streamsize>(prefix.size()));
    out.write(buffer_.data(), static_cast<std::streamsize>(buffer_.size()));
    out.write(suffix.data(), static_cast<std::streamsize>(suffix.size()));

    if (!out) {
        throw std::runtime_error("failed to reflect buffer into file: " + path_);
    }

    file_size_ = prefix.size() + buffer_.size() + suffix.size();
    loaded_end_offset_ = buffer_offset_ + buffer_.size();
    return *this;
}

namespace fcommand {

void create(std::string_view path) {
    std::ofstream out(std::string(path), std::ios::binary | std::ios::app);
    if (!out) {
        throw std::runtime_error("failed to create file: " + std::string(path));
    }
}

void remove(std::string_view path) {
    std::error_code ec;
    fs::remove(fs::path(path), ec);
    if (ec) {
        throw std::runtime_error("failed to remove file: " + std::string(path));
    }
}

void rename(std::string_view path, std::string_view new_name) {
    const fs::path src(path);
    const fs::path dst = src.parent_path() / fs::path(new_name);

    std::error_code ec;
    fs::rename(src, dst, ec);
    if (ec) {
        throw std::runtime_error("failed to rename file: " + std::string(path));
    }
}

void move(std::string_view path, std::string_view dest_dir) {
    const fs::path src(path);
    const fs::path dst_dir(dest_dir);
    const fs::path dst = dst_dir / src.filename();

    std::error_code ec;
    fs::create_directories(dst_dir, ec);
    if (ec) {
        throw std::runtime_error("failed to create destination dir: " + dst_dir.string());
    }

    ec.clear();
    fs::rename(src, dst, ec);
    if (!ec) {
        return;
    }

    ec.clear();
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        throw std::runtime_error("failed to move file: " + src.string());
    }

    ec.clear();
    fs::remove(src, ec);
    if (ec) {
        throw std::runtime_error("failed to remove original after move: " + src.string());
    }
}

void copy(std::string_view src, std::string_view dest) {
    const fs::path dst(dest);

    std::error_code ec;
    if (!dst.parent_path().empty()) {
        fs::create_directories(dst.parent_path(), ec);
        if (ec) {
            throw std::runtime_error("failed to create destination dir: " + dst.parent_path().string());
        }
    }

    ec.clear();
    fs::copy_file(fs::path(src), dst, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        throw std::runtime_error("failed to copy file: " + std::string(src));
    }
}

bool exists(std::string_view path) {
    std::error_code ec;
    return fs::exists(fs::path(path), ec);
}

} // namespace fcommand
