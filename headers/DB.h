#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Memtable.h"
#include "SSTable.h"

class DB {
public:
    DB();

    void put_value(std::string key, std::string value);
    void db_delete_value(std::string key);
    std::optional<std::string> db_get_value(std::string key);

private:
    Memtable memtable;
    std::vector<SSTable> sstables;
    int sstable_idx = 0;
    int memtable_size = 0;

    void load_sstables();
    void flush();
};
