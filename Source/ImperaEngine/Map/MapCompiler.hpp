#ifndef IMPERAENGINE_MAP_MAPCOMPILER_HPP
#define IMPERAENGINE_MAP_MAPCOMPILER_HPP

#include "Map.hpp"
#include <unordered_set>
#include <spdlog/spdlog.h>
#include <fstream>

namespace impera {
	
	class MapCompiler {
	
		inline static uint32_t packColor(uint8_t r, uint8_t g, uint8_t b) {
			return (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
		}
	
		public:
		
		bool compile(const std::string& fileName, const std::unordered_map<uint32_t, ProvinceId>& colorToProvince, Map& map);
	
	};

}

#endif