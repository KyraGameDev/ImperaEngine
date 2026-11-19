#ifndef IMPERAENGINE_MAP_MAPRENDERER_HPP
#define IMPERAENGINE_MAP_MAPRENDERER_HPP

#include "Map.hpp"
#include "MapChunkManager.hpp"
#include "Camera.hpp"

#include <SDL3/SDL.h>
#include <cmath>



namespace impera {
		
	template<class T>	
	class MapRenderer {
	
		T* m_Map = nullptr;
		
		MapChunkManager m_MapChunkManager;
		
		Province* m_HoveredProvince = nullptr;
		Province* m_SelectedProvince = nullptr;
		
		// TODO: THIS SHOULD BE IMPLEMENTED BY A MAPCHUNKPROCESSOR
		void calculateBorders( MapChunk* chunk, const T* map, 
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
						
		bool init(T* map) {
			if(!map) {
				return false;
			}
			m_Map = map;
			if(!m_MapChunkManager.init(map)) {
				return false;
			}
			for(int y = 0; y < map->getHeight(); ++y) {
				for( int x = 0; x < map->getWidth(); ++x) {
					int chunkX = x / CHUNK_SIZE;
					int chunkY = y / CHUNK_SIZE;
					int chunkId = m_MapChunkManager.getChunkId(chunkX,chunkY);
					if(m_MapChunkManager.isValid(chunkId)) {
						Province* province = map->getProvince({float(x),float(y)});
						if(province) {
							province->chunks.insert(chunkId);
						}
					}
				}
			}
			return true;
		}

		
		void updateBorders( const T* map, std::function<bool(ProvinceId, int, int, const T*)> provinceBorderCheckFunc, std::function<bool(const Country*, int, int, const T*)> countryBorderCheckFunc) {
			for(auto& chunk : m_MapChunkManager.getChunks()) {
				calculateBorders( &chunk, map, provinceBorderCheckFunc, countryBorderCheckFunc);			
			}
		}

		
		void setHoveredProvince(Province* province) {
			if(province == m_HoveredProvince) {
				return;
			}
			if(m_HoveredProvince && m_HoveredProvince->enabled) {
				for(auto& chunk : m_HoveredProvince->chunks) {
					impera::MapChunk* ch = m_MapChunkManager.getChunk( chunk );
					if(ch) {
						ch->dirty = true;
					}
				}
			}
			m_HoveredProvince = province;
			if(m_HoveredProvince && m_HoveredProvince->enabled) {
				for(auto& chunk : m_HoveredProvince->chunks) {
					impera::MapChunk* ch = m_MapChunkManager.getChunk( chunk );
					if(ch) {
						ch->dirty = true;
					}
				}
			}
		}
			
		void selectProvince() {
			if(m_HoveredProvince == m_SelectedProvince) {
				return;
			}
			if(m_SelectedProvince && m_SelectedProvince->enabled) {
				for(auto& chunk : m_SelectedProvince->chunks) {
					impera::MapChunk* ch = m_MapChunkManager.getChunk( chunk );
					if(ch) {
						ch->dirty = true;
					}
				}
			}
			m_SelectedProvince = m_HoveredProvince;
			if(m_SelectedProvince && m_SelectedProvince->enabled) {
				for(auto& chunk : m_SelectedProvince->chunks) {
					impera::MapChunk* ch = m_MapChunkManager.getChunk( chunk );
					if(ch) {
						ch->dirty = true;
					}
				}
			}
		}
	
		void draw(SDL_Renderer* renderer, const Camera& camera, MapModeRenderer<T>* mapModeRenderer) {
			if(!m_Map | !renderer | !mapModeRenderer) {
				return;
			}
			const SDL_FRect visible = camera.visibleWorldRect();
			const int minCX = std::max(0, (int)std::floor(visible.x / CHUNK_SIZE));
			const int minCY = std::max(0, (int)std::floor(visible.y / CHUNK_SIZE));
			const int maxCX = std::min(m_MapChunkManager.getChunksX() - 1, (int)(std::floor(visible.x + visible.w) / CHUNK_SIZE));
			const int maxCY = std::min(m_MapChunkManager.getChunksY() - 1, (int)(std::floor(visible.y + visible.h) / CHUNK_SIZE));
				
			for(int cy = minCY; cy <= maxCY; ++cy) {
				for(int cx = minCX; cx <= maxCX; ++cx) {
					MapChunk* chunk = m_MapChunkManager.getChunk(cx, cy);
					if(!chunk) {
						continue;
					}
					if(chunk->dirty) {
						m_MapChunkManager.rebuildChunk<T>(m_Map, chunk, m_HoveredProvince, m_SelectedProvince, mapModeRenderer);
						m_MapChunkManager.uploadChunk(renderer, chunk);
					}
						
					SDL_FRect destination {
						float(cx * CHUNK_SIZE),
						float(cy * CHUNK_SIZE),
						float(CHUNK_SIZE),
						float(CHUNK_SIZE)
					};
						
					SDL_FPoint topLeft = camera.worldToScreen(destination.x, destination.y);
					destination.x = topLeft.x;
					destination.y = topLeft.y;
					destination.w *= camera.zoom;
					destination.h *= camera.zoom;
					
					SDL_RenderTexture(renderer, chunk->texture, nullptr, &destination);
					
				}
			}
		}
	
	};
}

#endif