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
				if (!(i.tombstone)){//if this wasnt a delete
					table[i.key].first = i.value;
					table[i.key].second= i.tombstone;
				}
				else{// it is a remove operation
					auto obj = table.find(i.key);
					if (obj!=table.end()){
						obj->second.second=true;
					}
				}
			}
		}


			
	
	public:
		Memtable(){
			recover_from_wal();


		}
		void put(std::string key,std::string value){

			wal_log.write_wal(key,value,false);
			table[key].first = value;
			count++;

			if (count==5){
				flush_memtable();
				count=0;
				

			}
		}

		std::optional<std::string> get(std::string key_){
			try{
			auto answer =  table.at(key_);
			return answer.first;
			}
			catch (const std::out_of_range& e){
				std::cout<<"No data found"<<std::endl;
				return std::nullopt;
			}
			if (answer.second){
				return std::nullopt;
			}

		}

		void delete_value(std::string key_){
			auto obj = table.find(key_);
			if (obj!=table.end()){

			wal_log.write_wal(key_," ",true);
			obj->second.second=true;
			}

		}

	
};
