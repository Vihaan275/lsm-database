#include <iostream>
#include <optional>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

class SSTable{

  private:
    string file_path="tables/";
    int fd;
    std::vector<std::pair<std::string,int>> hash_map;

  public:

    SSTable(int index){
      //makes a .sst file with index as the file name
      file_path=file_path+std::to_string(index)+".sst";

      //opens the fd in append mode 
      fd = open(file_path.c_str(),O_RDWR| O_CREAT ,0600);


    }

    int flush_sstable(std::map<std::string,std::pair<std::string,bool>> table){
      int size = table.size();
      int arr_idx=0;
      uint32_t byte_count=0;
      std::vector<uint32_t> arr;
      std::vector<std::string> temp;

      //writing the data into the file
      for (auto const& [key,value] : table){
        arr.push_back(byte_count);

        //writing the key at the start of the line
        uint32_t key_len = key.size();
        write(fd,reinterpret_cast<const char*>(&key_len),sizeof(key_len));
        byte_count+=sizeof(key_len);
        write(fd,key.data(),key_len);
        byte_count+=key_len;
        temp.push_back(key);

        //adding value and also updating the corresponding byte index
        uint32_t value_len = value.first.size();
        write(fd,reinterpret_cast<const char*>(&value_len),sizeof(value_len));
        byte_count+=sizeof(value_len);
        write(fd,value.first.data(),value_len);
        byte_count+=value_len;

        //adding tombstone 
        uint8_t tomb;
        if (value.second){
          tomb=1;
        }
        else tomb=0;
        write(fd,&tomb,sizeof(tomb));
        byte_count+=1;

        arr_idx++;
      }

      //writing the index list at the end of the file
      uint32_t start_of_index_table = byte_count;

      for (int i=0;i<arr_idx;i++){

        uint32_t key_len = temp.at(i).size();

        write(fd,reinterpret_cast<const char*>(&key_len),sizeof(key_len));
        write(fd,temp.at(i).data(),key_len);
        write(fd,reinterpret_cast<const char*>(&arr.at(i)),sizeof(uint32_t));

      }

      write(fd,reinterpret_cast<const char*>(&start_of_index_table),sizeof(uint32_t);

      make_binary_searchable();

    } 

    void make_binary_searchable(){

      //we have the index to start the binary search from
      uint32_t start_index;
      lseek(fd,-sizeof(uint32_t),SEEK_END);
      read(fd,&start_index,sizeof(start_index));

      struct stat st{};
      if (fstat(fd,&st)==-1){
        std::cout<<"Error with reading file size"<<std::endl;
      }

      int bytes_to_read = st.st_size - start_index - 4;

      while(start_index<st.st_size-sizeof(start_index)){
        uint32_t key_len;
        pread(fd, &key_len,sizeof(key_len),start_index);
        start_index+=sizeof(key_len);

        std::string key_(key_len);
        pread(fd,key_.data(),key_len,start_index);
        start_index+=key_len;

        uint32_t byte_pos;
        pread(fd,&byte_pos,sizeof(uint32_t),start_index);
        start_index+=sizeof(byte_pos);

        hash_map.emplace_back(key_,byte_pos);
      }

    }


    std::optional<std::string> get_value(std::string key){

      //implement binary search on hash_map
      
      int left = 0;
      int right = hash_map.size();
      int byte_offset=-1;

      while(left<=right){

        int mid = left+(right-left)/2;
        auto mid_element = hash_map.at(mid)

        if (mid_element.first==key){
         byte_offset = mid_element.second;
        }

        else if(key<mid_element.first){

          right = mid-1;

        }
        else if(key>mid_element.first){
          left = mid+1;
        }

      }

      if (byte_offset==-1){
        return std::nullopt;
      }

      //parsing value
      uint32_t key_len;
      pread(fd, &key_len,sizeof(key_len),byte_offset);

      byte_offset+=sizeof(key_len);

      std::string key_(key_len);
      pread(fd,key_.data(),key_len,byte_offset);
      byte_offset+=key_len;

      uint32_t value_len;
      pread(fd,&value_len,sizeof(value_len),byte_offset);

      byte_offset+=sizeof(value_len);

      std::string value_(value_len);
      pread(fd,value_.data(),value_len,byte_offset);
      byte_offset+=value_len;

      uint8_t tomb;
      pread(fd,&tomb,sizeof(tomb),byte_offset);

      //got key,value and tombstone value of a specific peice of data

      if (!tomb){
        return value_;
      }

      return std::nullopt;







    }



};














