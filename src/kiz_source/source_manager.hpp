#pragma once

struct FileData {
    Vec<uint32_t> linenomap; // index => offset
    Str name;
    Str path;
    Str content;
    uint32_t id;
    uint32_t version = 1;
};

class SourceManager {
    Vec<FileData> files_;

public:
    SourceManager() = default;
    auto get_id_by_path(Str path) const noexcept -> Option<uint32_t>;
    auto get_content_by_path(Str name) const noexcept -> Option<Str>;
    auto get_content(uint32_t file_id) const noexcept -> Option<Str>;
    auto open_new(Str path) -> Result<uint32_t, SourceManagerError>;
    
    auto update_file_by_id(uint32_t id) -> bool;
    auto update_file_by_path(Str path) -> bool;

    auto clear() noexcept -> void;
    auto file_count() const noexcept -> uint32_t;
};