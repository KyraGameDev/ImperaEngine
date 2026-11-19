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

namespace impera {

	struct ApplicationDefinition {
		std::string name = "ImperaEngine";
		std::string logFile = "ImperaEngine.log.txt";
		std::string colorToProvinceMapFile = "./Data/Map/ColorToProvince.json";
		std::string provincesFile = "./Data/Data/Provinces.json";
		std::string countriesFile = "./Data/Data/Countries.json";
		std::string mapFile = "./Data/Map/Map.bmp";
	};

	class ImperaEngine {
	
		SDL_Window* m_Window = nullptr;
		SDL_Renderer* m_Renderer = nullptr;
		
		public:
		~ImperaEngine() {
			SDL_DestroyRenderer(m_Renderer);
			SDL_DestroyWindow(m_Window);
			SDL_Quit();
		}
		
		int run(const ApplicationDefinition& definition, int argc, char** argv) {

			auto logger = spdlog::basic_logger_mt("ImperaEngine", definition.logFile, true);
			spdlog::set_default_logger(logger);

			spdlog::info("Starting Impera Engine v0.1.0");
		
			spdlog::info("Init SDL");
			if(!SDL_Init(SDL_INIT_VIDEO)) {
				spdlog::error("Failed to initialise SDL: {}", SDL_GetError());
				return -1;
			}
			
			spdlog::info("Create SDL_Window");
			m_Window = SDL_CreateWindow(definition.name.c_str(), 1280, 720, 0);
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
			if(!json.open(definition.colorToProvinceMapFile)) {
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
			
			if(!json.open(definition.provincesFile)) {
				spdlog::error("Failed to load Provinces.json");
				return -3;
			}
				
			if (json.contains("Provinces")) {
				for (const auto& province : json["Provinces"]) {
					if (province.contains("Id") && province.contains("Owner")) {
						const auto id = province["Id"].get<impera::ProvinceId>();
						const auto owner = province["Owner"].get<impera::CountryId>();
						map.addProvince( {id, owner} );
					}
				}
			}
				
			if(!json.open(definition.countriesFile)) {
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
			if(!mapCompiler.compile(definition.mapFile, colorToProvinceMap, map)) {
				spdlog::error("Failed to compile map");
				return -4;
			}
			
			spdlog::info("Create Impera::MapRenderer");
			impera::MapRenderer mapRenderer;
			if(!mapRenderer.init(&map)) {
				spdlog::error("Failed to intialise Impera::MapRenderer");
				return -4;
			}
				
			spdlog::info("Create Impera::Camera");
			impera::Camera camera;
			
			bool isOpen = true;
			SDL_Event event;
			float zoomFactor = 1.f;
			while(isOpen) {
				while(SDL_PollEvent(&event)) {
					switch(event.type) {
						case SDL_EVENT_QUIT:
							isOpen = false;
						break;
						case SDL_EVENT_MOUSE_MOTION:
						{	
							impera::Province* tmp = map.getProvince( camera.screenToWorld(event.motion.x, event.motion.y) );
							mapRenderer.setHoveredProvince( tmp );
						}
						break;
						case SDL_EVENT_KEY_DOWN:
							if(event.key.key == SDLK_UP) {
								camera.y -= 15.0f;
							} else if( event.key.key == SDLK_DOWN) {
								camera.y += 15.0f;
							} else if( event.key.key == SDLK_LEFT) {
								camera.x -= 15.0f;
							} else if( event.key.key == SDLK_RIGHT) {
								camera.x += 15.0f;
							}
						break;
						case SDL_EVENT_MOUSE_WHEEL:
							if(event.wheel.y > 0) {
								zoomFactor += 0.05;
							} else {
								zoomFactor -= 0.05;
							}
							camera.zoomAt( event.wheel.mouse_x, event.wheel.mouse_y, zoomFactor);
						break;
						case SDL_EVENT_MOUSE_BUTTON_DOWN:
						{
							mapRenderer.selectProvince();
						}
						break;
					}
				}
				uint64_t beginFrame = SDL_GetPerformanceCounter();
				SDL_RenderClear(m_Renderer);
				mapRenderer.draw(m_Renderer, camera);
				SDL_RenderPresent(m_Renderer);
				uint64_t endFrame = SDL_GetPerformanceCounter();
				spdlog::info("FrameTime: {} Zoom: {}", (endFrame - beginFrame), zoomFactor);
			}
			
			spdlog::info("Finished");
			
			return 0;
		}
	
	
	};


}

#endif