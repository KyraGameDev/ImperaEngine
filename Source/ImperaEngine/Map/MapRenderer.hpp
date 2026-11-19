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
	
		public:
						
		bool init(Map* map);
		
		void setHoveredProvince(Province* province);
		
		void selectProvince();
		
		void draw(SDL_Renderer* renderer, const Camera& camera);
	
	};
}

#endif