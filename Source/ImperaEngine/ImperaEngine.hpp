#ifndef IMPERAENGINE_IMPERAENGINE_HPP
#define IMPERAENGINE_IMPERAENGINE_HPP

#include "JSONFile.hpp"
#include "Map/Camera.hpp"
#include "Map/MapChunkManager.hpp"
#include "Map/MapCompiler.hpp"
#include "Map/MapRenderer.hpp"
#include "Map/Map.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>
#include <typeindex>

namespace impera {

	class GameState {
	
		bool m_ShouldDrawMap = true;
	
		protected:
		
		void shouldDrawMap(bool value) {
			m_ShouldDrawMap = value;
		}
	
		public:
		
		virtual bool init() = 0;
		
		bool drawMapEnabled() const {
			return m_ShouldDrawMap;
		}
			
	};
	
	
	
	[[maybe_unused]] static bool isProvinceBorder(ProvinceId provinceId, int x, int y, const Map* map) {
		for (int dx = -2; dx <= 2; ++dx) {
			for (int dy = -2; dy <= 2; ++dy) {
				if (dx == 0 && dy == 0) {
					continue;
				}
				if (dx * dx + dy * dy > 4) {
					continue;
				}
				auto pid = map->getProvinceId(x + dx, y + dy);
				if (pid != -1 && pid != provinceId) {
					return true;
				}
			}
		}
		return false;
	}
	
	
	[[maybe_unused]] static bool isCountryBorder(const Country* country, int x, int y, const Map* map) {
		for (int dx = -4; dx <= 4; ++dx) {
			for (int dy = -4; dy <= 4; ++dy) {
				if (dx == 0 && dy == 0) {
					continue;
				}
				if (dx * dx + dy * dy > 16) {
					continue;
				}
				const Country* c = map->getCountryByPixel(x + dx, y + dy);
				if (c != country) {
					return true;
				}
			}
		}
		return false;
	}
	
	
	class ImperaEngine {
	
		SDL_Window* m_Window = nullptr;
		SDL_Renderer* m_Renderer = nullptr;
		
		std::string m_Title = "ImperaEngine";
		
		std::shared_ptr<GameState> m_ActiveGameState = nullptr;
		
		std::unordered_map<std::type_index, std::unique_ptr<MapModeRenderer>> m_MapModeRenderers;
		MapModeRenderer* m_ActiveMapModeRenderer = nullptr;
		
		std::function<bool(ProvinceId, int, int, const Map*)> m_ProvinceBorderCheckFunc = &isProvinceBorder;
		
		std::function<bool(const Country*, int, int, const Map*)> m_CountryBorderCheckFunc = &isCountryBorder;
		
		public:
		~ImperaEngine() {
			SDL_DestroyRenderer(m_Renderer);
			SDL_DestroyWindow(m_Window);
			SDL_Quit();
		}
		
		template<class T>
		void setActiveGameState() {
			m_ActiveGameState = std::make_unique<T>();
			m_ActiveGameState->init();
		}
		
		template<class T>
		bool setDefaultMapModeRenderer() {
			auto it = m_MapModeRenderers.find( std::type_index( typeid(T) ) );
			if(it == m_MapModeRenderers.end()) {
				return false;
			}
			m_ActiveMapModeRenderer = it->second.get();
			return true;
		}
		
		template<class T>
		void registerMapModeRenderer() {
			m_MapModeRenderers[ std::type_index(typeid(T)) ] = std::make_unique<T>();
		}
		
		void setTitle(const std::string& title) {
			m_Title = title;
		}
		
		void setProvinceBorderCheckFunction( std::function<bool(ProvinceId, int, int, const Map*)> func ) {
			m_ProvinceBorderCheckFunc = func;
		}
		
		void setCountryBorderCheckFunction( std::function<bool(const Country*, int, int, const Map*)> func ) {
			m_CountryBorderCheckFunc = func;
		}
		
		int run(int argc, char** argv) {

			auto logger = spdlog::basic_logger_mt("ImperaEngine", "ImperaEngine.log.txt", true);
			spdlog::set_default_logger(logger);

			spdlog::info("Starting Impera Engine v0.1.0");
		
			spdlog::info("Init SDL");
			if(!SDL_Init(SDL_INIT_VIDEO)) {
				spdlog::error("Failed to initialise SDL: {}", SDL_GetError());
				return -1;
			}
			
			spdlog::info("Create SDL_Window");
			m_Window = SDL_CreateWindow(m_Title.c_str(), 1280, 720, 0);
			if(!m_Window) {
				spdlog::error("Failed to create SDL_Window: {}", SDL_GetError());
				return -2;
			}
			
			spdlog::info("Create SDL_Renderer");
			m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
			if(!m_Renderer) {
				spdlog::error("Failed to create SDL_Renderer: {}", SDL_GetError());
				return -3;
			}
				
			impera::Map map;
			
			impera::JsonFile json;
			if(!json.open("./Data/Map/ColorToProvince.json")) {
				spdlog::error("Failed to load ColorToProvince.json");
				return -3;
			}

			std::unordered_map<uint32_t, ProvinceId> colorToProvinceMap;

			if (json.contains("Entries")) {
				for (const auto& entry : json["Entries"]) {
					if (entry.contains("Color") && entry.contains("Id")) {
						const auto color = entry["Color"].get<uint32_t>();
						const auto provinceId = entry["Id"].get<impera::ProvinceId>();
						colorToProvinceMap[color] = provinceId;
					}
				}
			}
			
			if(!json.open("./Data/Data/Provinces.json")) {
				spdlog::error("Failed to load Provinces.json");
				return -3;
			}
				
			if (json.contains("Provinces")) {
				for (const auto& province : json["Provinces"]) {
					if (province.contains("Id") && province.contains("Owner")) {
						const auto id = province["Id"].get<impera::ProvinceId>();
						const auto owner = province["Owner"].get<impera::CountryId>();
						const bool enabled = province["Enabled"].get<bool>();
						map.addProvince( {id, owner, enabled} );
					}
				}
			}
				
			if(!json.open("./Data/Data/Countries.json")) {
				spdlog::error("Failed to load Countries.json");
				return -3;
			}

			if (json.contains("Countries")) {
				for (const auto& country : json["Countries"]) {
					if (country.contains("Id") && country.contains("R") && country.contains("G") && country.contains("B")) {
						const auto id = country["Id"].get<impera::CountryId>();
						const auto r = country["R"].get<uint8_t>();
						const auto g = country["G"].get<uint8_t>();
						const auto b = country["B"].get<uint8_t>();
						impera::Country c;
						c.id = id;
						c.color = {r,g,b,255};
						map.addCountry(c);
					}
				}
			}
			
			spdlog::info("Compile Map");
			impera::MapCompiler mapCompiler;
			if(!mapCompiler.compile("./Data/Map/Map.bmp", colorToProvinceMap, map)) {
				spdlog::error("Failed to compile map");
				return -4;
			}
			
			spdlog::info("Create Impera::MapRenderer");
			impera::MapRenderer mapRenderer;
			if(!mapRenderer.init(&map)) {
				spdlog::error("Failed to intialise Impera::MapRenderer");
				return -4;
			}
			mapRenderer.updateBorders(&map, m_ProvinceBorderCheckFunc, m_CountryBorderCheckFunc); 
			
				
			spdlog::info("Create Impera::Camera");
			impera::Camera camera;
			
			bool isOpen = true;
			SDL_Event event;
			float zoomFactor = 1.f;
			while(isOpen && m_ActiveGameState) {
				auto gameState = m_ActiveGameState;
				while(SDL_PollEvent(&event)) {
					switch(event.type) {
						case SDL_EVENT_QUIT:
							isOpen = false;
						break;
						case SDL_EVENT_MOUSE_MOTION:
						{	
							if(m_ActiveGameState->drawMapEnabled()) {
								impera::Province* tmp = map.getProvince( camera.screenToWorld(event.motion.x, event.motion.y) );
								mapRenderer.setHoveredProvince( tmp );
							}
						}
						break;
						case SDL_EVENT_KEY_DOWN:
							if(m_ActiveGameState->drawMapEnabled()) {
								if(event.key.key == SDLK_UP) {
									camera.y -= 15.0f;
								} else if( event.key.key == SDLK_DOWN) {
									camera.y += 15.0f;
								} else if( event.key.key == SDLK_LEFT) {
									camera.x -= 15.0f;
								} else if( event.key.key == SDLK_RIGHT) {
									camera.x += 15.0f;
								}
							}
						break;
						case SDL_EVENT_MOUSE_WHEEL:
							if(m_ActiveGameState->drawMapEnabled()) {
								if(event.wheel.y > 0) {
									zoomFactor += 0.05;
								} else {
									zoomFactor -= 0.05;
								}
								camera.zoomAt( event.wheel.mouse_x, event.wheel.mouse_y, zoomFactor);
							}
						break;
						case SDL_EVENT_MOUSE_BUTTON_DOWN:
						if(m_ActiveGameState->drawMapEnabled()) {
							mapRenderer.selectProvince();						
						}
						break;
					}
				}
				SDL_RenderClear(m_Renderer);
				if(m_ActiveGameState->drawMapEnabled()) {
					mapRenderer.draw(m_Renderer, camera, m_ActiveMapModeRenderer);
				}
				SDL_RenderPresent(m_Renderer);
			}
			spdlog::info("Finished");
			return 0;
		}
	
	
	};


}

#endif