#ifndef IMPERAENGINE_MAP_CAMERA_HPP
#define IMPERAENGINE_MAP_CAMERA_HPP

#include <SDL3/SDL.h>
#include <algorithm>

namespace impera {
	
	class Camera {
		
		public:
	
		float x = 0.f;
		float y = 0.f;
		float zoom = 1.f;
		
		int screenWidth = 1280;
		int screenHeight = 720;
				
		SDL_FPoint screenToWorld(float sx, float sy) const {
			return {x + sx / zoom, y + sy / zoom};
		}
		
		SDL_FPoint worldToScreen(float wx, float wy) const {
			return {(wx-x) * zoom, (wy-y) * zoom };
		}
		
		void zoomAt(float sx, float sy, float factor) {
			const auto before = screenToWorld(sx, sy);
			zoom = factor;
			zoom = std::clamp(zoom, 0.15f, 5.f);
			const auto after = screenToWorld(sx, sy);
			x += before.x - after.x;
			y += before.y - after.y;
		}
		
		SDL_FRect visibleWorldRect() const {
			return {x,y,screenWidth / zoom, screenHeight / zoom};
		}
	
	};
}

#endif