#include "../headers/WAL.h"

#include <fcntl.h>
#include <filesystem>
#include <unistd.h>

void WAL::open_wal(std::string dir) {
    // creates folder
    std::filesystem::create_directory(dir);
    // creates file in folder and opens it in append mode
    file_path = dir + "/.log";
    fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0600);
    // make a read fd to read from when parsing
    read_fd = open(file_path.c_str(), O_RDONLY);
}

void WAL::write_wal(const std::string& key, const std::string& value, bool tombstone) {
    // writing key to db
    uint32_t key_len = key.size();
    write(fd, reinterpret_cast<const char*>(&key_len), sizeof(key_len));
    write(fd, key.data(), key_len);

    // writing value to log file
    uint32_t value_len = value.size();
    write(fd, reinterpret_cast<const char*>(&value_len), sizeof(value_len));
    write(fd, value.data(), value_len);

    // writing tombstone directly without length
    uint8_t t = tombstone ? 1 : 0;
    write(fd, &t, sizeof(t));
}

void WAL::flush_wal_log() {
    fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
}

std::vector<data_entry> WAL::recover_wal() {
    // make sure read_fd is at the start of the file
    lseek(read_fd, 0, SEEK_SET);
    std::vector<data_entry> entries;

    while (true) {
        uint32_t key_len;
        if (read(read_fd, &key_len, sizeof(key_len)) == 0) {
            break;
        }
        // if length of bytes returned by read = 0, that means we are EOF so exit

        // reading key
        std::string key_(key_len, '\0');
        read(read_fd, key_.data(), key_len);

        // reading length of value and then value itself
        uint32_t value_len;
        if (read(read_fd, &value_len, sizeof(value_len)) != sizeof(value_len)) {
            break;
        }
        std::string value_(value_len, '\0');
        read(read_fd, value_.data(), value_len);

        // reading bool
        uint8_t t;
        if (read(read_fd, &t, sizeof(t)) != sizeof(t)) {
            break;
        }
        bool tombstone_ = (t == 1);

        // making an entry object and adding it to the vector
        data_entry obj(key_, value_, tombstone_);
        entries.push_back(obj);
    }
    return entries;
}

void WAL::delete_wal_log() {
    fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
}

WAL::~WAL(){
  close(fd);
  close(read_fd);
}
