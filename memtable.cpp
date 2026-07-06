#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <optional>

class Memtable{

	private:
		std::map<std::string,std::pair<std::string,bool>> table;
		WAL wal_log;
		int count=0;
		vector<data_entry> past_writes;

		void flush_memtable(){
			//flush current memtable into a SSTable
			//reset map
			continue;
		}

		void recover_from_wal(){

			wal_log.open_wal("./data");

			vector<data_entry> past_logs = wal_log.recover();
			for (data_entry i:past_logs){
				table[i.key].first = i.value;
				table[i.key].second= i.tombstone;
			}
		}


			
	
	public:
		Memtable(){
			recover_from_wal();


		}
		void put(std::string key,std::string value){

			wal_log.write_wal(key,value,false);
			table[key].first = value;
			table[key].second = false;
			count++;

			if (count==5){
				flush_memtable();
				count=0;
				

			}
		}

		std::optional<std::string> get(std::string key_){
			try{
			auto answer =  table.at(key_);
			if (!answer.second){
			return answer.first;
			}
			return std::nullopt;
			}
			catch (const std::out_of_range& e){
				//ADD BEHAVIOUR TO SEARCH SSTABLE ONCE U MAKE IT
				std::cout<<"No data found"<<std::endl;
				return std::nullopt;
			}

		}

		void delete_value(std::string key_){
			wal_log.write_wal(key_," ",true);
			auto obj = table.find(key_);
			if (obj!=table.end()){

			obj->second.second=true;
			}
			//ADD LOGIC TO HANDLE THE SSTABLE LOGIC FOR THIS SO THAT DELETE IS COUNTED

		}

	
};
