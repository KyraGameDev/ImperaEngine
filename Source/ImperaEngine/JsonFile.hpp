#ifndef IMPERAENGINE_JSONFILE_HPP
#define IMPERAENGINE_JSONFILE_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

namespace impera {

	class JsonFile {
		
		nlohmann::json m_Data;
	
		public:
		
		using iterator = nlohmann::json::iterator;
		iterator begin() {return m_Data.begin();}
		iterator end() {return m_Data.end();}
		
		bool open( const std::string& file );
		
		bool contains(const std::string& id) const;
		
		const auto items() const {
			return m_Data.items();
		}
		
		auto operator[](const std::string& id) {
			return m_Data[id];
		}
	
	};

}

#endif