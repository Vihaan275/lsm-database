#include "../headers/Memtable.h"

#include <iostream>

void Memtable::recover_from_wal() {
    wal_log.open_wal(".././data");
    std::vector<data_entry> past_logs = wal_log.recover_wal();
    for (data_entry i : past_logs) {
        table[i.key].first = i.value;
        table[i.key].second = i.tombstone;
    }
}

Memtable::Memtable() {
    recover_from_wal();
}

void Memtable::flush_memtable() {
    wal_log.delete_wal_log();
}

void Memtable::put(std::string key, std::string value) {
    wal_log.write_wal(key, value, false);
    table[key].first = value;
    table[key].second = false;
}

std::map<std::string, std::pair<std::string, bool>> Memtable::get_table(){
  return table;
}

void Memtable::put_for_del(std::string key, std::string value) {
    wal_log.write_wal(key, value, true);
    table[key].first = value;
    table[key].second = true;
}

std::optional<std::string> Memtable::get(std::string key_) {
    try {
        auto answer = table.at(key_);
        if (!answer.second) {
            return answer.first;
        }
        return std::nullopt;
    } catch (const std::out_of_range& e) {
        // ADD BEHAVIOUR TO SEARCH SSTABLE ONCE U MAKE IT
        std::cout << "No data found" << std::endl;
        return std::nullopt;
    }
}

void Memtable::delete_value(std::string key_) {
    wal_log.write_wal(key_, " ", true);
    this->put_for_del(key_, "");
}
