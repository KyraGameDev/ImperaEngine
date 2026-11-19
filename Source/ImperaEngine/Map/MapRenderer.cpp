#include "MapRenderer.hpp"

namespace impera {

	bool MapRenderer::init(Map* map) {
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
		
	void MapRenderer::setHoveredProvince(Province* province) {
		if(province == m_HoveredProvince) {
			return;
		}
		if(m_HoveredProvince) {
			for(auto& chunk : m_HoveredProvince->chunks) {
				impera::MapChunk* ch = m_MapChunkManager.getChunk( chunk );
				if(ch) {
					ch->dirty = true;
				}
			}
		}
		m_HoveredProvince = province;
		if(m_HoveredProvince) {
			for(auto& chunk : m_HoveredProvince->chunks) {
				impera::MapChunk* ch = m_MapChunkManager.getChunk( chunk );
				if(ch) {
					ch->dirty = true;
				}
			}
		}
	}
		
	void MapRenderer::selectProvince() {
		if(m_HoveredProvince == m_SelectedProvince) {
			return;
		}
		if(m_SelectedProvince) {
			for(auto& chunk : m_SelectedProvince->chunks) {
				impera::MapChunk* ch = m_MapChunkManager.getChunk( chunk );
				if(ch) {
					ch->dirty = true;
				}
			}
		}
		m_SelectedProvince = m_HoveredProvince;
		if(m_SelectedProvince) {
			for(auto& chunk : m_SelectedProvince->chunks) {
				impera::MapChunk* ch = m_MapChunkManager.getChunk( chunk );
				if(ch) {
					ch->dirty = true;
				}
			}
		}
	}
	
	void MapRenderer::draw(SDL_Renderer* renderer, const Camera& camera) {
		if(!m_Map | !renderer) {
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
					m_MapChunkManager.rebuildChunk(m_Map, chunk, m_HoveredProvince, m_SelectedProvince);
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

}