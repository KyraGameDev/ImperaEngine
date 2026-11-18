#include "JSONFile.hpp"

namespace impera {

	bool JsonFile::open( const std::string& file ) {
		std::ifstream stream(file.c_str());
		if(!stream.is_open()) {
			return false;
		}
		stream >> m_Data;
		stream.close();
		return true;
	}
		
	bool JsonFile::contains(const std::string& id) const {
		return m_Data.contains(id);
	}

}