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
		
		bool compile(const std::string& fileName, const std::unordered_map<uint32_t, ProvinceId>& colorToProvince, Map& map) {
			
			std::unordered_set<int> colorsWithoutProvince;
			
			spdlog::info("{}", __PRETTY_FUNCTION__);
			
			spdlog::info("Load Bitmap: {}", fileName);
			SDL_Surface* bitmap = SDL_LoadBMP(fileName.c_str());
			if(!bitmap) {
				spdlog::error("Failed to load bitmap {} : {}", fileName, SDL_GetError());
				return false;
			}

			spdlog::info("Convert bitmap: {}", fileName);
			SDL_Surface* converted = SDL_ConvertSurface(bitmap, SDL_PIXELFORMAT_RGB24);
			if(!converted) {
				spdlog::error("Failed to convert bitmap {} : {}", fileName, SDL_GetError());
				SDL_DestroySurface(bitmap);
				return false;
			}
			
			map.setSize(converted->w, converted->h);
			
			SDL_LockSurface(converted);
			const auto* pixels = static_cast<const uint8_t*>(converted->pixels);
						
			for(int32_t y = 0; y < map.getHeight(); ++y) {
				const uint8_t* row = pixels + y * converted->pitch;
				for(int32_t x = 0; x < map.getWidth(); ++x) {
					const uint8_t* pixel = row + x * 3;
					const uint32_t color = packColor( pixel[0], pixel[1], pixel[2] );
					ProvinceId id = 0;
					auto it = colorToProvince.find(color);
					if(it != colorToProvince.end()) {
						id = it->second;
					} else {
						colorsWithoutProvince.insert(color);
					}
					map.setProvinceId(size_t(y) * map.getWidth() + std::size_t(x), id);
				}
			}
			
			std::ofstream stream("MapCompiler.log.txt");
			for(auto& entry : colorsWithoutProvince) {
				stream << entry << std::endl;
			}
			stream.close();
			
			SDL_UnlockSurface(converted);
			
			SDL_DestroySurface(converted);
			SDL_DestroySurface(bitmap);
			return true;
		}
	
	};

}

#endif