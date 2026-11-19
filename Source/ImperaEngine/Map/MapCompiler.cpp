#include "MapCompiler.hpp"

namespace impera {

	bool MapCompiler::compile(const std::string& fileName, const std::unordered_map<uint32_t, ProvinceId>& colorToProvince, Map& map) {

		spdlog::info("Map File: {}", fileName);
			
		std::unordered_set<int> colorsWithoutProvince;
			
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
			
		spdlog::info("Map size: {} x {}", converted->w, converted->h);
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
			
		for(auto& entry : colorsWithoutProvince) {
			spdlog::warn("There is no province defined for color {}", entry);
		}
			
		SDL_UnlockSurface(converted);
		SDL_DestroySurface(converted);
		SDL_DestroySurface(bitmap);
		return true;
	}
	
}