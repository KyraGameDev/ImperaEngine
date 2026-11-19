#ifndef IMPERAENGINE_MAP_MAPCHUNKMANAGER_HPP
#define IMPERAENGINE_MAP_MAPCHUNKMANAGER_HPP

#include "Map.hpp"

#include <SDL3/SDL.h>
#include <vector>
#include <functional>

#include <spdlog/spdlog.h>

namespace impera {
	
	
	static constexpr int CHUNK_SIZE = 256;
	static constexpr int COUNTRY_BORDER = 1;
	static constexpr int PROVINCE_BORDER = 2;
	static constexpr int NO_BORDER = 0;
	
	
	struct MapChunk {
		int x = 0;
		int y = 0;
		SDL_Texture* texture = nullptr;
		bool dirty = true;
		std::vector<uint8_t> pixelBuffer;
		std::vector<uint8_t> borderBuffer;
	};
	
	
	class MapModeRenderer {
	
		public:
		virtual ~MapModeRenderer() = default;
		
		virtual Color getPixelColor(int x, 
									int y, 
									const MapChunk* chunk, 
									const Province* province, 
									const Map* map, 
									int chunkSize,
									bool hovered,
									bool selected) const = 0;
	};
	
	class DefaultMapModeRenderer : public MapModeRenderer {
	
		Color m_ProvinceBorderColor = {0,0,0,255};
		Color m_ProvinceBorderColorHovered = {0,0,0,255};
		Color m_ProvinceBorderColorSelected = {0,0,0,255};
		Color m_CountryBorderColor = {0,0,0,255};
		Color m_CountryBorderColorHovered = {0,0,0,255};
		Color m_CountryBorderColorSelected = {0,0,0,255};
	
		public:
		
		virtual Color getPixelColor(int x, 
									int y, 
									const MapChunk* chunk, 
									const Province* province, 
									const Map* map, 
									int chunkSize,
									bool hovered,
									bool selected) const {
			Color color = {0,0,0,255};
			const std::size_t borderIndex = (std::size_t(y) * chunkSize + x);
			if(province) {
				const Country* country = map->getCountry(province->owner);
				if( chunk->borderBuffer[borderIndex] == COUNTRY_BORDER) {
					if(selected && province->enabled) {
						color = m_CountryBorderColorSelected;
					} else if(hovered && province->enabled) {
						color = m_CountryBorderColorHovered;
					} else {
						color = m_CountryBorderColor;
					}
				} else if(chunk->borderBuffer[borderIndex] == PROVINCE_BORDER) {
					if(selected && province->enabled) {
						color = m_ProvinceBorderColorSelected;
					} else if(hovered && province->enabled) {
						color = m_ProvinceBorderColorHovered;
					} else {
						color = m_ProvinceBorderColor;
					}
				} else if(chunk->borderBuffer[borderIndex] == NO_BORDER) {
					if(country) {
						color = country->color;
					}
					if(selected && province->enabled) {
						color.r += 50;
						color.g += 50;
						color.b += 50;
					}else if(hovered && province->enabled) {
						color.r += 25;
						color.g += 25;
						color.b += 25;
					}
				}
			}
			return color;
			
		}
	};
		
	class MapChunkManager {
	
		std::vector<MapChunk> m_Chunks;
		
		int m_ChunksX = 0;
		int m_ChunksY = 0;

		std::unordered_map<std::string, MapModeRenderer*>* m_MapModeRenderers = nullptr;

		DefaultMapModeRenderer m_DefaultMapModeRenderer;
		MapModeRenderer* m_ActiveMapMode = nullptr;


		public:
		~MapChunkManager() {
			for(auto& chunk : m_Chunks) {
				SDL_DestroyTexture(chunk.texture);
			}
		}
		
		int getChunksX() const {
			return m_ChunksX;
		}
		
		int getChunksY() const {
			return m_ChunksY;
		}
			
		bool init(Map* map) {
			spdlog::info("Initialise MapChunkManager");
			m_ChunksX = (map->getWidth() + CHUNK_SIZE-1) / CHUNK_SIZE;
			m_ChunksY = (map->getHeight() + CHUNK_SIZE-1) / CHUNK_SIZE;
			spdlog::info("Initialise {} x {} chunks, total : {}", m_ChunksX, m_ChunksY, m_ChunksX * m_ChunksY);
			spdlog::info("Chunk size: {}", CHUNK_SIZE);
			spdlog::info("Chunk pixel buffer size: {}",CHUNK_SIZE * CHUNK_SIZE * 4); 
			m_Chunks.resize(m_ChunksX * m_ChunksY);
			for (int y = 0; y < m_ChunksY; ++y) {
				for (int x = 0; x < m_ChunksX; ++x) {
					spdlog::info("Initialise chunk {}", y * m_ChunksX + x);
								auto& c = m_Chunks[y * m_ChunksX + x];
					c.x = x;
					c.y = y;
					c.pixelBuffer.resize(
						CHUNK_SIZE * CHUNK_SIZE * 4
					);
					c.borderBuffer.resize(CHUNK_SIZE * CHUNK_SIZE);
				}
			}
			return true;
		}
		
		std::vector<MapChunk>& getChunks() {
			return m_Chunks;
		}
		
		MapChunk* getChunk(int x, int y) {
			if(x < 0 || y < 0) {
				return nullptr;
			}
			if(x >= m_ChunksX || y >= m_ChunksY) {
				return nullptr;
			}
			return &m_Chunks[y * m_ChunksX + x];
		}
		
		int getChunkId(int x, int y) const {
			return y * m_ChunksX + x;
		}
		
		bool isValid(int id) {
			return id >= 0 && id < static_cast<int>(m_Chunks.size());
		}
		
		MapChunk* getChunk(int id) {
			if(id < 0 || id >= static_cast<int>(m_Chunks.size())) {
				return nullptr;
			}
			return &m_Chunks[id];
		}
		
		SDL_Texture* createChunkTexture(SDL_Renderer* renderer) {
			SDL_Texture* texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, CHUNK_SIZE, CHUNK_SIZE);
			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
			return texture;
		}
		
		void rebuildChunk(Map* map, MapChunk* chunk, Province* hovered, Province* selected, MapModeRenderer* renderer) {
			if( !map || !chunk || !renderer) {
				return;
			}
			const int startX = chunk->x * CHUNK_SIZE;
			const int startY = chunk->y * CHUNK_SIZE;
			for(int y = 0; y < CHUNK_SIZE; ++y) {
				for(int x = 0; x < CHUNK_SIZE; ++x) {
					const int worldX = startX + x;
					const int worldY = startY + y;
					Color color{0,0,0,255};
					if(worldX < map->getWidth() && worldY < map->getHeight()) {
						const std::size_t index = size_t(worldY) * map->getWidth() + worldX;
						const ProvinceId provinceId = map->getProvinceIds()[index];
						const Province* province = map->getProvince(provinceId);
						color = renderer->getPixelColor(x,y,
															   chunk, 
															   province,
															   map, 
															   CHUNK_SIZE,
															   province == hovered,
															   province == selected);
					}
					const std::size_t pixelIndex = (std::size_t(y) * CHUNK_SIZE + x) * 4;
					chunk->pixelBuffer[pixelIndex + 0] = color.r;
					chunk->pixelBuffer[pixelIndex + 1] = color.g;
					chunk->pixelBuffer[pixelIndex + 2] = color.b;
					chunk->pixelBuffer[pixelIndex + 3] = color.a;
				}
			}
			chunk->dirty = false;
		}
		
		void uploadChunk(SDL_Renderer* renderer, MapChunk* chunk) {
			if(!chunk->texture) {
				chunk->texture = createChunkTexture(renderer);
			}
			SDL_UpdateTexture(chunk->texture, nullptr, chunk->pixelBuffer.data(), CHUNK_SIZE * 4);
		}
	
	};
}

#endif