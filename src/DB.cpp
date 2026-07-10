#include "../headers/DB.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

void DB::load_sstables() {
    std::string file_path = "tables/";
    for (auto& entry : std::filesystem::directory_iterator(file_path)) {
        sstables.emplace_back(std::stoi(entry.path().stem().string()));
    }
    std::sort(sstables.begin(), sstables.end(),
              [](const SSTable& a, const SSTable& b) { return a.get_index() < b.get_index(); });
    sstable_idx = sstables.empty() ? 0 : sstables.back().get_index() + 1;
}

void DB::flush() {
    auto temp = std::move(memtable);
    temp.flush_memtable();
    memtable = Memtable{};
    memtable_size = 0;
    sstables.emplace_back(sstable_idx);
    sstables.back().flush_sstable(temp.get_table());
    sstable_idx++;
}

DB::DB() {
    load_sstables();
}

void DB::put_value(std::string key, std::string value) {
    memtable.put(key, value);
    memtable_size += key.size() + value.size();
    if (memtable_size >= 200) { // change to 4 mil as 4mb, 200 is for testing
        flush();
    }
}

void DB::db_delete_value(std::string key) {
    memtable.delete_value(key);
}

std::optional<std::string> DB::db_get_value(std::string key) {
    auto value = memtable.get(key);
    if (value) {
        return *value;
    }
    // have to check sstables now
    if (sstables.size() == 0) {
        return std::nullopt;
    }
    for (auto it = sstables.rbegin(); it != sstables.rend(); ++it) {
        auto value = it->get_value(key);
        if (!value) { // it's not in this sstable
            continue; // go to the next sstable
        }
        if (!*value) { // in the sstable, but has a tombstone of true
            return std::nullopt;
        }
        return **value;
    }
    return std::nullopt;
}
