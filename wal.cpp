#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>


struct entry{
    string key;
    string value;
    bool tombstone;

    entries(string key_,string value_,bool tombstone_){
        key = key_;
        value = value_;
        tombstone = tombstone_;
    }

    string get_key(){
        return key;
    }

    string get_value(){
        return value;
    }

    string get_tombstone(){
        return tombstone;
    }
}

class WAL{

    private:
        int fd,read_fd;
        std::string file_path;

    public:
    void open(string dir){
        //creaters folder
        std::filesystem::create_directory(dir);

        //creates file in folder and opens it in append mode
        file_path = dir+"/.log";
        fd = open(file_path,O_APPEND,0600);

        //make a read fd to read from when parsing
        read_fd = open(file_path,O_RDONLY);

    }

    void write(std::string& key, std::string& value, bool tombstone){

        //writing key to db
        uint32_t key_len = (key).size();
        write(fd,reinterpret_cast<const char*>(&key_len),sizeof(key_len);
        write(fd,key.data(),key_len);

        //writing value to log file
        uint32_t value_len = (value).size();
        write(fd,reinterpret_cast<const char*>(&value_len),sizeof(value_len));
        write(fd,value.data(),value_len);

        //writing tombstone directly without length
        uint8_t t;
        if (tombstone){
            t=0;
        }
        else t=1;
        write(fd, &t, sizeof(t));
    }

    std::vector<entry> recover(){
        
        //make sure read_fd is at the start of the file
        lseek(read_fd,0,SEEK_SET);

        std::vector<entry> entries;
        while(true){
            uint32_t key_len;
            if (read(read_fd,&key_len,sizeof(key_len)))==0){
                break;
            }
            //if length of bytes returned by read =0, that means we are EOF so exit
            
            //reading key
            string key_;
            read(read_fd,&key_,key_len);


            //reading length of value and then value itself
            uint32_t value_len;
            read(read_fd,&value_len,sizeof(value_len));
            string value_;
            read(read_fd,&value_,value_len);

            //reading bool
            bool tombstone_;
            uint8_t t;
            read(read_fd,&t,sizeof(t));
            bool tombstone_ = (t==1);

            //mkaing an entry object and adding it to the vector
            entry obj(key_,value_,tombstone_);

            entries.push_back(obj);

        }




    }

};
