#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

class SSTable {
public:
    explicit SSTable(int index);
    explicit SSTable(const std::string& filepath);

    int get_index() const;

    int flush_sstable(std::map<std::string, std::pair<std::string, bool>> table);
    void make_binary_searchable();
    std::optional<std::optional<std::string>> get_value(const std::string& key);

    ~SSTable();

private:
    std::string file_path = "../tables/";
    int fd;
    std::vector<std::pair<std::string, int>> hash_map;
    int table_index;
};
