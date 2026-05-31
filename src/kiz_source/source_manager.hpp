#pragma once


class SourceManager : NoCopyMove {
    Vec<FileMetaData> files_;

public:
    auto get_id_by_path(Str path) const noexcept -> Option<uint32_t>;
    auto get_content_by_path(Str name) const noexcept -> Option<Str>;
    auto get_content(uint32_t file_id) const noexcept -> Option<Str>;
    auto open_new(Str path) -> Result<uint32_t, SourceManagerError>;
    
    auto update_file_by_id(uint32_t id) -> bool;
    auto update_file_by_path(Str path) -> bool;

    auto clear() noexcept -> void;
    auto file_count() const noexcept -> uint32_t;
};