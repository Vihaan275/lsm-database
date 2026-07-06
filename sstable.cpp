#include <iostream>
#include <vector>
#include <string>

class SSTable{

  private:
    string file_path="tables/";
    int fd;

  public:

    SSTable(int index){
      //makes a .sst file with index as the file name
      file_path=file_path+std::to_string(index)+".sst";

      //opens the fd in append mode
      fd = open(file_path.c_str(),O_WRONLY | O_CREAT | O_TRUNC,0600);


    }

    int flush_sstable(std::map<std::string,std::pair<std::string,bool>> table){
      int size = table.size();
      int arr_idx=0;
      uint32_t byte_count=0;
      uint32_t arr[size];
      std::vector<std::string> temp;

      //writing the data into the file
      for (auto const& [key,value] : table){
        arr[arr_idx]=byte_count

        //writing the key at the start of the line
        uint32_t key_len = key.size();
        write(fd,reinterpret_cast<const char*>(&key_len),sizeof(key_len));
        byte_count+=sizeof(key_len)
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
      int start_of_index_table = byte_count;

      for (int i=0;i<arr_idx,i++){

        uint32_t key_len = temp.at(i).size();

        write(fd,reinterpret_cast<const char*>(&key_len),sizeof(key_len));
        write(fd,temp.at(i).data(),key_len);
        write(fd,arr[i],sizeof(uint32_t));

      }

    } 



};
