#include "../headers/SSTable.h"

#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

SSTable::SSTable(int index) {
    file_path = file_path + std::to_string(index) + ".sst";
    fd = open(file_path.c_str(), O_RDWR | O_CREAT, 0600);
    table_index = index;
}

SSTable::SSTable(const std::string& filepath) {
    file_path = filepath;
    fd = open(file_path.c_str(), O_RDWR | O_CREAT, 0600);
    table_index = -1; // not derived from a numeric index
}

int SSTable::get_index() const {
    return table_index;
}

int SSTable::flush_sstable(std::map<std::string, std::pair<std::string, bool>> table) {
    int size = table.size();
    int arr_idx = 0;
    uint32_t byte_count = 0;
    std::vector<uint32_t> arr;
    std::vector<std::string> temp;

    for (auto const& [key, value] : table) {
        arr.push_back(byte_count);

        uint32_t key_len = key.size();
        write(fd, reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        byte_count += sizeof(key_len);
        write(fd, key.data(), key_len);
        byte_count += key_len;
        temp.push_back(key);

        uint32_t value_len = value.first.size();
        write(fd, reinterpret_cast<const char*>(&value_len), sizeof(value_len));
        byte_count += sizeof(value_len);
        write(fd, value.first.data(), value_len);
        byte_count += value_len;

        uint8_t tomb = value.second ? 1 : 0;
        write(fd, &tomb, sizeof(tomb));
        byte_count += 1;

        arr_idx++;
    }

    uint32_t start_of_index_table = byte_count;

    for (int i = 0; i < arr_idx; i++) {
        uint32_t key_len = temp.at(i).size();
        write(fd, reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        write(fd, temp.at(i).data(), key_len);
        write(fd, reinterpret_cast<const char*>(&arr.at(i)), sizeof(uint32_t));
    }

    write(fd, reinterpret_cast<const char*>(&start_of_index_table), sizeof(uint32_t));

    make_binary_searchable();

    return size;
}

void SSTable::make_binary_searchable() {
    uint32_t start_index;
    lseek(fd, -static_cast<off_t>(sizeof(uint32_t)), SEEK_END);
    read(fd, &start_index, sizeof(start_index));

    struct stat st{};
    if (fstat(fd, &st) == -1) {
        std::cout << "Error with reading file size" << std::endl;
    }

    while (start_index < static_cast<uint32_t>(st.st_size) - sizeof(start_index)) {
        uint32_t key_len;
        pread(fd, &key_len, sizeof(key_len), start_index);
        start_index += sizeof(key_len);

        std::string key_(key_len, '\0');
        pread(fd, key_.data(), key_len, start_index);
        start_index += key_len;

        uint32_t byte_pos;
        pread(fd, &byte_pos, sizeof(uint32_t), start_index);
        start_index += sizeof(byte_pos);

        hash_map.emplace_back(key_, byte_pos);
    }
}

std::optional<std::optional<std::string>> SSTable::get_value(const std::string& key) {
    int left = 0;
    int right = hash_map.size() - 1;
    int byte_offset = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        auto& mid_element = hash_map.at(mid);

        if (mid_element.first == key) {
            byte_offset = mid_element.second;
            break;
        } else if (key < mid_element.first) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    if (byte_offset == -1) {
        return std::nullopt; // key not present in this table at all
    }

    uint32_t key_len;
    pread(fd, &key_len, sizeof(key_len), byte_offset);
    byte_offset += sizeof(key_len);

    std::string key_(key_len, '\0');
    pread(fd, key_.data(), key_len, byte_offset);
    byte_offset += key_len;

    uint32_t value_len;
    pread(fd, &value_len, sizeof(value_len), byte_offset);
    byte_offset += sizeof(value_len);

    std::string value_(value_len, '\0');
    pread(fd, value_.data(), value_len, byte_offset);
    byte_offset += value_len;

    uint8_t tomb;
    pread(fd, &tomb, sizeof(tomb), byte_offset);

    if (!tomb) {
        return std::optional<std::string>{value_}; // present, live value
    }

    return std::optional<std::string>{std::nullopt}; // present, but tombstoned
}

SSTable::~SSTable(){
  close(fd);
}
