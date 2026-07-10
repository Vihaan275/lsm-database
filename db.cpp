#include <iostream>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

class DB{


  private:
    Memtable memtable;
    std::vector<SSTable> sstables;
    int sstable_idx=0;
    int memtable_size;

    void load_sstables(){
      std::string file_path = "tables/";
      std::vector<SSTable> temp;

      for (auto& entry:std::filesystem::director_iterator(file_path)){

        sstables.emplace_back(std::stoi(entry.path().stem().string()));

        std::sort(sstables.begin(),sstables.end(),[](const SSTable& a, const SSTable& b){return a.get_index() < b.get_index()});

        sstable_idx = sstables.empty() ? 0: sstables[sstables.size()-1].get_index()+1;
      }



    }



    void flush(){

      auto temp = std::move(memtable);
      temp.flush_memtable();
      memtable = Memtable{};
      memtable_size=0;
      sstables.emplace_back(sstable_idx);
      sstables.back().flush_sstable(temp);
      sstable_idx++;

    }


  public:

    DB(){
      load_sstables();

    }

    void put_value(std::string key, std::string value){

      memtable.put(key,value);
      memtable_size+= key.size()+value.size();


      if (memtable_size>=200){//change to 4 mil as 4mb, 200 is for testing
        flush();
      }
    }

    void db_delete_value(std::string key){
      memtable.delete_value(key);

    }

    std::optional<std::string> db_get_value(std::string key){

      auto value = memtable.get(key);

      if (value){
        return *value;
      }

      //have to check sstables now

      if (sstables.size()==0){
        return std::nullopt;
      }

      for (auto it=sstables.rbegin();it!=sstables.rend();++it){
        auto value = it->get_value(key);

        if (!value){//it's not in this sstable

          continue;//go to the next sstable
        }

        if (!*value){//in the sstable, but has a tombstone of false
          return std::nullopt;
        }

          return **value;
      }  


      return std::nullopt;

};
