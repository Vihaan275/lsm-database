#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct data_entry {
    std::string key;
    std::string value;
    bool tombstone;

    data_entry(std::string key_, std::string value_, bool tombstone_)
        : key(std::move(key_)), value(std::move(value_)), tombstone(tombstone_) {}

    std::string get_key() { return key; }
    std::string get_value() { return value; }
    bool get_tombstone() { return tombstone; }
};

class WAL {
public:
    void open_wal(std::string dir);
    void write_wal(const std::string& key, const std::string& value, bool tombstone);
    void flush_wal_log();
    void delete_wal_log();
    std::vector<data_entry> recover_wal();
    ~WAL();

private:
    int fd, read_fd;
    std::string file_path;
};
