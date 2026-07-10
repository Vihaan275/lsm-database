#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "wal.h"

class Memtable {
public:
    Memtable();

    void flush_memtable();
    void put(std::string key, std::string value);
    void put_for_del(std::string key, std::string value);
    std::optional<std::string> get(std::string key_);
    void delete_value(std::string key_);

private:
    std::map<std::string, std::pair<std::string, bool>> table;
    WAL wal_log;
    std::vector<data_entry> past_writes;

    void recover_from_wal();
};
