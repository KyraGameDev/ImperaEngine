#ifndef IMPERAENGINE_MAP_MAPRENDERER_HPP
#define IMPERAENGINE_MAP_MAPRENDERER_HPP

#include "Map.hpp"
#include "MapChunkManager.hpp"
#include "Camera.hpp"

#include <SDL3/SDL.h>
#include <cmath>



namespace impera {
		
	class MapRenderer {
	
		Map* m_Map = nullptr;
		
		MapChunkManager m_MapChunkManager;
		
		Province* m_HoveredProvince = nullptr;
		Province* m_SelectedProvince = nullptr;
		
		void calculateBorders( MapChunk* chunk, const Map* map, 
							   std::function<bool(ProvinceId, int, int, const Map*)> provinceBorderCheckFunc,
							   std::function<bool(const Country*, int, int, const Map*)> countryBorderCheckFunc
							   ) {
			const int startX = chunk->x * CHUNK_SIZE;
			const int startY = chunk->y * CHUNK_SIZE;
			for(int y = 0; y < CHUNK_SIZE; ++y) {
				for(int x = 0; x < CHUNK_SIZE; ++x) {
					const int worldX = startX + x;
					const int worldY = startY + y;
					if(worldX < map->getWidth() && worldY < map->getHeight()) {
						const std::size_t index = size_t(worldY) * map->getWidth() + worldX;
						const ProvinceId provinceId = map->getProvinceIds()[index];
						const Province* province = map->getProvince(provinceId);
						if(province) {
							const Country* country = map->getCountry(province->owner);
							if(countryBorderCheckFunc(country, worldX, worldY, map)) {
								chunk->borderBuffer[y * CHUNK_SIZE + x] = COUNTRY_BORDER;
							}else if(provinceBorderCheckFunc(provinceId,worldX, worldY, map)) {
								chunk->borderBuffer[y * CHUNK_SIZE + x] = PROVINCE_BORDER;
							} else {
								chunk->borderBuffer[y * CHUNK_SIZE + x] = NO_BORDER;
							}
						}
					}
				}
			}
		}
		
		public:
						
		bool init(Map* map);

		
		void updateBorders( const Map* map, std::function<bool(ProvinceId, int, int, const Map*)> provinceBorderCheckFunc, std::function<bool(const Country*, int, int, const Map*)> countryBorderCheckFunc) {
			for(auto& chunk : m_MapChunkManager.getChunks()) {
				calculateBorders( &chunk, map, provinceBorderCheckFunc, countryBorderCheckFunc);			
			}
		}

		
		void setHoveredProvince(Province* province);
			
		void selectProvince();
	
		void draw(SDL_Renderer* renderer, const Camera& camera, MapModeRenderer* mapModeRenderer);
	
	};
}

#endif