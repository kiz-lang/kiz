#pragma once

struct FileMetaData {
    Vec<uint32_t> lineno_map; // index => offset
    Str name;
    Str path;
    Str content;
    uint32_t id;
    uint32_t version = 1;
};